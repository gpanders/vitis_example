CC := aarch64-linux-gnu-g++
CCFLAGS += -std=c++11 -g -Wall -Wpedantic -D__USE_XOPEN2K8 --sysroot $(SYSROOT) -I$(SYSROOT)/usr/include/xrt
LDFLAGS += -lxilinxopencl -lpthread -lrt -lxrt_core -lgmp --sysroot $(SYSROOT)
NAME := host
SRCS := host.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS) | check-sysroot
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp | check-sysroot
	$(CC) -c -o $@ $< $(CCFLAGS)

clean:
	rm -rf $(NAME) $(OBJS)

check-sysroot:
    ifndef SYSROOT
	$(error "SYSROOT is undefined. Set it as an environment variable or provide it on the commandline invocation of make")
    endif

.PHONY: all clean
