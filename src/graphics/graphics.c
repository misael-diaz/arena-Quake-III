#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <errno.h>

#include "video.h"
#include "graphics.h"
#include "util.h"

extern struct Video Video;

Display *display = NULL;

static GC gc;
static Window window;
static XSetWindowAttributes windowAttributes;
static Visual *visual = NULL;
static XVisualInfo *visualInfo = NULL;
static XImage *framebuffer = NULL;
static uint32_t curpalette[256];
static uint64_t bitshift_red = 0;
static uint64_t bitshift_green = 0;
static uint64_t bitshift_blue = 0;
static bool bitshifts_initialized = false;
static bool X11Active = false;

static void shiftmasks_init (void)
{
	if (!visual) {
		Util_Clear();
		fprintf(stderr, "shiftmasks_init: NullVisualError\n");
		exit(EXIT_FAILURE);
	}

	bitshift_blue = 0;
	for (uint64_t x = 0xff; x < visual->blue_mask; bitshift_blue += 8) {
		x <<= 8;
	}

	bitshift_green = 0;
	for (uint64_t x = 0xff; x < visual->green_mask; bitshift_green += 8) {
		x <<= 8;
	}

	bitshift_red = 0;
	for (uint64_t x = 0xff; x < visual->red_mask; bitshift_red += 8) {
		x <<= 8;
	}

	bitshifts_initialized = true;
}

unsigned int Graphics_RGB (unsigned long r, unsigned long g, unsigned long b)
{
	if (!bitshifts_initialized) {
		shiftmasks_init();
	}

	uint64_t p = 0;
	p |= ((r << bitshift_red) & visual->red_mask);
	p |= ((g << bitshift_green) & visual->green_mask);
	p |= ((b << bitshift_blue) & visual->blue_mask);
	return (p & 0x00000000ffffffff);
}

void Graphics_ImpInit (void)
{
	display = XOpenDisplay(0);
	if (!display) {
		char const *envDisplay = getenv("DISPLAY");
		if (envDisplay) {
			char errmsg[] = "Graphics_ImpInit: failed to open display [%s]\n";
			fprintf(stderr, errmsg, envDisplay);
		} else {
			char errmsg[] = "Graphics_ImpInit: "
					"failed to open local display\n";
			fprintf(stderr, "%s", errmsg);
		}
		Util_Clear();
		exit(EXIT_FAILURE);
	}
	char const *envDisplay = getenv("DISPLAY");
	char *msg = "Graphics_ImpInit: openned display [DISPLAY%s]\n";
	fprintf(stdout, msg, envDisplay);
}

void Graphics_ImpEndFrame (void)
{
	if (!framebuffer) {
		fprintf(stdout, "Graphics_ImpEndFrame: unallocated framebuffer\n");
		return;
	}

	// memset(Video.buffer, 0, Video.bufferSize);
	// Draw_CheckeredPattern();
	// Draw_Fill(0, 0, Video.width, 0.75 * Video.height, Graphics_RGB(0, 0, 255));
	// Draw_ShowQuakePalette();

	XPutImage(display,
		  window,
		  gc,
		  framebuffer,
		  0,
		  0,
		  0,
		  0,
		  Video.width,
		  Video.height);
}

void ResetFrameBuffer (void)
{
	if (framebuffer) {
		XDestroyImage(framebuffer);
		framebuffer = NULL;
	}

	int pwidth = (((visualInfo->depth / 8) == 3)? 4 : (visualInfo->depth / 8));
	int bytes = ((Video.width * pwidth + 7) & ~7) * Video.height;
	char *data = malloc(bytes);
	if (!data) {
		XFreeGC(display, gc);
		XFree(visualInfo);
		XFreeColormap(display, windowAttributes.colormap);
		XCloseDisplay(display);
		Util_Clear();
		fprintf(stderr, "ResetFrameBuffer: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(data, 0, bytes);
	fprintf(stdout, "ResetFrameBuffer: pwidth: %d\n", pwidth);
	fprintf(stdout, "ResetFrameBuffer: framebuffer-size: 0x%x\n", bytes);

	framebuffer = XCreateImage(display,
				   visual,
				   visualInfo->depth,
				   ZPixmap,
				   0,
				   data,
				   Video.width,
				   Video.height,
				   32,
				   0);

	fprintf(stdout,
		"ResetFrameBuffer: bytes-per-line %d\n",
		framebuffer->bytes_per_line);

	Video.rowBytes = framebuffer->bytes_per_line;
        Video.buffer = (Byte*) framebuffer->data;
	Video.bufferSize = bytes;
}

void Graphics_ImpShutdown (void)
{
	if (!X11Active) {
		return;
	}

	if (framebuffer) {
		XDestroyImage(framebuffer);
		framebuffer = NULL;
	}

	if (visualInfo) {
		XFree(visualInfo);
		XFreeGC(display, gc);
		XFreeColormap(display, windowAttributes.colormap);
		XDestroyWindow(display, window);
		memset(&gc, 0, sizeof(GC));
		memset(&window, 0, sizeof(Window));
		visualInfo = NULL;
	}

	X11Active = false;
}

bool Graphics_ImpInitGraphics (bool fullscreen)
{
	if (fullscreen) {
		fprintf(stdout, "Graphics_ImpInitGraphics: fullscreen\n");
	} else {
		fprintf(stdout, "Graphics_ImpInitGraphics: normal-screen\n");
	}

	Graphics_ImpShutdown();

	int screen = XDefaultScreen(display);
	visual = XDefaultVisual(display, screen);
	fprintf(stdout, "mask-blue:  0x%016lx\n", visual->blue_mask);
	fprintf(stdout, "mask-green: 0x%016lx\n", visual->green_mask);
	fprintf(stdout, "mask-red:   0x%016lx\n", visual->red_mask);
	VisualID visualID = XVisualIDFromVisual(visual);
	int num_visuals = 0;
	XVisualInfo visualInfoTemplate;
	visualInfoTemplate.visualid = visualID;
	visualInfo = XGetVisualInfo(display,
				    VisualIDMask,
				    &visualInfoTemplate,
				    &num_visuals);

	if (visualInfo->class == TrueColor) {
		fprintf(stdout, "Graphics_ImpInitGraphics: visual.class == TrueColor\n");
	}
	fprintf(stdout,
		"Graphics_ImpInitGraphics: visual.depth == %d\n",
		visualInfo->depth);

	int windowAttributeFlags = CWEventMask | CWColormap | CWBorderPixel;
	Colormap cmap = XCreateColormap(display,
					XRootWindow(display, screen),
					visual,
					AllocNone);

	windowAttributes.event_mask =	StructureNotifyMask |
					KeyPressMask |
					KeyReleaseMask |
					ExposureMask |
					PointerMotionMask |
					ButtonPressMask |
					ButtonReleaseMask;
	windowAttributes.border_pixel = 0;
	windowAttributes.colormap = cmap;
	window = XCreateWindow(display,
			       XRootWindow(display, visualInfo->screen),
			       0,
			       0,
			       Video.width,
			       Video.height,
			       0,
			       visualInfo->depth,
			       InputOutput,
			       visual,
			       windowAttributeFlags,
			       &windowAttributes);
	XStoreName(display, window, "Quake");

	//FIXME: add CreateNullCursor it is quite the thing that you want to do!
	//XDefineCursor(display, window, CreateNullCursor(display, window));

	XGCValues gcValues;
	int gcFlags = GCGraphicsExposures;
	gcValues.graphics_exposures = false;
	gc = XCreateGC(display, window, gcFlags, &gcValues);
	XMapWindow(display, window);

	while (true) {
		XEvent ev;
		XNextEvent(display, &ev);
		if (ev.type == Expose && !ev.xexpose.count) {
			char const *msg = "Graphics_ImpInitGraphics: "
					  "dequeued first exposure event\n";
			fprintf(stdout, "%s", msg);
			break;
		}
	}

	ResetFrameBuffer();

        Video.rowBytes = framebuffer->bytes_per_line;
        Video.buffer = (Byte*) framebuffer->data;

        X11Active = true;
	return true;
}

void Graphics_ImpSetPalette (Byte const *palette)
{
	if (!X11Active) {
		return;
	}

	for (int i = 0; i != 256; ++i) {
		uint64_t r = palette[4 * i + 0];
		uint64_t g = palette[4 * i + 1];
		uint64_t b = palette[4 * i + 2];
		curpalette[i] = Graphics_RGB(r, g, b);
	}
}

enum GraphicsErrorType Graphics_ImpSetMode (int *width,
					    int *height,
					    int mode,
					    bool fullscreen)
{
	if (!VID_GetModeInfo(width, height, mode)) {
		return GERR_INVALID_MODE;
	}

	fprintf(stdout,
		"Graphics_ImpSetMode: Video.width: %d Video.height: %d\n",
		*width,
		*height);

	if (!Graphics_ImpInitGraphics(false)) {
		return GERR_INVALID_MODE;
	}

	Refresh_GammaCorrectAndSetPalette((Byte const*) curpalette);

	return GERR_OK;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/graphics.c

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
