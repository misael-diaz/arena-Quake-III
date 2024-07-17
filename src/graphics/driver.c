#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "video.h"
#include "graphics.h"
#include "game.h"
#include "util.h"

extern struct Video Video;
extern struct OldRefresh oldRefDef;
extern struct Refresh refresh;
extern int r_screenwidth;
extern float xscale;
extern float yscale;

struct CacheSurface *d_initial_rover = NULL;
Byte *d_viewbuffer = NULL;
// NOTE: if you change the type of d_pzbuffer you must also update d_zrowbytes accordingly
short *d_pzbuffer = NULL;
short *d_zspantable[MAXHEIGHT];

int d_scantable[MAXHEIGHT];
unsigned int d_8to24table[256];
float d_scalemip[NUM_MIPS - 1];
float d_scale_for_mip = 0;
int d_zrowbytes = 0;
int d_zwidth = 0;
int d_pix_min = 0;
int d_pix_max = 0;
int d_pix_shift = 0;
int d_viewRectX = 0;
int d_viewRectY = 0;
int d_spanpixcount = 0;
int d_viewRectRight_particle = 0;
int d_viewRectBottom_particle = 0;
int d_aflatcolor = 0;
int d_minmip = 0;
bool d_roverwrapped = false;

void Driver_Patch (void)
{
	return;	// no driver patch required for X86_64
}

void Driver_ViewChanged (void)
{
	d_scale_for_mip = (yscale > xscale)? yscale : xscale;

	if (!refresh.width || !refresh.height) {
		Q_Shutdown();
		fprintf(stderr, "Driver_ViewChanged: InitNewRefDefError\n");
		exit(EXIT_FAILURE);
	}

	if (!d_pzbuffer) {
		Q_Shutdown();
		fprintf(stderr, "Driver_ViewChanged: NullZBufferError\n");
		exit(EXIT_FAILURE);
	}

	d_zrowbytes = Video.width * sizeof(*d_pzbuffer);
	d_zwidth = Video.width;

	// note that 320 is the smallest supported video mode width
	d_pix_min = (oldRefDef.viewRect.width / 320);
	if (d_pix_min < 1) {
		d_pix_min = 1;
	}

	d_pix_max = ((int) (((float) oldRefDef.viewRect.width) / (320.0f / 4.0f) + 0.5f));
	if (d_pix_max < 1) {
		d_pix_max = 1;
	}

	// NOTE: might break the code if we attempt an unsupported video mode
	d_pix_shift = 8 - ((int) (((float) oldRefDef.viewRect.width) / 320.0f + 0.5f));
	if (oldRefDef.viewRect.width > 1600) {
		Q_Shutdown();
		fprintf(stderr, "Driver_ViewChanged: UnsupportedVideoMode\n");
		exit(EXIT_FAILURE);
	}

	d_viewRectX = oldRefDef.viewRect.x;
	d_viewRectY = oldRefDef.viewRect.y;
	d_viewRectRight_particle = oldRefDef.viewRectRight - d_pix_max;
	d_viewRectBottom_particle = oldRefDef.viewRectBottom - d_pix_max;

	for (int i = 0; i != Video.height; ++i) {
		d_scantable[i] = i * r_screenwidth;
		d_zspantable[i] = d_pzbuffer + i * d_zwidth;
	}

	if (refresh.RDFlags & RDF_NOWORLDMODEL) {
		int d_pzbuffer_sz = Video.width * Video.height * sizeof(*d_pzbuffer);
		memset(d_pzbuffer, 0xff, d_pzbuffer_sz);
		int black = 0;
		int color = black;
		Draw_Fill(refresh.x, refresh.y, refresh.width, refresh.height, color);
	}

	Driver_Patch();
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/driver.c

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
