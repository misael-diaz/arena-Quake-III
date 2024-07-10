#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "util.h"
#include "system.h"

long Sys_ClockNanoSeconds (void)
{
	struct timespec tp;
	int err = clock_gettime(CLOCK_MONOTONIC, &tp);
	if (err) {
		fprintf(stderr, "Sys_ClockNanoSeconds: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}
	long time = ((1000000000L * tp.tv_sec) + tp.tv_nsec);
	return time;
}

long Sys_ClockMilliSeconds (void)
{
	struct timespec tp;
	int err = clock_gettime(CLOCK_MONOTONIC, &tp);
	if (err) {
		fprintf(stderr, "Sys_ClockMilliSeconds: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}
	long time = ((1000L * tp.tv_sec) + (tp.tv_nsec / 1000000L));
	return time;
}

long Sys_ElapsedTime (struct timespec *tp_start, struct timespec *tp_end)
{
	long etime = (((long) (1.0e9)) * (tp_end->tv_sec - tp_start->tv_sec) +
			(tp_end->tv_nsec - tp_start->tv_nsec));
	return etime;
}

void Sys_DelayMillis (void)
{
	struct timespec tp;
	clockid_t clockid = CLOCK_MONOTONIC;
	int const err = clock_gettime(clockid, &tp);
	if (err) {
		fprintf(stderr, "Sys_DelayMillisec: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	do {
		time_t const sec = tp.tv_sec;
		long const nsec = (tp.tv_nsec + 1000000L);
		struct timespec tp_target;
		if (nsec > 999999999L) {
			tp_target.tv_sec = (1 + sec);
			tp_target.tv_nsec = (nsec % 999999999L);
		} else {
			tp_target.tv_sec = sec;
			tp_target.tv_nsec = nsec;
		}
		int const flags = TIMER_ABSTIME;
		int const err = clock_nanosleep(clockid, flags, &tp_target, NULL);
		if (err && err != EINTR) {
			fprintf(stderr, "Sys_DelayMillisec: unexpected error %d\n", err);
			exit(EXIT_FAILURE);
		}
	} while (err == EINTR);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/system/system.c

Copyright (C) 2024 Misael Díaz-Maldonado

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/
