#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "video.h"
#include "graphics.h"
#include "util.h"

extern Display *display;
extern struct Video Video;
extern struct Vector3D modelorg;
extern int mod_registration_sequence;

struct CVar *fullbright = NULL;
struct GraphState GraphState;
struct OldReferenceDefinition oldRefDef;
struct Vector3D origin;
struct OldReferenceDefinition oldRefDef;
int framecount = 1;

static struct CVar *vid_gamma;
static struct CVar *vid_fullscreen;
static struct CVar *graphics_mode;
static struct Image *r_notexture_mip = NULL;
static struct ClipPlane view_clipplanes[4];
static unsigned int curpalette[256];
static Byte r_notexture_buffer[1024];
static float r_aliasuvscale = 1.0f;

void Graphics_SetupFrame (void)
{
	if (fullbright->modified) {
		fullbright->modified = false;
		D_FlushCaches();
	}

	++framecount;

	struct ViewRectangle viewRectangle;
	Vector3DCopy(&oldRefDef.vieworg, &modelorg);
	Vector3DCopy(&oldRefDef.vieworg, &origin);

	// TODO: there's a lot to do here still
}

void Graphics_RenderFrame (struct ReferenceDefinition *RD)
{
	struct ReferenceDefinition newRD = *RD;
	Vector3DCopy(&RD->vieworg, &newRD.vieworg);
	Vector3DCopy(&RD->viewangles, &newRD.viewangles);

	// TODO: there's also a lot to do here
}

void Graphics_Register (void)
{
	fullbright = CVAR_GetCVar("fullbright", "0", 0);
	vid_gamma = CVAR_GetCVar("vid_gamma", "1", 0);
	vid_fullscreen = CVAR_GetCVar("vid_fullscreen", "0", 0);
	graphics_mode = CVAR_GetCVar("graphics_mode", "8", 0);

	vid_gamma->modified = true;
	graphics_mode->modified = true;
}

void Graphics_InitTextures (void)
{
	r_notexture_mip = (struct Image*) r_notexture_buffer;
	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->pixels[0] = &r_notexture_buffer[sizeof(struct Image)];
	r_notexture_mip->pixels[1] = r_notexture_mip->pixels[0] + 16 * 16;
	r_notexture_mip->pixels[2] = r_notexture_mip->pixels[1] + 8 * 8;
	r_notexture_mip->pixels[3] = r_notexture_mip->pixels[2] + 4 * 4;
	for (int n = 0; n < 4; ++n) {
		Byte *dst = r_notexture_mip->pixels[n];
		int const height = (16 >> n);
		for (int y = 0; y < height; ++y) {
			int const width = (16 >> n);
			for (int x = 0; x < width; ++x) {
				if ((y < (8 >> n)) ^ (x < (8 >> n))) {
					*dst = 0;
				} else {
					*dst = 255;
				}
				++dst;
			}
		}
	}
}

void Graphics_InitImages (void)
{
	mod_registration_sequence = 1;
}

void Graphics_EndFrame (void)
{
	Graphics_ImpEndFrame();
}

void Graphics_Shutdown (void)
{
	Graphics_ImpShutdown();
}

void Graphics_Free (void)
{
	Graphics_ImpShutdown();
	if (display) {
		XCloseDisplay(display);
		display = NULL;
	}
}

void Graphics_GammaCorrectAndSetPalette (Byte const *palette)
{
	Byte const *gammatable = (Byte const*) GraphState.gammatable;
	Byte *currentpalette = GraphState.currentpalette;
	for (int i = 0; i != 256; ++i) {
		currentpalette[4 * i + 0] = gammatable[palette[4 * i + 0]];
		currentpalette[4 * i + 1] = gammatable[palette[4 * i + 1]];
		currentpalette[4 * i + 2] = gammatable[palette[4 * i + 2]];
	}

	Graphics_ImpSetPalette(currentpalette);
}

void Graphics_InitGraphics (int width, int height)
{
	if (width == 0 || height == 0) {
		fprintf(stderr, "Graphics_InitGraphics: InitError");
		return;
	}

	Video.width = width;
	Video.height = height;

	Graphics_GammaCorrectAndSetPalette((Byte const*) curpalette);
}

void Graphics_BeginFrame (void)
{
	if (vid_gamma->modified) {
		Draw_BuildGammaTable();
		Graphics_GammaCorrectAndSetPalette((Byte const*) curpalette);
		vid_gamma->modified = false;
	}

	if (graphics_mode->modified) {
		enum GraphicsErrorType err = Graphics_ImpSetMode(&Video.width,
								 &Video.height,
								 graphics_mode->data,
								 vid_fullscreen->data);
		if (err) {
			fprintf(stderr, "Graphics_BeginFrame: SetModeError\n");
			Graphics_Shutdown();
			XCloseDisplay(display);
			Util_Clear();
			exit(EXIT_FAILURE);
		}
		Graphics_InitGraphics(Video.width, Video.height);
		GraphState.prev_mode = graphics_mode->data;
		vid_fullscreen->modified = false;
		graphics_mode->modified = false;
	}
}

void Graphics_Init (void)
{
	Graphics_InitImages();
	MOD_Init();
	Draw_InitLocal();
	Graphics_InitTextures();

	view_clipplanes[0].leftedge = true;	view_clipplanes[0].rightedge = false;
	view_clipplanes[1].leftedge = false;	view_clipplanes[1].rightedge = true;
	view_clipplanes[2].leftedge = false;	view_clipplanes[2].rightedge = false;
	view_clipplanes[3].leftedge = false;	view_clipplanes[3].rightedge = false;

	oldRefDef.xOrigin = XCENTERING;
	oldRefDef.yOrigin = YCENTERING;

	r_aliasuvscale = 1.0f;

	Graphics_Register();
	Draw_GetPalette();
	Graphics_ImpInit();

	Graphics_BeginFrame();
	Draw_PatchQuakePalette();
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/main.c

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
