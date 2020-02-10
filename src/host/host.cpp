#include <iostream>
#include <fstream>
#include <cmath>

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

#define KERNEL_NAME "axpy"

static constexpr size_t VECTOR_SIZE = 1024;

template <typename T>
struct aligned_allocator
{
    using value_type = T;
    T* allocate(std::size_t num)
    {
        void *ptr = nullptr;
        if (posix_memalign(&ptr, 4096, num * sizeof(T))) {
            throw std::bad_alloc();
        }
        return reinterpret_cast<T*>(ptr);
    }

    void deallocate(T *p, std::size_t num)
    {
        free(p);
    }
};

int create_vector(std::vector<float, aligned_allocator<float>> &vec, size_t length);
int read_xclbin(std::string const& fname, std::vector<char> &binary);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <xclbin>";
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    int err;
    std::vector<char> binary;
    std::vector<float, aligned_allocator<float>> data_a, data_x, data_y, data_out;

    err = read_xclbin(argv[1], binary);
    if (err) {
        return err;
    }

    err = create_vector(data_a, VECTOR_SIZE);
    if (err) {
        return err;
    }

    err = create_vector(data_x, VECTOR_SIZE);
    if (err) {
        return err;
    }

    err = create_vector(data_y, VECTOR_SIZE);
    if (err) {
        return err;
    }

    uint32_t length = data_a.size();
    uint32_t size_bytes = length * sizeof(float);
    data_out.resize(length);

    std::vector<cl::Device> devices;
    cl::Device device;

    std::vector<cl::Platform> platforms;

    cl::Platform::get(&platforms);
    bool found_device = false;
    for (const auto& platform : platforms) {
        std::string name = platform.getInfo<CL_PLATFORM_NAME>();
        if (name == "Xilinx") {
            platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);
            if (devices.size()) {
                device = devices[0];
                found_device = true;
                break;
            }
        }
    }

    if (!found_device) {
        std::cerr << "Device not found" << std::endl;
        return EXIT_FAILURE;
    }

    cl::Context context(device);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE);

    cl::Program::Binaries bins{{binary.data(), binary.size()}};
    devices.resize(1);
    cl::Program program(context, devices, bins);

    cl::Kernel kernel(program, KERNEL_NAME);

    cl::Buffer buffer_a(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_a.data());
    cl::Buffer buffer_x(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_x.data());
    cl::Buffer buffer_y(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_y.data());
    cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_out.data());

    kernel.setArg(0, buffer_a);
    kernel.setArg(1, buffer_x);
    kernel.setArg(2, buffer_y);
    kernel.setArg(3, buffer_out);
    kernel.setArg(4, length);

    q.enqueueMigrateMemObjects({buffer_a, buffer_x, buffer_y}, 0);
    q.enqueueMigrateMemObjects({buffer_out}, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);
    q.enqueueBarrierWithWaitList();

    cl::Event event;
    q.enqueueTask(kernel, NULL, &event);

    std::vector<cl::Event> events{event};
    q.enqueueMigrateMemObjects({buffer_out}, CL_MIGRATE_MEM_OBJECT_HOST, &events);

    q.finish();

    bool success = true;
    for (size_t i = 0; i < length; i++) {
        float expected = data_a[i]*data_x[i] + data_y[i];
        if (std::fabs(expected - data_out[i]) >= 1e-6) {
            success = false;
            fprintf(stderr, "Mismatch at index %lu. Expected %f but got %f\n",
                    i, expected, data_out[i]);
        }
    }

    if (success) {
        printf("Success!\n");
        return 0;
    }

    return 1;
}

int create_vector(std::vector<float, aligned_allocator<float>> &vec, size_t length)
{
    vec.resize(length);
    for (size_t i = 0; i < length; i++) {
        vec[i] = i % 256;
    }

    return 0;
}

int read_xclbin(std::string const& fname, std::vector<char> &binary)
{
    std::ifstream bin(fname, std::ifstream::binary);
    bin.seekg(0, bin.end);
    long nb = bin.tellg();
    bin.seekg(0, bin.beg);

    binary.resize(nb);
    bin.read(binary.data(), nb);

    return 0;
}
