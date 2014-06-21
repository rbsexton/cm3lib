CFLAGS+=-I/opt/local/include

all: cunit
cunit: ringbuffer.o ringbuffer-cunit.o
	cc -o cunit ringbuffer.o ringbuffer-cunit.o -L/opt/local/lib -lcunit

