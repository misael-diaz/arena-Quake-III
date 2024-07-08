#include <stdio.h>
#include <assert.h>
#include <endian.h>

#include "util.h"
#include "common.h"
#include "system.h"
#include "graphics.h"

static void asserts (void)
{
	static_assert(sizeof(struct DataVisibility) == 68);
	static_assert(MAX_MAP_VISIBILITY % sizeof(struct DataVisibility) == 0);
	static_assert(__BYTE_ORDER == __LITTLE_ENDIAN);
	static_assert(__FLOAT_WORD_ORDER == __LITTLE_ENDIAN);
	static_assert(sizeof(struct Vector) == 12);
	static_assert(sizeof(struct ModelNode) == sizeof(struct ModelLeaf));
	static_assert(sizeof(struct ModelNode) == 80);
	static_assert(sizeof(struct CVar) == 64);
	static_assert(sizeof(int) == 4);
	static_assert(sizeof(float) == 4);
	static_assert(sizeof(long) == 8);
	static_assert(sizeof(Byte) == 1);
}

int main ()
{
	asserts();
	printf("quake\n");
	FS_Init();
	CM_LoadMap("maps/demo1.bsp");
	/*
	Refresh_Init();
	Refresh_EndFrame();
	for (int i = 0; i != 4; ++i) {
		Sys_DelayMillis();
		Refresh_BeginFrame();
		Refresh_EndFrame();
	}
	*/
	Q_Shutdown();
	return 0;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/main/main.c

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
