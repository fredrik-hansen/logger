/* detect.c -- simple execve() wrapper detection 
 * $Id: detect.c,v 1.4 2000/11/04 22:20:52 mbm Exp $
 * Copyright (c) 2000 marius@linux.com,mbm@linux.com
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <dlfcn.h>
#include "std_libc.h"

#ifndef RTLD_DEFAULT
#  define RTLD_DEFAULT ((void *) 0)
#endif

int main(void) {
	void *handle = dlopen(STD_LIBC_PATH, RTLD_LAZY);
	if (handle == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n", STD_LIBC_PATH, dlerror());
		return 1;
	}

	/* simple test to see if the execve in memory matches libc.so.6 */
	if (dlsym(handle, "execve") != dlsym(RTLD_DEFAULT, "execve"))
		printf("logger successfully loaded\n");
	else
		printf("logger NOT loaded\n");

	dlclose(handle);
	return 0;
}
