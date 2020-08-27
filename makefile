HEADERS=/usr/linux-headers-5.7.0-2-common
PLATFORM_HEADERS=/usr/src/linux-headers-5.7.0-2-amd64
# OK if HEADERS == PLATFORM_HEADERS

CC=gcc
CFLAGS+=-I$(PLATFORM_HEADERS)/arch/x86/include/generated/uapi
CFLAGS+=-I$(HEADERS)/include/uapi/
CFLAGS+=-I$(HEADERS)/arch/x86/include/uapi
CFLAGS+=-I$(HEADERS)/include
LDFLAGS+=-static

mover: main.c
	$(CC) -o mover main.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o mover
