/* logger.c -- execve() logging wrapper
 * Copyright (c) Fredrik Hansen
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
 *
 * This lib overloads execv(), dumps content to syslog() and returns to execv()
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <syslog.h>
#include <sys/statvfs.h> /* Required for disk space check */

extern char *getlogin(void);

#define min(a,b) ((a) < (b) ? (a) : (b))

/* Log only the actions running under uid 0, set 1 to enable
 * If this is set to 1, the NO_SYSTEM option has no effect.
 */
#define UID0_ONLY 0 

/* Don't log users with UIDs below NO_SYSTEM. Set to 500 for a default linux to skip service logging.
 * Example: #define NO_SYSTEM 500
 * This will only log users with an uid > 500
 * Useful if theres alot of scripts/cron's running.
 */
#define NO_SYSTEM 0

/* Track and deny all attempts to unset LD_PRELOAD. 0 = off, 1 = on
 * (code not finished)
 */
#define PROTECT_LOGGER 0

/* Define the minimum amount of disk space (in KB) required to write logs.
 * If the disk has less space than this, logging will be skipped.
 */
#define MIN_DISK_SPACE_KB 1024  /* 1MB */

/* MAX
 * maximum size of any argument.  if set to 0, logger will ensure that all
 * arguments get logged to its full lenght; this is also a slower process
 */
#define MAX 250

/* MAX_ARGC
 * maximum number of arguments to process (safety limit)
 */
#define MAX_ARGC 1024

#if defined(RTLD_NEXT)
#  define REAL_LIBC RTLD_NEXT
#else
#  define REAL_LIBC ((void *) -1L)
#endif

#define FN(ptr,type,name,args)  ptr = (type (*)args)dlsym(REAL_LIBC, name)

static inline void logger(const char *filename, char **argv) {
	static char *logstring = NULL;
	static int argc, size = 0;
	register int i, spos = 0;
	char *login;
	struct statvfs vfs;

	/* Check disk space before logging */
	if (statvfs("/", &vfs) == 0) {
		unsigned long available_space = (unsigned long)vfs.f_bavail * (unsigned long)vfs.f_frsize;
		if (available_space / 1024 < MIN_DISK_SPACE_KB) {
			/* Disk space is too low, skip logging */
			return;
		}
	} else {
		/* Error getting disk space, skip logging to avoid crashes */
		return;
	}

	#if UID0_ONLY
	if (getuid() != 0)
		return;
	#endif
	#if NO_SYSTEM
	if (getuid() < NO_SYSTEM)
		return;
	#endif

	/* Count arguments with bounds checking */
	for (argc = 0; argv && argv[argc] != NULL && argc < MAX_ARGC; argc++);

	if (argc == 0)
		return;

	openlog("logger", LOG_PID, LOG_AUTHPRIV);

	#if MAX
	logstring = (char *)malloc(sizeof(char) * MAX * argc);
	if (logstring == NULL) {
		syslog(LOG_ERR, "logger: malloc failed");
		closelog();
		return;
	}
	for (i = 0; i < argc; i++)
		spos += min(snprintf(logstring + spos, MAX, "%s ", argv[i]), MAX);
	#else
	for (i = 0; i < argc; i++)
		size += sizeof(char) * strlen(argv[i]) + 1;
	size++; /* make space for that \0 */
	logstring = (char *)malloc(sizeof(char) * size);
	if (logstring == NULL) {
		syslog(LOG_ERR, "logger: malloc failed");
		closelog();
		return;
	}

	for (i = 0; i < argc; i++)
		spos += sprintf(logstring + spos, "%s ", argv[i]);
	#endif
	login = getlogin();
	syslog(LOG_INFO, "%s [uid:%d sid:%d]: %s", (login) ? login : "<null>", getuid(), getsid(0), logstring);
	free(logstring);
	closelog();
}

/* Override execve */
int execve(const char *filename, char **argv, char **envp) {
	static int (*func)(const char *, char **, char **);

	FN(func, int, "execve", (const char *, char **, char **));
	logger(filename, argv);

	return (*func)(filename, argv, envp);
}

/* Override execv */
int execv(const char *filename, char **argv) {
	static int (*func)(const char *, char **);

	FN(func, int, "execv", (const char *, char **));
	logger(filename, argv);

	return (*func)(filename, argv);
}
