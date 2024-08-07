#include "common.h"
#include "video.h"
#include "graphics.h"
#include "client.h"

extern struct Video Video;

static struct CtrlVar *screen_viewsize = NULL;

struct ViewRectangle screen_viewRectangle;

static bool screen_initialized = false;

void Screen_DrawLoading (void)
{
	return;
}

void Screen_DrawCrossHair (void)
{
	return;
}

void Screen_ClearTile (void)
{
	// nothing to do because our viewsize is 100%
	return;
}

void Screen_CalculateViewRectangle (void)
{
	screen_viewRectangle.width = Video.width;
	screen_viewRectangle.height = Video.height;
	screen_viewRectangle.x = 0;
	screen_viewRectangle.y = 0;
}

void Screen_UpdateScreen (void)
{
	if (!screen_initialized) {
		return;
	}

	Refresh_BeginFrame();
	Screen_CalculateViewRectangle();
	Screen_ClearTile();
	View_RenderView();
	Screen_DrawLoading();
	Refresh_EndFrame();
}

void Screen_Init (void)
{
	screen_initialized = true;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/screen.c

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
