#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#define TARGET_FREQUENCY_HZ 60.0
#define TARGET_PERIOD_NANOSEC ((int64_t)((1.0e9 * (1.0 / TARGET_FREQUENCY_HZ))))

void Game_Loop (void)
{
	clockid_t clockid = CLOCK_MONOTONIC;
	printf("period: %"PRId64"\n", TARGET_PERIOD_NANOSEC);
	size_t num_cycles = 256;
	struct timespec tp_start;
	clock_gettime(clockid, &tp_start);
	for (size_t i = 0; i != num_cycles; ++i) {

		struct timespec tp;
		int const err = clock_gettime(clockid, &tp);
		if (err) {
			fprintf(stderr, "game_loop: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		do {
			time_t const sec = tp.tv_sec;
			int64_t const nsec = (tp.tv_nsec + TARGET_PERIOD_NANOSEC);
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
				fprintf(stderr, "game_loop: unexpected error %d\n", err);
				exit(EXIT_FAILURE);
			}
		} while (err == EINTR);
	}
	struct timespec tp_end;
	clock_gettime(clockid, &tp_end);

	int64_t etime_nsec = elapsed_time(tp_start, tp_end);
	double etime_sec = (1.0e-9 * ((double) etime_nsec));
	printf("etime: %"PRId64"\n", etime_nsec);
	printf("rate: %lf\n", num_cycles / etime_sec);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/game/game.c

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
