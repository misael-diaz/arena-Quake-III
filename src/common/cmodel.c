#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "common.h"
#include "game.h"

#define NUM_LEAFS_BOX_HULL 1
#define NUM_LEAF_BRUSHES_BOX_HULL 1
#define NUM_BRUSHES_BOX_HULL 1
#define NUM_SIDES_BOX_HULL 6
#define NUM_PLANES_BOX_HULL (2 * NUM_SIDES_BOX_HULL)
#define NUM_NODES_BOX_HULL NUM_SIDES_BOX_HULL

static Byte *cmod_base = NULL;
static struct CVar *map_noareas = NULL;
static struct CBrush *box_brush = NULL;
struct CLeaf *box_leaf = NULL;

static Byte map_visibility[MAX_MAP_VISIBILITY];
static struct CModel map_cmodels[MAX_MAP_MODELS];
static struct CArea map_areas[MAX_MAP_AREAS];
static struct DataAreaPortal map_areaportals[MAX_MAP_AREAPORTALS];
static struct CLeaf map_leafs[MAX_MAP_LEAFS + NUM_LEAFS_BOX_HULL];
static struct CPlane map_planes[MAX_MAP_PLANES + NUM_PLANES_BOX_HULL];
static struct CNode map_nodes[MAX_MAP_NODES + NUM_NODES_BOX_HULL];
static struct CBrush map_brushes[MAX_MAP_BRUSHES + NUM_BRUSHES_BOX_HULL];
static struct CBrushSide map_brushsides[MAX_MAP_BRUSHSIDES + NUM_SIDES_BOX_HULL];
static struct MapSurface map_surfaces[MAX_MAP_TEXINFO];
static struct MapSurface nullsurface;
static unsigned short map_leafbrushes[MAX_MAP_LEAF_BRUSHES + NUM_LEAF_BRUSHES_BOX_HULL];
static bool portalopen[MAX_MAP_AREAPORTALS];

static char map_name[MAX_QPATH];
static char map_entitystring[MAX_MAP_ENTITY_STRING];

static int numareas = 0;
static int numAreaPortals = 0;
static int numplanes = 0;
static int numbrushes = 0;
static int numbrushsides = 0;
static int numnodes = 0;
static int numleafs = 0;
static int numleafbrushes = 0;
static int numcmodels = 0;
static int numclusters = 0;
static int numvisibility = 0;
static int numentitychars = 0;
static int numtexinfo = 0;

static int floodvalid = 0;
static int checkcount = 0;
static int emptyleaf = -1;

static int box_headnode = 0;

static void CM_LoadSurfaces (struct Lump const *lump)
{
	struct InfoTexture *textures = ((void*) cmod_base + lump->fileofs);
	struct MapSurface *surfaces = map_surfaces;
	if (lump->filelen % sizeof(*textures)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSurfaces: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*textures);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSurfaces: MapNoSurfacesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_TEXINFO) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSurfaces: MapMaxSurfacesError\n");
		exit(EXIT_FAILURE);
	}

	numtexinfo = count;
	fprintf(stdout, "CM_LoadSurfaces: textures: %d\n", numtexinfo);
	for (int i = 0; i != numtexinfo; ++i) {

		struct InfoTexture const *texture = textures;
		struct MapSurface *surface = surfaces;

		strncpy(surface->csurface.name,
			texture->name,
			MAX_CSURFACE_NAME_SIZE);

		strncpy(surface->rname,
			texture->name,
			MAX_MAPSURFACE_RNAME_SIZE);

		surface->csurface.name[MAX_CSURFACE_NAME_SIZE - 1] = 0;
		surface->rname[MAX_MAPSURFACE_RNAME_SIZE - 1] = 0;

		surface->csurface.flags = texture->flags;
		surface->csurface.value = texture->value;

		++textures;
		++surfaces;
	}
}

static void CM_LoadLeafs (struct Lump const *lump)
{
	struct DataLeaf const *_leafs = ((void*) cmod_base + lump->fileofs);
	struct CLeaf *leafs = map_leafs;

	if (lump->filelen % sizeof(*_leafs)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafs: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_leafs);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafs: MapNoLeafsError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_LEAFS) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafs: MapMaxLeafsError\n");
		exit(EXIT_FAILURE);
	}

	numleafs = count;
	numclusters = 0;
	for (int i = 0; i != numleafs; ++i, ++_leafs, ++leafs) {
		struct DataLeaf const *_leaf = _leafs;
		struct CLeaf *leaf = leafs;

		leaf->contents = _leaf->contents;
		leaf->cluster = _leaf->cluster;
		leaf->area = _leaf->area;
		leaf->firstleafbrush = _leaf->firstleafbrush;
		leaf->numleafbrushes = _leaf->numleafbrushes;

		if (leaf->cluster >= numclusters) {
			numclusters = leaf->cluster + 1;
		}
	}

	if (CONTENTS_SOLID != map_leafs[0].contents) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafs: LeafContentsSolidError\n");
		exit(EXIT_FAILURE);
	}

	emptyleaf = -1;
	for (int i = 0; i != numleafs; ++i) {
		if (!map_leafs[i].contents) {
			emptyleaf = i;
			break;
		}
	}

	if (-1 == emptyleaf) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafs: NoEmptyLeafError\n");
		exit(EXIT_FAILURE);
	}
}

static void CM_LoadLeafBrushes (struct Lump const *lump)
{
	unsigned short const *_leafbrushes = ((void*) cmod_base + lump->fileofs);
	unsigned short *leafbrushes = map_leafbrushes;

	if (lump->filelen % sizeof(*_leafbrushes)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafBrushes: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_leafbrushes);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafBrushes: MapNoLeafBrushesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_LEAF_BRUSHES) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadLeafBrushes: MapMaxLeafBrushesError\n");
		exit(EXIT_FAILURE);
	}

	numleafbrushes = count;
	memcpy(leafbrushes, _leafbrushes, numleafbrushes * sizeof(*_leafbrushes));
}

static void CM_LoadPlanes (struct Lump const *lump)
{
	struct DataPlane const *_planes = ((void*) cmod_base + lump->fileofs);
	struct CPlane *planes = map_planes;

	if (lump->filelen % sizeof(*_planes)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadPlanes: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_planes);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadPlanes: MapNoPlanesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_PLANES) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadPlanes: MapMaxPlanesError\n");
		exit(EXIT_FAILURE);
	}

	numplanes = count;
	fprintf(stdout, "CM_LoadPlanes: planes: %d\n", numplanes);
	for (int i = 0; i != numplanes; ++i, ++_planes, ++planes) {
		struct DataPlane const *_plane = _planes;
		struct CPlane *plane = planes;

		float const *_normal = (float const*) &_plane->normal;
		float *normal = (float*) &plane->normal;
		for (int j = 0; j != NUM_AXES; ++j) {
			normal[j] = _normal[j];
		}

		int bits = 0;
		for (int j = 0; j != NUM_AXES; ++j) {
			if (normal[j] < 0.0f) {
				bits |= (1 << j);
			}
		}

		plane->dist = _plane->dist;
		plane->type = _plane->type;
		plane->signbits = bits;
	}
}

static void CM_LoadBrushes (struct Lump const *lump)
{
	struct DataBrush const *_brushes = ((void*) cmod_base + lump->fileofs);
	struct CBrush *brushes = map_brushes;

	if (lump->filelen % sizeof(*_brushes)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushes: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_brushes);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushes: MapNoBrushesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_BRUSHES) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushes: MapMaxBrushesError\n");
		exit(EXIT_FAILURE);
	}

	numbrushes = count;
	fprintf(stdout, "CM_LoadBrushs: brushes: %d\n", numbrushes);
	for (int i = 0; i != numbrushes; ++i, ++_brushes, ++brushes) {
		struct DataBrush const *_brush = _brushes;
		struct CBrush *brush = brushes;

		brush->firstbrushside = _brush->firstside;
		brush->numsides = _brush->numsides;
		brush->contents = _brush->contents;
	}
}

static void CM_LoadBrushSides (struct Lump const *lump)
{
	struct DataBrushSide const *_brushsides = ((void*) cmod_base + lump->fileofs);
	struct CBrushSide *brushsides = map_brushsides;

	if (lump->filelen % sizeof(*_brushsides)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushSides: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_brushsides);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushSides: MapNoBrushSidesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_BRUSHSIDES) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadBrushSides: MapMaxBrushSidesError\n");
		exit(EXIT_FAILURE);
	}

	numbrushsides = count;
	for (int i = 0; i != numbrushsides; ++i, ++_brushsides, ++brushsides) {
		struct DataBrushSide const *_brushside = _brushsides;
		struct CBrushSide *brushside = brushsides;

		unsigned short const planenum = _brushside->planenum;
		short const texinfo = _brushside->texinfo;
		short const surfacenum = texinfo;

		if (planenum >= numplanes) {
			Q_Shutdown();
			fprintf(stderr, "CM_LoadBrushSides: PlaneError\n");
			exit(EXIT_FAILURE);
		}

		if (texinfo >= numtexinfo) {
			Q_Shutdown();
			fprintf(stderr, "CM_LoadBrushSides: TextureInfoError\n");
			exit(EXIT_FAILURE);
		}

		brushside->plane = &map_planes[planenum];
		brushside->surface = &map_surfaces[surfacenum];
	}
}

static void CM_LoadSubModels (struct Lump const *lump)
{
	struct DataModel const *_models = ((void*) cmod_base + lump->fileofs);
	struct CModel *models = map_cmodels;

	if (lump->filelen % sizeof(*_models)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSubModels: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_models);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSubModels: MapNoSubModelsError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_MODELS) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadSubModels: MapMaxSubModelsError\n");
		exit(EXIT_FAILURE);
	}

	numcmodels = count;
	for (int i = 0; i != numcmodels; ++i, ++_models, ++models) {
		struct DataModel const *_model = _models;
		struct CModel *model = models;

		VectorCopy(&_model->mins, &model->mins);
		VectorCopy(&_model->maxs, &model->maxs);
		VectorCopy(&_model->origin, &model->origin);

		model->headnode = _model->headnode;
	}
}

static void CM_LoadNodes (struct Lump const *lump)
{
	struct DataNode const *_nodes = ((void*) cmod_base + lump->fileofs);
	struct CNode *nodes = map_nodes;

	if (lump->filelen % sizeof(*_nodes)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadNodes: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_nodes);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadNodes: MapNoNodesError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_PLANES) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadNodes: MapMaxNodesError\n");
		exit(EXIT_FAILURE);
	}

	numnodes = count;
	fprintf(stdout, "CM_LoadNodes: nodes: %d\n", numnodes);
	for (int i = 0; i != numnodes; ++i, ++_nodes, ++nodes) {
		struct DataNode const *_node = _nodes;
		struct CNode *node = nodes;

		for (int j = 0; j != NUM_NODE_CHILDREN; ++j) {
			node->children[j] = _node->children[j];
		}

		int const planenum = _node->planenum;
		if (planenum >= numplanes) {
			Q_Shutdown();
			fprintf(stderr, "CM_LoadNodes: PlaneError\n");
			exit(EXIT_FAILURE);
		}

		node->plane = &map_planes[planenum];
	}
}

static void CM_LoadAreas (struct Lump const *lump)
{
	struct DataArea const *_areas = ((void*) cmod_base + lump->fileofs);
	struct CArea *areas = map_areas;

	if (lump->filelen % sizeof(*_areas)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreas: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_areas);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreas: MapNoAreasError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_AREAS) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreas: MapMaxAreasError\n");
		exit(EXIT_FAILURE);
	}

	numareas = count;
	fprintf(stdout, "CM_LoadAreas: areas: %d\n", numareas);
	for (int i = 0; i != numareas; ++i, ++_areas, ++areas) {
		struct DataArea const *_area = _areas;
		struct CArea *area = areas;

		area->numAreaPortals = _area->numAreaPortals;
		area->firstAreaPortal = _area->firstAreaPortal;
		area->floodvalid = 0;
		area->floodnum = 0;
	}
}

static void CM_LoadAreaPortals (struct Lump const *lump)
{
	struct DataAreaPortal const *_areaportals = ((void*) cmod_base + lump->fileofs);
	struct DataAreaPortal *areaportals = map_areaportals;

	if (lump->filelen % sizeof(*_areaportals)) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreaPortals: WrongLumpSize\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_areaportals);
	if (count < 1) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreaPortals: MapNoAreaPortalsError\n");
		exit(EXIT_FAILURE);
	}

	if (count > MAX_MAP_AREAPORTALS) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadAreaPortals: MapMaxAreaPortalsError\n");
		exit(EXIT_FAILURE);
	}

	numAreaPortals = count;
	memcpy(areaportals, _areaportals, numAreaPortals * sizeof(*_areaportals));
}

static void CM_LoadVisibility (struct Lump const *lump)
{
	struct DataVisibility const *_visibility = ((void*) cmod_base + lump->fileofs);
	struct DataVisibility *visibility = (struct DataVisibility*) map_visibility;

	if (lump->filelen > MAX_MAP_VISIBILITY) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadVisibility: MapMaxLoadVisibilityError\n");
		exit(EXIT_FAILURE);
	}

	numvisibility = lump->filelen;
	memcpy(visibility, _visibility, numvisibility);
}

void CM_LoadEntityString (struct Lump const *lump)
{
        numentitychars = lump->filelen;
        if (lump->filelen > MAX_MAP_ENTITY_STRING) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadEntityString: MapMaxLoadEntityStringError\n");
		exit(EXIT_FAILURE);
	}

        memcpy(map_entitystring, cmod_base + lump->fileofs, lump->filelen);
}

static void CM_InitBoxHull (void)
{
	box_leaf = &map_leafs[numleafs];
	box_leaf->contents = CONTENTS_MONSTER;
	box_leaf->firstleafbrush = numleafs;
	box_leaf->numleafbrushes = NUM_LEAFS_BOX_HULL;

	box_brush = &map_brushes[numbrushes];
	box_brush->contents = CONTENTS_MONSTER;
	box_brush->numsides = NUM_SIDES_BOX_HULL;
	box_brush->firstbrushside = numbrushes;
	box_brush->checkcount = checkcount;

	map_leafbrushes[numleafbrushes] = numbrushes;

	struct CPlane *box_planes = &map_planes[numplanes];
	struct CBrushSide *box_brushsides = &map_brushsides[numbrushsides];
	struct CPlane *planes = box_planes;
	struct CBrushSide *brushsides = box_brushsides;
	for (int i = 0; i != NUM_SIDES_BOX_HULL; ++i) {
		int const side = (i & 1);
		int const planenum = side + (2 * i);
		struct CPlane *plane = &planes[planenum];
		struct CBrushSide *brushside = &brushsides[i];
		brushside->plane = plane;
		brushside->surface = &nullsurface;
	}

	box_headnode = numnodes;
	struct CNode *box_nodes = &map_nodes[numnodes];
	struct CNode *nodes = box_nodes;
	for (int i = 0; i != NUM_SIDES_BOX_HULL; ++i) {
		int const side = (i & 1);
		int const nodenum = i;
		int const planenum = (2 * i);
		struct CNode *node = &nodes[i];
		struct CPlane *plane = &planes[planenum];
		node->plane = plane;
		node->children[side] = (-1 - emptyleaf);
		if (i == (NUM_SIDES_BOX_HULL - 1)) {
			node->children[side ^ 1] = (-1 - numleafs);
		} else {
			node->children[side ^ 1] = (nodenum + 1);
		}
	}

	for (int i = 0; i != NUM_SIDES_BOX_HULL; ++i) {
		int const axnum = (i >> 1);
		int const planenum = (2 * i);
		struct CPlane *plane = &planes[planenum];
		float *normal = (float*) &plane->normal;
		VectorClear(&plane->normal);
		normal[axnum] = 1.0f;
		plane->dist = 0.0f;
		plane->type = axnum;
		plane->signbits = 0;
	}

	for (int i = 0; i != NUM_SIDES_BOX_HULL; ++i) {
		int const axnum = (i >> 1);
		int const planenum = (2 * i + 1);
		struct CPlane *plane = &planes[planenum];
		float *normal = (float*) &plane->normal;
		VectorClear(&plane->normal);
		normal[axnum] = -1.0f;
		plane->dist = 0.0f;
		plane->type = axnum;
		plane->signbits = (1 << axnum); // maybe set to zero as in the original
	}
}

static void FloodAreaRecursively (struct CArea *area, int const floodnum)
{
	area->floodnum = floodnum;
	area->floodvalid = floodvalid;
	int const firstAreaPortal = area->firstAreaPortal;
	struct DataAreaPortal *portal = &map_areaportals[firstAreaPortal];
	for (int i = 0; i != area->numAreaPortals; ++i) {
		if (portalopen[portal->portalnum]) {
			int const otherAreaNum = portal->otherarea;
			struct CArea *otherarea = &map_areas[otherAreaNum];
			FloodAreaRecursively(otherarea, floodnum);
		}
	}
}

static void FloodAreaConnections (void)
{
	++floodvalid;
	int floodnum = 0;
	for (int i = 1; i != numareas; ++i) {
		struct CArea *area = &map_areas[i];
		if (floodvalid == area->floodvalid) {
			continue;
		}

		++floodnum;
		FloodAreaRecursively(area, floodnum);
	}
}

struct CModel *CM_LoadMap (char const *name)
{
	map_noareas = CVAR_GetCVar("map_noareas", "0", 0);

	// there's missing code here

	numplanes = 0;
	numnodes = 0;
	numleafs = 0;
	numcmodels = 0;
	numvisibility = 0;
	numentitychars = 0;
	map_entitystring[0] = 0;
	map_name[0] = 0;

	void *buf = NULL;
	int length = FS_FLoadFile(name, &buf);
	if (!buf) {
		Q_Shutdown();
		fprintf(stderr, "CM_LoadMap: LoadMapError\n");
		exit(EXIT_FAILURE);
	}

	// missing code here

	struct DataHeader header = *((struct DataHeader const*) buf);
        if (header.ident != ID_BSP_HEADER) {
                fprintf(stdout, "CM_LoadMap: BadBSPError\n");
        }

        if (header.version != BSP_VERSION) {
                fprintf(stdout, "CM_LoadMap: BSPVersionError\n");
        }

	cmod_base = (Byte*) buf;
	CM_LoadSurfaces(&header.lumps[LUMP_TEXINFO]);
	CM_LoadLeafs(&header.lumps[LUMP_LEAFS]);
	CM_LoadLeafBrushes(&header.lumps[LUMP_LEAF_BRUSHES]);
	CM_LoadPlanes(&header.lumps[LUMP_PLANES]);
	CM_LoadBrushes(&header.lumps[LUMP_BRUSHES]);
	CM_LoadBrushSides(&header.lumps[LUMP_BRUSHSIDES]);
	CM_LoadSubModels(&header.lumps[LUMP_MODELS]);
	CM_LoadNodes(&header.lumps[LUMP_NODES]);
	CM_LoadAreas(&header.lumps[LUMP_AREAS]);
	CM_LoadAreaPortals(&header.lumps[LUMP_AREAPORTALS]);
	CM_LoadVisibility(&header.lumps[LUMP_VISIBILITY]);
	CM_LoadEntityString(&header.lumps[LUMP_ENTITIES]);

	buf = Util_Free(buf);

	CM_InitBoxHull();	// there's missing code here
	// and there's missing code here

	memset(portalopen, 0, sizeof(portalopen[0]) * MAX_MAP_AREAPORTALS);
	FloodAreaConnections();

	strncpy(map_name, name, MAX_QPATH);
	map_name[MAX_QPATH - 1] = 0;
	return &map_cmodels[0];
}


/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/common/cmodel.c

Copyright (C) 1997-2001 Id Software, Inc.
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
