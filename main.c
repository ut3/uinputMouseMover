/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright 2020 J Rick Ramstetter, rick.ramstetter@gmail.com
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Jitter your Wayland mouse cursor ever second using libuinput 
 * device. Adapted from:
 * https://www.kernel.org/doc/Documentation/input/uinput.rst
 */

#if BEGIN_MAKEFILE_EXAMPLE
=====================================================================
HEADERS=/usr/linux-headers-5.7.0-2-common
PLATFORM_HEADERS=/usr/src/linux-headers-5.7.0-2-amd64

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
=====================================================================
#endif // END_MAKEFILE_EXAMPLE

#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define CHECK(x) { \
   	int rv = 0; \
	errno = 0; \
	do { rv = (x); } while(0); \
	if (0 != errno || rv < 0) { \
		printf("%s: %s (errno=%d, rv=%d)\n",#x, strerror(errno), errno, rv); \
	   	exit(errno); \
   	}\
}

static int g_fd; 

void emit(int type, int code, int val)
{
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;

	ie.time.tv_sec = 0;
	ie.time.tv_usec = 0;

	CHECK(write(g_fd, &ie, sizeof(ie)));
}

void cleanup(int sig) {
	printf("Cleanup fd %d\n", g_fd); 
	ioctl(g_fd, UI_DEV_DESTROY);
   	close(g_fd);
	exit(sig);
}

void handler(int sig) { cleanup(sig); }


int main(void)
{
   struct uinput_setup usetup;
   int flip = 0;
   struct sigaction sighandle;
   memset(&sighandle, 0, sizeof(sighandle));

   sighandle.sa_handler = handler;
   sighandle.sa_flags = SA_RESTART;
   sigaction(SIGINT, &sighandle, 0);
   sigaction(SIGKILL, &sighandle, 0);
   sigaction(SIGTERM, &sighandle, 0);

   CHECK(g_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK));

   CHECK(ioctl(g_fd, UI_SET_EVBIT, EV_KEY));
   CHECK(ioctl(g_fd, UI_SET_KEYBIT, BTN_LEFT));

   CHECK(ioctl(g_fd, UI_SET_EVBIT, EV_REL));
   CHECK(ioctl(g_fd, UI_SET_RELBIT, REL_X));
   CHECK(ioctl(g_fd, UI_SET_RELBIT, REL_Y));

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0xDEAD;
   usetup.id.product = 0xBEEF;
   strcpy(usetup.name, "Mouse mover");

   CHECK(ioctl(g_fd, UI_DEV_SETUP, &usetup));
   CHECK(ioctl(g_fd, UI_DEV_CREATE));

   sleep(1);

   for(;;) {
      emit(EV_REL, REL_X, flip ? -10 : 10);
      emit(EV_REL, REL_Y, flip ? -10 : 10);
      emit(EV_SYN, SYN_REPORT, 0);

	  emit(EV_KEY, BTN_LEFT, 1);
      emit(EV_SYN, SYN_REPORT, 0);
	  usleep(1000);
	  emit(EV_KEY, BTN_LEFT, 0);
      emit(EV_SYN, SYN_REPORT, 0);
	  flip = flip ? 0 : 1;
      sleep(2);
   }

   cleanup(0); 
}
