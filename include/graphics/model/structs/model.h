#ifndef GUARD_QUAKE_GRAPHICS_MODEL_STRUCTS_MODEL_H
#define GUARD_QUAKE_GRAPHICS_MODEL_STRUCTS_MODEL_H

#include <stdbool.h>

#include "../../../common/types/Byte.h"
#include "../../../common/structs/Vector.h"
#include "../../../game/defs.h"
#include "../../local/structs/local.h"
#include "../../model/enums/model.h"

struct ModelPlane {
        struct Vector3D normal;
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

struct DataModel {
	float mins[3];
	float maxs[3];
	float origin[3];
	int headnode;
	int firstface;
	int numfaces;
}; // dmodel_t

struct ModelNode {
	struct ModelNode *parent;
	struct ModelNode *children[2];
	struct ModelPlane *plane;
	int minmaxs[6];
	int contents;
	int visframe;
	int firstsurface;
	int numsurfaces;
}; // mnode_t

struct ModelLeaf {
	struct ModelNode *parent;
	struct ModelSurface **firstmarksurface;
	int minmaxs[6];
	int contents;
	int visframe;
	int cluster;
	int area;
	int numMarkedSurfaces;
	int key;
}; // mleaf_t

struct ModelEdge {
        unsigned int v[2];
        unsigned int cachedEdgeOffset;
}; // medge_t

struct Vertex {
	struct Vector3D position;
}; // mvertex_t

struct DataVisibility {
	int numclusters;
	int bitofs[8][2];
}; // dvis_t

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
	struct Vector3D mins;
	struct Vector3D maxs;
	struct Vector3D clipmins;
	struct Vector3D clipmaxs;
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
	struct Vector3D origin;
	struct Vector3D color;
	float intensity;
}; // dlight_t

struct Particle {
        struct Vector3D origin;
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

struct ReferenceDefinition {
	struct Entity *entities;
	struct LigthStyle *lightstyles;
	struct Particle *particles;
	struct DataLight *dlights;
	Byte *areabits;
	struct Vector3D vieworg;
	struct Vector3D viewangles;
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
