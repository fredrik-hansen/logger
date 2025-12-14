#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include "std_libc.h"

#ifndef RTLD_DEFAULT
#  define RTLD_DEFAULT ((void *) 0)
#endif

/* this is test case */
int main(int argc, char **argv)
{
	struct timespec ts;

	if (argc < 2) {
		printf("Usage: %s <program> [ arg1 arg2 ... argN ]\n", argv[0]);
		exit(1);
	}

	void *handle = dlopen(STD_LIBC_PATH, RTLD_LAZY);
	if (handle == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n", STD_LIBC_PATH, dlerror());
		exit(1);
	}

	/* simple test to see if the func in memory matches libc.so.6 */
	if (dlsym(handle, "execve") == dlsym(RTLD_DEFAULT, "execve")) {
		/* not overridden, setup LD_PRELOAD */
		if (setenv("LD_PRELOAD", LOGGER_PATH "/logger.so", 1) != 0) {
			perror("setenv");
			exit(1);
		}

		/* Sleep for 100ms */
		ts.tv_sec = 0;
		ts.tv_nsec = 100000000L;
		nanosleep(&ts, NULL);

		printf("setting up LD_PRELOAD (ctrl + c to escape)\n");
		execv(argv[0], argv);
		perror("execv");
		exit(1);
	}

	printf("execute: %s\n", argv[1]);
	execv(argv[1], argv + 1);
	perror("execv");

	return 1;
}

