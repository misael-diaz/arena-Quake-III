#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <errno.h>

#include "types/Byte.h"
#include "video.h"
#include "graphics.h"
#include "util.h"

#define XCENTERING 0.5f
#define YCENTERING XCENTERING
#define MAX_QPATH 64
#define MAX_MAP_LEAFS 0x10000
#define MODEL_NOVIS_SIZE (MAX_MAP_LEAFS / 8)

enum GraphicsErrorType {
        GERR_OK,
        GERR_INVALID_FULLSCREEN,
        GERR_INVALID_MODE,
        GERR_UNKNOWN
};

enum ImageType {
	IMG_SKIN,
	IMG_SPRIMGE,
	IMG_WALL,
	IMG_PIC,
	IMG_SKY
};

struct Vector3D {
	float x;
	float y;
	float z;
	int32_t: 32;
};

struct ClipPlane {
	struct ClipPlane *next;
	struct Vector3D normal;
	float dist;
	Byte leftedge;
	Byte rightedge;
	Byte reserved[2];
};

struct GraphState {
	int prev_mode;
	Byte gammatable[256];
	Byte currentpalette[1024];
	bool fullscreen;
	char padding[251];
};

struct Image {
	char name[MAX_QPATH];
	enum ImageType type;
	int width;
	int height;
	int registration_sequence;
	Byte *pixels[4];
	bool transparent;
	int64_t: 64;
};

struct OldRefDef {
	float xOrigin;
	float yOrigin;
}; // NOTE: MINIMAL OLDREFDEF, MISSING FIELDS

struct Cfg {
        struct Cfg *next;
        char *latched_string;
        char *string;
        char *name;
        float value;
        int flags;
        bool modified;
	char padding[23];
};

extern struct Video Video;

static struct OldRefDef RefDef;
static struct GraphState GraphState;
static struct Cfg vid_gamma;
static struct Cfg vid_fullscreen;
static struct Cfg graphics_mode;
static GC gc;
static Window window;
static XSetWindowAttributes windowAttributes;
static Display *display = NULL;
static Visual *visual = NULL;
static XVisualInfo *visualInfo = NULL;
static XImage *framebuffer = NULL;
static struct Image *r_notexture_mip = NULL;
static struct ClipPlane view_clipplanes[4];
static uint32_t table[256];
static uint32_t curpalette[256];
static Byte r_notexture_buffer[1024];
static Byte mod_novis[MODEL_NOVIS_SIZE];
static uint64_t bitshift_red = 0;
static uint64_t bitshift_green = 0;
static uint64_t bitshift_blue = 0;
static bool bitshifts_initialized = false;
static int model_registration_sequence = 0;
static float r_aliasuvscale = 1.0f;
static bool X11Active = false;

static size_t FS_FileSize (FILE *file)
{
	fseek(file, 0L, SEEK_SET);
	ssize_t b = ftell(file);
	if (b == -1) {
		fclose(file);
		fprintf(stderr, "FS_FileSize: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}

	fseek(file, 0L, SEEK_END);
	ssize_t e = ftell(file);
	if (e == -1) {
		fclose(file);
		fprintf(stderr, "FS_FileSize: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}

	fseek(file, 0L, SEEK_SET);
	size_t size = (e - b);
	return size;
}

static void *AllocBufferFile (FILE *file, size_t const size)
{
	size_t const bytes = (size + 1);
	void *buffer = Util_Malloc(bytes);
	if (!buffer) {
		fclose(file);
		fprintf(stderr, "AllocBufferFile: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}
	memset(buffer, 0, bytes);
	return buffer;
}

static void LoadBufferFile (FILE *file, void *buffer, size_t const size)
{
	size_t const bytes_read = fread(buffer, 1, size, file);
	if (bytes_read != size) {
		fclose(file);
		fprintf(stderr, "LoadBufferFile: failed to read file\n");
		Util_Clear();
		exit(EXIT_FAILURE);
	}
}

static void LoadPalette (void)
{
	char const *lmp = "assets/palette.lmp";
	FILE *flmp = fopen(lmp, "r");
	if (!flmp) {
		fprintf(stderr, "LoadPalette: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	size_t const len = FS_FileSize(flmp);
	if (len != 256 * 3) {
		fclose(flmp);
		fprintf(stderr, "LoadPalette: wrong file size\n");
		exit(EXIT_FAILURE);
	}
	void *vbuffer = AllocBufferFile(flmp, len);
	LoadBufferFile(flmp, vbuffer, len);

	Byte const *data = (Byte const*) vbuffer;
	Byte *palette = (Byte*) table;
	memset(palette, 0, 1024);
	for (size_t n = 0, i = 0; i != len; ++n, i += 3) {
		palette[4 * n + 0] = data[i + 0];
		palette[4 * n + 1] = data[i + 1];
		palette[4 * n + 2] = data[i + 2];
	}

	for (size_t i = 0; i != len; i +=3) {
		fprintf(stdout, "%03d %03d %03d\n", data[i], data[i + 1], data[i + 2]);
	}

	vbuffer = Util_Free(vbuffer);
	fclose(flmp);
}

static void Draw_InitLocal (void)
{
	return;
}

static void Draw_GetPalette (void)
{
	LoadPalette();
}

void Draw_BuildGammaTable (void)
{
	memset(GraphState.gammatable, 0, sizeof(GraphState.gammatable));
	for (int i = 0; i != 256; ++i) {
		GraphState.gammatable[i] = i;
	}
}

void Draw_Fill (int x, int y, int width, int height, int color)
{
	if (!Video.buffer) {
		fprintf(stderr, "Draw_Fill: NullVidBufferError\n");
		return;
	}

	if (!Video.rowBytes) {
		fprintf(stderr, "Draw_Fill: VidSetError\n");
		return;
	}

	if ((width < 0) || (height < 0)) {
		fprintf(stderr, "Draw_Fill: InvalidDimsError\n");
		return;
	}

	if (x < 0) {
		fprintf(stderr, "Draw_Fill: Fixing X Position\n");
		width += (-x);
		x = 0;
	}

	if (y < 0) {
		fprintf(stderr, "Draw_Fill: Fixing Y Position\n");
		height += (-y);
		y = 0;
	}

	if ((x + width) > Video.width) {
		if (x == 0) {
			fprintf(stderr, "Draw_Fill: BigDimsError\n");
			return;
		}
		fprintf(stderr, "Draw_Fill: Fixing Width\n");
		width = (Video.width - x);
	}

	if ((y + height) > Video.height) {
		if (y == 0) {
			fprintf(stderr, "Draw_Fill: BigDimsError\n");
			return;
		}
		fprintf(stderr, "Draw_Fill: Fixing Height\n");
		height = (Video.height - y);
	}

	// FIXME: could overflow if x, y exceed their respective thresholds
	int const c = color;
	int offset = (Video.rowBytes / 4);
	int *buffer = (int*) Video.buffer;
	int *p = buffer + offset * y + x;
	int jy = 0, ix = 0;
	for (jy = 0; jy != height; ++jy) {
		for (ix = 0; ix != width; ++ix) {
			p[ix] = c;
		}
		p += offset;
	}
}

static void shiftmasks_init (void)
{
	if (!visual) {
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

static uint32_t RGB32 (uint64_t r, uint64_t g, uint64_t b)
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
	model_registration_sequence = 1;
}

void Mod_Init (void)
{
	memset(mod_novis, 255, MODEL_NOVIS_SIZE);
}

static void Graphics_ImpInit (void)
{
	display = XOpenDisplay(0);
	if (!display) {
		char const *envDisplay = getenv("DISPLAY");
		if (envDisplay) {
			char errmsg[] = "Graphics_ImpInit: failed to open display [%s]\n";
			fprintf(stderr, errmsg, envDisplay);
		} else {
			char errmsg[] = "Graphics_ImpInit: failed to open local display\n";
			fprintf(stderr, "%s", errmsg);
		}
		exit(EXIT_FAILURE);
	}
	char const *envDisplay = getenv("DISPLAY");
	char *msg = "Graphics_ImpInit: openned display [DISPLAY%s]\n";
	fprintf(stdout, msg, envDisplay);
}

static void Graphics_ImpEndFrame (void)
{
	if (!framebuffer) {
		fprintf(stdout, "Graphics_ImpEndFrame: unallocated framebuffer\n");
		return;
	}

	memset(Video.buffer, 0, Video.bufferSize);
	Draw_Fill(000, 000, 640, 512, RGB32(128, 128, 128));
	Draw_Fill(640, 512, 640, 512, RGB32(128, 128, 128));
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

void Graphics_EndFrame (void)
{
	Graphics_ImpEndFrame();
}

static void ResetFrameBuffer (void)
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
		fprintf(stderr, "ResetFrameBuffer: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(data, 0, bytes);
	memcpy(data, table, sizeof(table));
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

static void Graphics_ImpShutdown (void)
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

static bool Graphics_ImpInitGraphics (bool fullscreen)
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

static void Graphics_ImpSetPalette (Byte const *palette)
{
	if (!X11Active) {
		return;
	}

	for (int i = 0; i != 256; ++i) {
		uint64_t r = palette[4 * i + 0];
		uint64_t g = palette[4 * i + 1];
		uint64_t b = palette[4 * i + 2];
		curpalette[i] = RGB32(r, g, b);
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

static enum GraphicsErrorType Graphics_ImpSetMode (int *width,
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

	Graphics_GammaCorrectAndSetPalette((Byte const*) curpalette);

	return GERR_OK;
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
	if (vid_gamma.modified) {
		Draw_BuildGammaTable();
		Graphics_GammaCorrectAndSetPalette((Byte const*) curpalette);
		vid_gamma.modified = false;
	}

	if (graphics_mode.modified) {
		enum GraphicsErrorType err = Graphics_ImpSetMode(&Video.width,
								 &Video.height,
								 graphics_mode.value,
								 vid_fullscreen.value);
		if (err) {
			fprintf(stderr, "Graphics_BeginFrame: SetModeError\n");
			Graphics_Shutdown();
			XCloseDisplay(display);
			exit(EXIT_FAILURE);
		}
		Graphics_InitGraphics(Video.width, Video.height);
		GraphState.prev_mode = graphics_mode.value;
		vid_fullscreen.modified = false;
		graphics_mode.modified = false;
	}
}

void Graphics_Register (void)
{
	vid_fullscreen.value = 0;
	graphics_mode.value = 8;
	graphics_mode.modified = true;
	vid_gamma.modified = true;
}

void Graphics_Init (void)
{
	Graphics_InitImages();
	Mod_Init();
	Draw_InitLocal();
	Graphics_InitTextures();

	view_clipplanes[0].leftedge = true;	view_clipplanes[0].rightedge = false;
	view_clipplanes[1].leftedge = false;	view_clipplanes[1].rightedge = true;
	view_clipplanes[2].leftedge = false;	view_clipplanes[2].rightedge = false;
	view_clipplanes[3].leftedge = false;	view_clipplanes[3].rightedge = false;

	RefDef.xOrigin = XCENTERING;
	RefDef.yOrigin = YCENTERING;

	r_aliasuvscale = 1.0f;

	Graphics_Register();
	Draw_GetPalette();
	Graphics_ImpInit();

	Graphics_BeginFrame();
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
