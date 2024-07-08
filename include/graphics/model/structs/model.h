#ifndef GUARD_QUAKE_GRAPHICS_MODEL_STRUCTS_MODEL_H
#define GUARD_QUAKE_GRAPHICS_MODEL_STRUCTS_MODEL_H

#include <stdbool.h>

#include "../../../common/types/Byte.h"
#include "../../../common/structs/Vector.h"
#include "../../../game/defs.h"
#include "../../local/structs/local.h"
#include "../../model/enums/model.h"

struct ModelPlane {
        struct Vector normal;
        float dist;
        Byte type;
        Byte signbits;
        Byte pad[10];
}; // mplane_t

struct ModelTexInfo {
        struct ModelTexInfo *next;
        struct Image *image;
        float vecs[2][4];
        float mipAdjust;
        int flags;
        int numframes;
}; // mtexinfo_t;

struct ModelSurface {
	struct CacheSurface *cacheSpots[MIPLEVELS];
	struct ModelSurface *nextAlphaSurface;
	struct ModelPlane *plane;
	struct ModelTexInfo *texinfo;
	Byte *samples;
	Byte styles[MAXLIGHTMAPS];
	int visframe;
	int dlightframe;
	int dlightbits;
	int flags;
	int firstedge;
	int numedges;
	int texturemins[2];
	int extents[2];
}; // msurface_t

// TODO: make sizeof(ModelNode) = sizeof(ModeLeaf) so that we can cast from one to another
// with confidence
struct ModelNode {
	int contents;
	int visframe;
	int minmaxs[6];
	struct ModelNode *parent;
	struct ModelNode *children[2];
	struct ModelPlane *plane;
	int firstsurface;
	int numsurfaces;
	long: 64;
}; // mnode_t

struct ModelLeaf {
	int contents;
	int visframe;
	int minmaxs[6];
	struct ModelNode *parent;
	struct ModelSurface **firstmarksurface;
	int cluster;
	int area;
	int numMarkedSurfaces;
	int key;
	long: 64;
	long: 64;
}; // mleaf_t

struct ModelEdge {
        unsigned int v[2];
        unsigned int cachedEdgeOffset;
}; // medge_t

struct Vertex {
	struct Vector position;
}; // mvertex_t

struct Model {
	struct Image *skins[MAX_MD2SKINS];
	struct DataModel *submodels;
	struct ModelSurface *surfaces;
	struct ModelSurface **marksurfaces;
	struct ModelPlane *planes;
	struct ModelNode *nodes;
	struct ModelLeaf *leafs;
	struct ModelEdge *edges;
	struct Vertex *vertexes;
	struct ModelTexInfo *texinfo;
	struct DataVisibility *vis;
	int *surfedges;
	Byte *lightdata;
	void *extradata;
	struct Vector mins;
	struct Vector maxs;
	struct Vector clipmins;
	struct Vector clipmaxs;
	char name[MAX_QPATH];
	enum ModelType type;
	int registration_sequence;
	int numframes;
	int flags;
	int firstmodelsurface;
	int nummodelsurfaces;
	int numsubmodels;
	int numplanes;
	int numleafs;
	int numvertexes;
	int numedges;
	int numnodes;
	int firstnode;
	int numtexinfo;
	int numsurfaces;
	int numsurfedges;
	int nummarksurfaces;
	int extradatasize;
	bool clipbox;
}; // model_t

struct Entity {
	struct Image *skin;
	struct Model *model;
	float angles[3];
	float origin[3];
	float oldorigin[3];
	float backlerp;
	float alpha;
	int oldframe;
	int frame;
	int skinnum;
	int lightstyle;
	int flags;
}; // entity_t

struct Span {
        struct Span *next;
	int count;
        int u;
	int v;
}; // espan_t

struct DataLight {
	struct Vector origin;
	struct Vector color;
	float intensity;
}; // dlight_t

struct Particle {
        struct Vector origin;
        float alpha;
        int color;
}; // particle_t

struct LightStyle {
	float rgb[3];
	float white;
}; // lightstyle_t

struct Surface {
	struct Surface *next;
	struct Surface *prev;
	struct ModelSurface *modelSurface;
	struct Entity *entity;
	struct Span *spans;
	float nearZi;
	float d_ziOrigin;
	float d_ziStepU;
	float d_ziStepV;
	int key;
	int last_u;
	int spanstate;
	int flags;
	bool insubmodel;
	int pad[2];
	char xpad[3];
}; // surf_t

struct RefreshDefinition {
	struct Entity *entities;
	struct LigthStyle *lightstyles;
	struct Particle *particles;
	struct DataLight *dlights;
	Byte *areabits;
	struct Vector vieworg;
	struct Vector viewangles;
	float fov_x;
	float fov_y;
	float blend[4];
	float time;
	int x;
	int y;
	int width;
	int height;
	int RDFlags;
	int num_entities;
	int num_dlights;
	int num_particles;
}; //refdef_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/graphics/model/structs/model.h

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
