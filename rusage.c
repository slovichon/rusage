/* $Id$ */
/*
 * Written by Jared Yanovich
 * This file belongs to the public domain.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void usage(void) __attribute__((__noreturn__));

int
main(int argc, char *argv[])
{
	struct timeval *rt, *ut, *st, tb, ta;
	struct rusage ru;
	int status;

	if (argc < 2)
		usage();

	(void)gettimeofday(&tb, (struct timezone *)NULL);
	switch (fork()) {
	case -1:
		err(EX_OSERR, "fork");
		/* NOTREACHED */
	case 0:
		(void)execvp(argv[1], argv + 1);
		err(EX_OSERR, "execve %s", argv[1]);
		/* NOTREACHED */
	}
	(void)wait(&status);
	(void)gettimeofday(&ta, (struct timezone *)NULL);

	if (getrusage(RUSAGE_CHILDREN, &ru) == -1)
		err(EX_OSERR, "getrusage");
	ut = &ru.ru_utime;
	st = &ru.ru_stime;
	rt = &ta;
	rt->tv_sec -= tb.tv_sec;
	rt->tv_usec -= tb.tv_usec;
	if (ta.tv_usec < 0) {
		rt->tv_usec += 1000 * 1000;
		rt->tv_sec--;
	}
	(void)printf("%ld.%02ld real ", rt->tv_sec, rt->tv_usec / 1000);
	(void)printf("%ld.%02ld user ", st->tv_sec, st->tv_usec / 1000);
	(void)printf("%ld.%02ld sys ",  st->tv_sec, st->tv_usec / 1000);
	(void)printf("%ld pf ",  ru.ru_majflt);	 /* page faults */
	(void)printf("%ld pr ",  ru.ru_minflt);	 /* page reclaims */
	(void)printf("%ld sw ",  ru.ru_nswap);	 /* swaps */
	(void)printf("%ld rb ",  ru.ru_inblock); /* blocks read */
	(void)printf("%ld wb ",  ru.ru_oublock); /* blocks wrote */
	(void)printf("%ld vcx ", ru.ru_nvcsw);	 /* voluntary ctxsws */
	(void)printf("%ld icx ", ru.ru_nivcsw);	 /* involuntary ctxsws */
	(void)printf("%ld mx ",  ru.ru_maxrss);	 /* max res set size */
	(void)printf("%ld ix ",  ru.ru_ixrss);	 /* text */
	(void)printf("%ld id ",  ru.ru_idrss);	 /* data */
	(void)printf("%ld is\n", ru.ru_isrss);	 /* stack */
	exit(status);
}

void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s command [argument ...]\n",
	    __progname);
	exit(EX_USAGE);
}
