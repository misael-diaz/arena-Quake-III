#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "graphics.h"

extern int r_cnumsurfs;

static struct Surface *surfaces = NULL;
static struct Surface *surf_max = NULL;

struct CacheSurface *cs_base = NULL;
struct CacheSurface *cs_rover = NULL;
int cs_size = 0;

// this is our way of defining surface of index 0 as a dummy surface
struct Surface *Surface_GetSurface (int const idx)
{
	if (!surfaces || !surf_max) {
		Q_Shutdown();
		fprintf(stderr, "Surface_GetSurface: NullSurfaceError\n");
		exit(EXIT_FAILURE);
	}

	if (idx <= 0) {
		Q_Shutdown();
		fprintf(stderr, "Surface_GetSurface: InvalidSurfaceIndex\n");
		exit(EXIT_FAILURE);
	}

	int const surfnum = (idx - 1);
	if ((surfaces + surfnum) >= surf_max) {
		Q_Shutdown();
		fprintf(stderr, "Surface_GetSurface: InvalidSurfaceIndex\n");
		exit(EXIT_FAILURE);
	}

	return &surfaces[surfnum];
}

// TODO: rename Driver_FlushCaches -> Surface_FlushCaches
//       in the main code define Driver_FlushCaches as a proxy
void Driver_FlushCaches (void)
{
	if (!cs_base) {
		return;
	}

	// for now we don't expect to get here yet
	Q_Shutdown();
	fprintf(stderr, "D_FlushCaches: ImpError\n");
	exit(EXIT_FAILURE);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/surface.c

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
