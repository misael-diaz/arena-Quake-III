#ifndef GUARD_QUAKE_GRAPHICS_LOCAL_STRUCTS_LOCAL_H
#define GUARD_QUAKE_GRAPHICS_LOCAL_STRUCTS_LOCAL_H

#include <stdbool.h>
#include "../../../common/types/Byte.h"
#include "../../../common/structs/Vector.h"
#include "../../../game/defs.h"
#include "../../local/enums/local.h"

struct ClipPlane {
	struct ClipPlane *next;
	struct Vector normal;
	float dist;	// if you change to double, update R_SetUpFrustumIndexes()
	Byte leftedge;
	Byte rightedge;
	Byte reserved[2];
}; // clipplane_t

struct GraphState {
        int prev_mode;
        Byte gammatable[256];
        Byte currentpalette[1024];
        bool fullscreen;
        char padding[251];
}; // swstate_t

struct Image {
        Byte *pixels[4];
        char name[MAX_QPATH];
        enum ImageType type;
        int width;
        int height;
        int registration_sequence;
        bool transparent;
}; // image_t

struct CacheSurface {
        struct CacheSurface **owner;
        struct CacheSurface *next;
        struct Image *image;
        int lightadj[MAXLIGHTMAPS];
        int dlight;
        int size;
        int width;
        int height;
        float mipscale;
        Byte data[4];
}; // surfcache_t;

struct ViewRectangle {
	struct ViewRectangle *next;
	int width;
	int height;
	int x;
	int y;
}; // vrect_t;

struct OldRefreshDefinition {
	struct ViewRectangle viewRect;
	struct Vector vieworg;
	struct Vector viewangles;
	float fViewRectRightEdge;
	float fViewRectX;
	float fViewRectY;
	float fViewRectX_adj;
	float fViewRectY_adj;
	float fViewRectRight;
	float fViewRectBottom;
	float fViewRectRight_adj;
	float fViewRectBottom_adj;
	float fHorizontalFieldOfView;
	float fVerticalFieldOfView;
	float xOrigin;
	float yOrigin;
	int ambientlight;
	int viewRectRight;
	int viewRectBottom;
	int viewRectRight_adj_shift20;
	int viewRectX_adj_shift20;
}; // oldrefdef_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/graphics/local/structs/local.h

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
