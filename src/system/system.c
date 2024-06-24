#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "util.h"
#include "system.h"

int64_t Sys_ClockNanoSeconds (void)
{
	struct timespec tp;
	int err = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	if (err) {
		fprintf(stderr, "Sys_ClockNanoSeconds: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}
	int64_t time = ((1000000000L * tp.tv_sec) + tp.tv_nsec);
	return time;
}

int64_t Sys_ElapsedTime (struct timespec *tp_start, struct timespec *tp_end)
{
	int64_t etime = (((int64_t) (1.0e9)) * (tp_end->tv_sec - tp_start->tv_sec) +
			(tp_end->tv_nsec - tp_start->tv_nsec));
	return etime;
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
