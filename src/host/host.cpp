#include <iostream>
#include <fstream>
#include <cmath>

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include <CL/cl2.hpp>

#define KERNEL "axpy"

int read_vector(std::string const& fname, std::vector<float> &vec);
int read_xclbin(std::string const& fname, std::vector<char> &binary);

int main(int argc, char *argv[])
{
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " <xclbin> <data_a.txt> <data_x.txt> <data_y.txt>";
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    int err;
    std::vector<char> binary;
    std::vector<float> data_a, data_x, data_y;

    err = read_xclbin(argv[1], binary);
    if (err) {
        return err;
    }

    err = read_vector(argv[2], data_a);
    if (err) {
        return err;
    }

    err = read_vector(argv[3], data_x);
    if (err) {
        return err;
    }

    err = read_vector(argv[4], data_y);
    if (err) {
        return err;
    }

    size_t length = data_a.size();
    size_t size_bytes = length * sizeof(float);

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

    cl::Program::Binaries bins {{binary.data(), binary.size()}};
    devices.resize(1);
    cl::Program program(context, devices, bins);

    cl::Kernel kernel(program, KERNEL);

    cl::Buffer buffer_a(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_a.data());
    cl::Buffer buffer_x(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_x.data());
    cl::Buffer buffer_y(context, CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR, size_bytes, data_y.data());
    cl::Buffer buffer_out(context, CL_MEM_WRITE_ONLY, size_bytes);

    kernel.setArg(0, buffer_a);
    kernel.setArg(1, buffer_x);
    kernel.setArg(2, buffer_y);
    kernel.setArg(3, buffer_out);
    kernel.setArg(4, length);

    float *result = (float *) q.enqueueMapBuffer(buffer_out, CL_TRUE, CL_MAP_READ, 0, size_bytes);

    q.enqueueMigrateMemObjects({buffer_a, buffer_x, buffer_y}, 0);

    q.enqueueTask(kernel);

    q.enqueueMigrateMemObjects({buffer_out}, CL_MIGRATE_MEM_OBJECT_HOST);

    q.finish();

    bool success = true;
    for (size_t i = 0; i < length; i++) {
        float expected = data_a[i]*data_x[i] + data_y[i];
        if (std::fabs(expected - result[i]) >= 1e-6) {
            success = false;
            fprintf(stderr, "Mismatch at index %lu. Expected %f but got %f\n",
                    i, expected, result[i]);
        }
    }

    q.enqueueUnmapMemObject(buffer_out, result);
    q.finish();

    if (success) {
        printf("Success!\n");
        return 0;
    }

    return 1;
}

int read_vector(std::string const& fname, std::vector<float> &vec)
{
    std::ifstream fil(fname);
    float x;
    while (fil >> x) {
        vec.push_back(x);
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
