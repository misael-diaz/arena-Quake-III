#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "video.h"
#include "graphics.h"
#include "util.h"

extern struct Video Video;
extern struct GraphState GraphState;

static unsigned int table[256];
static unsigned int palette[256];

static size_t FS_FileSize (FILE *file)
{
	fseek(file, 0L, SEEK_SET);
	ssize_t b = ftell(file);
	if (b == -1) {
		fclose(file);
		Util_Clear();
		fprintf(stderr, "FS_FileSize: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	fseek(file, 0L, SEEK_END);
	ssize_t e = ftell(file);
	if (e == -1) {
		fclose(file);
		Util_Clear();
		fprintf(stderr, "FS_FileSize: %s\n", strerror(errno));
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
		Util_Clear();
		fprintf(stderr, "AllocBufferFile: %s\n", strerror(errno));
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
		Util_Clear();
		fprintf(stderr, "LoadBufferFile: failed to read file\n");
		exit(EXIT_FAILURE);
	}
}

static void LoadPalette (void)
{
	char const *lmp = "assets/palette.lmp";
	FILE *flmp = fopen(lmp, "r");
	if (!flmp) {
		Util_Clear();
		fprintf(stderr, "LoadPalette: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	size_t const len = FS_FileSize(flmp);
	if (len != 256 * 3) {
		fclose(flmp);
		Util_Clear();
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
		palette[4 * n + 3] = 0xff;
	}

	for (size_t i = 0; i != len; i +=3) {
		fprintf(stdout, "%03d %03d %03d\n", data[i], data[i + 1], data[i + 2]);
	}

	vbuffer = Util_Free(vbuffer);
	fclose(flmp);
}

void Draw_InitLocal (void)
{
	return;
}

void Draw_GetPalette (void)
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

void Draw_CheckeredPattern (void)
{
	int const width = 64;
	int const height = 64;
	if (width > Video.width || height > Video.height) {
		fprintf(stderr, "Draw_CheckeredPattern: would exceed video dims\n");
		return;
	}
	unsigned int size = (Video.bufferSize / 4);
	unsigned int const rowSize = (Video.rowBytes / 4);
	unsigned int *buffer = (unsigned int*) Video.buffer;
	unsigned int *p = buffer;
	for (unsigned int k = 0; k != size; ++k) {
		unsigned int i = ((k % rowSize) / width);
		unsigned int j = ((k / rowSize) / height);
		if ((i % 2) ^ (j % 2)) {
			p[k] = Graphics_RGB(128, 128, 128);
		} else {
			p[k] = Graphics_RGB(0, 0, 0);
		}
	}
}

void Draw_ShowQuakePalette (void)
{
	unsigned int const height = (Video.height / 2);
	unsigned int const width = 256;
	unsigned int const rowSize = (Video.rowBytes / 4);
	unsigned int const offset = rowSize;
	unsigned int *buffer = (unsigned int*) Video.buffer;
	unsigned int *p = buffer;
	for (unsigned int j = 0; j != height; ++j) {
		for (unsigned int i = 0; i != width; ++i) {
			p[4 * i + 0] = table[i];
			p[4 * i + 1] = table[i];
			p[4 * i + 2] = table[i];
			p[4 * i + 3] = table[i];
		}
		p += offset;
	}
}

void Draw_PatchQuakePalette (void)
{
	for (int j = 0; j != 8; ++j) {
		for (int i = 0; i != 16; ++i) {
			int const idx = 16 * j + i;
			palette[idx] = table[idx];
		}
	}

	// we need to map ligth-to-dark to dark-to-light here, see Quake wiki
	for (int j = 8; j != 14; ++j) {
		for (int i = 0; i != 16; ++i) {
			int const idx = 16 * j + i;
			int const jdx = 16 * (j + 1) - (i + 1);
			palette[idx] = table[jdx];
		}
	}

	for (int j = 14; j != 16; ++j) {
		for (int i = 0; i != 16; ++i) {
			int const idx = 16 * j + i;
			palette[idx] = table[idx];
		}
	}

	Byte const *src = (Byte const*) palette;
	for (int i = 0; i != 1024; i += 4) {
		table[(i / 4)] = Graphics_RGB(src[i + 0], src[i + 1], src[i + 2]);
	}
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/draw.c

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
