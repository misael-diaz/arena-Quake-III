#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "common.h"
#include "graphics.h"

#define MAX_REFRESH_IMAGES 1024

extern struct Image *r_notexture_mip;
extern int mod_registration_sequence;

static struct Image r_images[MAX_REFRESH_IMAGES];
static int numRefreshImages = 0;

static struct Image *Image_FindFreeImage (void)
{
	int idx = 0;
	struct Image *image = &r_images[0];
	for (int i = 0; i != numRefreshImages; ++image, ++idx, ++i) {
		if (!image->registration_sequence) {
			break;
		}
	}

	if (idx == numRefreshImages) {
		if (idx == MAX_REFRESH_IMAGES) {
			Q_Shutdown();
			fprintf(stderr, "Image_FindFreeImage: MaxRefreshImagesError\n");
			exit(EXIT_FAILURE);
		}
		++numRefreshImages;
	}

	if (&r_images[idx] != image) {
		Q_Shutdown();
		fprintf(stderr, "Image_FindFreeImage: ImplError\n");
		exit(EXIT_FAILURE);
	}

	return image;
}

static struct Image *Image_LoadWal (char const *image_name)
{
	if (strlen(image_name) >= MAX_QPATH) {
		Q_Shutdown();
		fprintf(stderr, "Image_LoadWal: ImageNameLengthError\n");
		exit(EXIT_FAILURE);
	}

	struct MIPTexture *miptexture = NULL;
	size_t const len = FS_FLoadFile(image_name, (void**) &miptexture);
	if (!miptexture) {
		return r_notexture_mip;
	}

	int const width = miptexture->width;
	int const height = miptexture->height;

	struct Image *image = Image_FindFreeImage();
	strncpy(image->name, image_name, MAX_QPATH);
	image->width = width;
	image->height = height;
	image->type = IT_WALL;
	image->registration_sequence = mod_registration_sequence;

	size_t const image_size = (width * height);
	if ((image_size % 256) || (image_size % 16) || (image_size % 4)) {
		Q_Shutdown();
		fprintf(stderr, "Image_LoadWal: ImageSizeError\n");
		exit(EXIT_FAILURE);
	}

	size_t const image_allocSize = (((width * height) + 0xff) & ~0xff);
	size_t const image_allocMIPSize3 = (image_allocSize / 64);
	size_t const image_allocMIPSize2 = (image_allocSize / 16);
	size_t const image_allocMIPSize1 = (image_allocSize / 4);
	size_t const image_allocMIPSize0 = (image_allocSize / 1);
	size_t const bytes = image_allocMIPSize0 +
			     image_allocMIPSize1 +
			     image_allocMIPSize2 +
			     image_allocMIPSize3;

	Byte *data = Util_Malloc(bytes);
	if (!data) {
		Q_Shutdown();
		fprintf(stderr, "Image_LoadWal: MallocError\n");
		exit(EXIT_FAILURE);
	}

	memset(data, 0, bytes);
	size_t const miptexture0_size = (width * height) / 1;
	size_t const miptexture1_size = (width * height) / 4;
	size_t const miptexture2_size = (width * height) / 16;
	size_t const miptexture3_size = (width * height) / 64;
	size_t const miptexture_size = miptexture0_size +
				       miptexture1_size +
				       miptexture2_size +
				       miptexture3_size;
	image->pixels[0] = data;
	image->pixels[1] = image->pixels[0] + miptexture0_size;
	image->pixels[2] = image->pixels[1] + miptexture1_size;
	image->pixels[3] = image->pixels[2] + miptexture2_size;

	size_t const offset = miptexture->offsets[0];

	if (miptexture_size > ((size_t)((miptexture + len) - (miptexture + offset)))) {
		Q_Shutdown();
		fprintf(stderr, "Image_LoadWal: MIPTextureSizeError\n");
		exit(EXIT_FAILURE);
	}

	memcpy(data, miptexture + offset, miptexture_size);
	miptexture = Util_Free(miptexture);
	return image;
}

struct Image *Image_FindImage (char const *image_name)
{
	if (!image_name) {
		Q_Shutdown();
		fprintf(stderr, "Image_FindImage: NullImageNameError\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(image_name) < 5) {
		Q_Shutdown();
		fprintf(stderr, "Image_FindImage: InvalidImageNameError\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i != numRefreshImages; ++i) {
		struct Image *img = &r_images[i];
		if (!strcmp(img->name, image_name)) {
			img->registration_sequence = mod_registration_sequence;
			return img;
		}
	}

	// TODO: add code to load PCX images as in the original
	char const *ext = (image_name + strlen(image_name) - 4);
	if (!strcmp(ext, ".wal")) {
		struct Image *image = Image_LoadWal(image_name);
		return image;
	}

	Q_Shutdown(); // NOTE: "Maybe" because it could be a valid PCX image
	fprintf(stderr, "Image_FindImage: MaybeBadImageExtensionError\n");
	exit(EXIT_FAILURE);
	return NULL;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/surface.c

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
