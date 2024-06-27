#include "video.h"

struct VideoMode {
        const char *description;
        int width;
	int height;
        int mode;
	char pad[8];
} vidmode_t;

struct VideoMode VideoModes[] = {
        {.description = "Mode 0: 320x240",   .width =  320, .height =  240,  .mode = 0},
        {.description = "Mode 1: 400x300",   .width =  400, .height =  300,  .mode = 1},
        {.description = "Mode 2: 512x384",   .width =  512, .height =  384,  .mode = 2},
        {.description = "Mode 3: 640x480",   .width =  640, .height =  480,  .mode = 3},
        {.description = "Mode 4: 800x600",   .width =  800, .height =  600,  .mode = 4},
        {.description = "Mode 5: 960x720",   .width =  960, .height =  720,  .mode = 5},
        {.description = "Mode 6: 1024x768",  .width = 1024, .height =  768,  .mode = 6},
        {.description = "Mode 7: 1152x864",  .width = 1152, .height =  864,  .mode = 7},
        {.description = "Mode 8: 1280x1024", .width = 1280, .height = 1024,  .mode = 8},
        {.description = "Mode 9: 1600x1200", .width = 1600, .height = 1200,  .mode = 9}
};

struct Video Video;

void VID_NewVideo (int width, int height)
{
	Video.width = width;
	Video.height = height;
}

bool VID_GetModeInfo (int *width, int *height, int mode)
{
	int const vid_num_modes = (sizeof(VideoModes) / sizeof(VideoModes[0]));
	if (mode < 0 || mode > vid_num_modes) {
		return false;
	}

	*width = VideoModes[mode].width;
	*height = VideoModes[mode].height;
	return true;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/video/video.c

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
