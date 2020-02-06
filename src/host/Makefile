CC := $(CROSS_COMPILE)g++
CCFLAGS += -std=c++11 -Wall -Wpedantic -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include
LDFLAGS += -lOpenCL -L$(XILINX_XRT)/lib
NAME := host
SRCS := host.cpp
OBJS := $(SRCS:.cpp=.o)

ifneq ($(SYSROOT),)
    CCFLAGS += --sysroot $(SYSROOT)
    LDFLAGS += --sysroot $(SYSROOT)
endif

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -c -o $@ $< $(CCFLAGS)

clean:
	rm -rf $(NAME) $(OBJS)

.PHONY: all clean
