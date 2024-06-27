#ifndef GUARD_QUAKE_VIDEO_H
#define GUARD_QUAKE_VIDEO_H

#include <stdbool.h>
#include "types/Byte.h"

struct Video {
	Byte *buffer;
	Byte *colormap;
	Byte *alphamap;
	int rowBytes;
	int width;
	int height;
	int: 32;
	long: 64;
	long: 64;
	long: 64;
}; // viddef_t

void VID_NewVideo(int width, int height);
bool VID_GetModeInfo(int *width, int *height, int mode);

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/client/video.h

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
