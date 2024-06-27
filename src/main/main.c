#include <stdio.h>

#include "util.h"
#include "graphics.h"

int main ()
{
	printf("quake\n");
	Graphics_Init();
	Graphics_EndFrame();
	for (int i = 0; i != 256; ++i) {
		Graphics_BeginFrame();
		Graphics_EndFrame();
	}
	Graphics_Free();
	Util_Clear();
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
