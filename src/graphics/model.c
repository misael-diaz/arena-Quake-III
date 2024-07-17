#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "util.h"
#include "common.h"
#include "graphics.h"
#include "game.h"

// s, t axes local to the surface
#define _SURFACE_NUM_AXES_ 2
#define _POSITIVE_INFINITY_FLOAT_ 0x7f800000
#define _NEGATIVE_INFINITY_FLOAT_ 0xff800000
#define _DECIMAL_ 10
#define _SIGNMASK_ 0x80000000
#define _FLATTENED_VECTOR_LENGTH_ 8
#define _MAX_MAP_LIGHT_STYLES_ MAXLIGHTMAPS
#define _MAX_NUM_VISIBILITY_CLUSTERS_ 8
#define _NUM_CHILDREN_ 2

#define NUM_VERTEXES_SKYBOX 8
#define NUM_EDGES_SKYBOX 12
#define NUM_SURFACE_EDGES_SKYBOX 24
#define NUM_PLANES_SKYBOX 6
#define NUM_TEXTURES_SKYBOX 6
#define NUM_SURFACES_SKYBOX 6

extern struct Image *r_notexture_mip;
extern struct Model *r_worldmodel;
extern struct ModelSurface *r_skysurfaces;
extern struct ModelVertex *r_skyvertexes;
extern struct ModelEdge *r_skyedges;
extern int *r_skysurfedges;
extern int *r_vistoleaf;
extern int *r_leaftovis;
extern int r_oldviewcluster;
extern int r_numVisibleLeafs;

static union {
	int bin;
	float data;
} _positive_infinity_ = { .bin = _POSITIVE_INFINITY_FLOAT_ };

static union {
	int bin;
	float data;
} _negative_infinity_ = { .bin = _NEGATIVE_INFINITY_FLOAT_ };

static void *mod_hunk = NULL;
static void *mod_base = NULL;
static struct Model *mod_loadmodel = NULL;
static struct Model mod_known[MAX_NUM_KNOWN_MODELS];
static struct Model mod_inline[MAX_NUM_KNOWN_MODELS];
static struct CVar *flushmap = NULL;
static size_t mod_filelen = 0;
static int mod_numknown = 0;

// vertices of the skybox, it's easier to use ones just to store the coordinates these
// will get scaler later to the range [-128, 128] for an extent of 256 for each face;
// the indexes of the vertices range [1, 8] the ZERO index is a DUMMY.
static float box_verts[8][3] = {
	{-1,-1,-1},		// vertex 1
	{-1, 1,-1},		// vertex 2
	{ 1, 1,-1},		// vertex 3
	{ 1,-1,-1},		// vertex 4
	{-1,-1, 1},		// vertex 5
	{-1, 1, 1},		// vertex 6
	{ 1,-1, 1},		// vertex 7
	{ 1, 1, 1}		// vertex 8
};

// the edges array stores the indexes of the vertices, and sense matters here so
// edge 1 points from vertex 1 to vertex 2 but edge -1 points in the opposite direction
// from vertex 2 to vertex 1. Again the ZERO index is a DUMMY.
static int box_edges[24] = {
	1, 2,			// edge 0x1
	2, 3,			// edge 0x2
	3, 4,			// edge 0x3
	4, 1,			// edge 0x4
	1, 5,			// edge 0x5
	5, 6,			// edge 0x6
	6, 2,			// edge 0x7
	7, 8,			// edge 0x8
	8, 6,			// edge 0x9
	5, 7,			// edge 0xA
	8, 3,			// edge 0xB
	7, 4			// edge 0xC
};

// the surfedges array stores the edge indexes, if you make a sketch you will notice that
// the sense of the edges indicate the direction of the unit normal of the surface.
static int box_surfedges[24] = {
	  1,  2,  3,  4,
	 -1,  5,  6,  7,
	  8,  9, -6, 10,
	 -2, -7, -9, 11,
  	 12, -3,-11, -8,
	-12,-10, -5, -4
};

// this one is a little tricky at first, the first column stores the direction of the
// unit normal (0, 1, 2) corresponds to (x, y, z). The second column stores the position
// of the plane.
static int skybox_planes[12] = {
	2,-128,			// xy-plane pointing towards -z, located at z = -128
	0,-128,			// yz-plane pointing towards -x, located at x = -128
	2, 128,			// xy-plane pointing towards +z, located at z =  128
	1, 128,			// xz-plane pointing towards +y, located at y =  128
	0, 128,			// yz-plane pointing towrads +x, located at x =  128
	1,-128			// xz-plane pointing towards -y, located at y = -128
};

// stores the unit vectors tangent to each surface; the sense of the cross product of
// these vectors is consistent with the sense of rotation set by the edges order
struct Vector box_vecs[6][2] = {
	{ { 0,-1, 0}, {-1, 0, 0} },
	{ { 0, 1, 0}, { 0, 0,-1} },
	{ { 0,-1, 0}, { 1, 0, 0} },
	{ { 1, 0, 0}, { 0, 0,-1} },
	{ { 0,-1, 0}, { 0, 0,-1} },
	{ {-1, 0, 0}, { 0, 0,-1} }
};

// this array stores flags for each face of the skybox SURF_PLANEBACK == 2 still need to
// find out more about this
static int box_faces[6] = {
	0,
	0,
	2,
	2,
	2,
	0
};

// NOTE the BOX Arrays are consistent with one another, refer to the same surface/face

int mod_registration_sequence = 0;
Byte mod_novis[MODEL_NOVIS_SIZE];

// NOTE: storing more fields in the header than in the original source for convenience
struct HunkHeader {
	long cursize;
	long maxsize;
	void *data;
	void *hunk;
};

static void *Hunk_Begin (long hunk_maxsize)
{
	static_assert(sizeof(struct HunkHeader) == CACHELINE_SIZE);
	long const hunk_cursize = 0;
	long const headersize = sizeof(struct HunkHeader);
	size_t const bytes = hunk_maxsize + headersize;
	void *p = Util_Malloc(bytes); // not using mmap() as in the original source
	if (!p) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Begin: MallocError\n");
		exit(EXIT_FAILURE);
	}

	memset(p, 0, bytes);
	struct HunkHeader *header = p;
	header->cursize = hunk_cursize;
	header->maxsize = hunk_maxsize;
	header->data = p;
	header->hunk = (p + headersize);

	if (((long) header->hunk) & (CACHELINE_SIZE - 1)) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Begin: HunkAlignError\n");
		exit(EXIT_FAILURE);
	}

	void *hunk = header->hunk;
	return hunk;
}

static long Hunk_End (void **p)
{
	struct HunkHeader *header = (((struct HunkHeader*) *p) - 1);
	if (*p != header->hunk) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_End: InvalidHunkError\n");
		exit(EXIT_FAILURE);
	}

	if (((long) header->hunk) & (CACHELINE_SIZE - 1)) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_End: ImplError\n");
		exit(EXIT_FAILURE);
	}

	long const cursize = header->cursize;

	void *data = Util_Malloc(cursize);
	if (!data) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_End: MallocError\n");
		exit(EXIT_FAILURE);
	}

	header = (struct HunkHeader*) data;
	header->cursize = 0;
	header->maxsize = cursize;
	header->data = data;
	header->hunk = data + sizeof(*header);
	void *hunk = header->hunk;

	void *oldhunk = *p;
	memcpy(hunk, oldhunk, cursize);
	oldhunk = Util_Free(oldhunk);

	*p = hunk;
	return cursize;
}

static void *Hunk_Alloc (void *p, long const size)
{
	struct HunkHeader *header = (((struct HunkHeader*) p) - 1);
	if (p != header->hunk) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Alloc: InvalidHunkError\n");
		exit(EXIT_FAILURE);
	}

	if (((long) header->hunk) & (CACHELINE_SIZE - 1)) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Alloc: ImplError\n");
		exit(EXIT_FAILURE);
	}

	long const sz = ((size + (CACHELINE_SIZE - 1)) + ~(CACHELINE_SIZE - 1));
	if ((header->cursize + sz) > header->maxsize) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Alloc: MemError\n");
		exit(EXIT_FAILURE);
	}

	void *hunk = header->hunk + header->cursize;
	header->cursize += sz;
	return hunk;
}

static void Hunk_Free (void *p)
{
	struct HunkHeader *header = (((struct HunkHeader*) p) - 1);
	if (p != header->hunk) {
		Q_Shutdown();
		fprintf(stderr, "Hunk_Alloc: InvalidHunkError\n");
		exit(EXIT_FAILURE);
	}
	Util_Free(header->data);
	p = NULL;
}

static void Model_FreeExtraData (struct Model *model)
{
	Hunk_Free(model->hunk);
}

void Model_FreeModel (struct Model *model)
{
	Model_FreeExtraData(model);
	memset(model, 0, sizeof(*model));
}

static void Model_SetParent (struct ModelNode* node, struct ModelNode *parent)
{
	node->parent = parent;
	if (node->contents != CONTENTS_NODE) {
		return;
	}

	Model_SetParent(node->children[0], node);
	Model_SetParent(node->children[1], node);
}

static void Model_NumberLeafs (struct ModelNode* node)
{
	if (node->contents != CONTENTS_NODE) {
		struct ModelLeaf *leaf = (struct ModelLeaf*) node;
		if (leaf->contents & CONTENTS_SOLID) {
			return;
		}
		int const leafnum = (leaf - mod_loadmodel->leafs);
		int const visnum = r_numVisibleLeafs;
		r_leaftovis[leafnum] = r_numVisibleLeafs;
		r_vistoleaf[visnum] = leafnum;
		++r_numVisibleLeafs;
		return;
	}

	Model_NumberLeafs(node->children[0]);
	Model_NumberLeafs(node->children[1]);
}

// TODO: there's missing code here; first try to do this on your own then compare against
//       the original source code
static void Model_InitSkyBox (void)
{
	struct Model *model = mod_loadmodel;
	int const numsurfaces = model->numsurfaces;
	int const numtextures = model->numtextures;
	int const numplanes = model->numplanes;
	int const numvertexes = model->numvertexes;
	int const numedges = model->numedges;
	int const numsurfedges = model->numsurfedges;
	r_skysurfaces = &model->surfaces[numsurfaces];
	r_skyvertexes = &model->vertexes[numvertexes];
	r_skysurfedges = &model->surfedges[numsurfedges];
	r_skyedges = &model->edges[numedges];

	// we have to subtract one because the zero edge number is a dummy
	for (int i = 0; i != NUM_SURFACE_EDGES_SKYBOX; ++i) {
		int *surfedges = &model->surfedges[numsurfedges];
		int *surfedge = &surfedges[i];
		int box_surfedge = box_surfedges[i];
		if (box_surfedge > 0) {
			*surfedge = numsurfedges + (box_surfedge - 1);
		} else {
			*surfedge = -(numsurfedges + ((-box_surfedge) - 1));
		}
	}
	
	// setting the normal vector based on my analysis instead of using the original
	for (int i = 0; i != NUM_PLANES_SKYBOX; ++i) {
		struct ModelPlane *planes = &model->planes[numplanes];
		struct ModelPlane *plane = &planes[i];
		int const *skybox_plane_type = &skybox_planes[2 * i];
		int const *skybox_plane_dist = &skybox_planes[2 * i + 1];
		int const axnum = *skybox_plane_type;
		VectorClear(&plane->normal);
		float *normal = (float*) &plane->normal;
		normal[axnum] = (
			(*skybox_plane_dist) / (*skybox_plane_dist & (~_SIGNMASK_))
		);
		plane->dist = *skybox_plane_dist;
		plane->type = *skybox_plane_type;
		if (*skybox_plane_dist & _SIGNMASK_) {
			plane->signbits = (1 << axnum);
		}
	}

	for (int i = 0; i != NUM_TEXTURES_SKYBOX; ++i) {
		struct ModelTexture *textures = &model->textures[numtextures];
		struct ModelTexture *texture = &textures[i];
		struct Vector *s = (struct Vector*) &texture->vecs[0][0];
		struct Vector *t = (struct Vector*) &texture->vecs[1][0];
		VectorCopy(&box_vecs[i][0], s);
		VectorCopy(&box_vecs[i][1], t);
	}

	for (int i = 0; i != NUM_SURFACES_SKYBOX; ++i) {
		struct ModelSurface *surfaces = &model->surfaces[numsurfaces];
		struct ModelSurface *surface = &surfaces[i];
		int const *surfedges = &model->surfedges[numsurfedges];
		int const *surfedge = &surfedges[4 * i];
		surface->firstedge = *surfedge;
		surface->numedges = 4;
		surface->texturemins[0] = -128;
		surface->texturemins[1] = -128;
		surface->extents[0] = 256;
		surface->extents[1] = 256;
	}

	// TODO: update counters (numsurfedges, numedges, etc.)
}

static void Model_LoadVertexes (struct Lump const *lump)
{
	struct DataVertex *_vertexes = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_vertexes)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadVertexes: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_vertexes);
	int const numvertexes = count;
	int const numvertexes_skybox = NUM_VERTEXES_SKYBOX;
	int const totvertexes = numvertexes + numvertexes_skybox;
	struct ModelVertex *vertexes = Hunk_Alloc(mod_hunk,
						 (totvertexes * sizeof(*vertexes)));
	memcpy(vertexes, _vertexes, numvertexes * sizeof(*_vertexes));
	mod_loadmodel->numvertexes = numvertexes;
	mod_loadmodel->vertexes = vertexes;
}

static void Model_LoadEdges (struct Lump const *lump)
{
	struct DataEdge *_edges = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_edges)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadEdges: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_edges);
	int const numedges = count;
	int const numedges_skybox = NUM_EDGES_SKYBOX;
	int const totedges = numedges + numedges_skybox;
	struct ModelEdge *edges = Hunk_Alloc(mod_hunk, totedges * sizeof(*edges));
	for (int i = 0; i != numedges; ++i) {
		struct DataEdge const *_edge = &_edges[i];
		struct ModelEdge *edge = &edges[i];
		unsigned int const *_data = (unsigned int const*) &_edge->vertexnums[0];
		unsigned int *data = (unsigned int*) &edge->vertexnums[0];
		*data = *_data;
	}
	mod_loadmodel->numedges = numedges;
	mod_loadmodel->edges = edges;
}

static void Model_LoadSurfaceEdges (struct Lump const *lump)
{
	int *_surfedges = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_surfedges)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadSurfaceEdges: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_surfedges);
	int const numsurfedges = count;
	int const numsurfedges_skybox = NUM_SURFACE_EDGES_SKYBOX;
	int const totsurfedges = numsurfedges + numsurfedges_skybox;
	int *surfedges = Hunk_Alloc(mod_hunk,
				   (totsurfedges * sizeof(*surfedges)));
	memcpy(surfedges, _surfedges, numsurfedges * sizeof(*_surfedges));
	mod_loadmodel->numsurfedges = numsurfedges;
	mod_loadmodel->surfedges = surfedges;
}

static void Model_LoadLighting (struct Lump const *lump)
{
	Byte const *_dlights = (mod_base + lump->fileofs);
	if (!lump->filelen || lump->filelen % (3 * sizeof(*_dlights))) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadLighting: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / (3 * sizeof(*_dlights));
	int const numdlights = count;
	Byte *dlights = Hunk_Alloc(mod_hunk, numdlights * sizeof(*dlights));

	Byte *dlight = dlights;
	Byte const *_dlight = _dlights;
	for (int i = 0; i != numdlights; ++i) {
		if ((_dlight[0] > _dlight[1]) && (_dlight[0] > _dlight[2])) {
			*dlight = _dlight[0];
		} else if ((_dlight[1] > _dlight[0]) && (_dlight[1] > _dlight[2])) {
			*dlight = _dlight[1];
		} else {
			*dlight = _dlight[2];
		}
		_dlight += (3 * sizeof(*_dlight));
		dlight += sizeof(*dlight);
	}

	mod_loadmodel->lightdata = dlights;
}

static void Model_LoadPlanes (struct Lump const *lump)
{
	struct DataPlane *_planes = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_planes)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadPlanes: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_planes);
	int const numplanes = count;
	int const numplanes_skybox = NUM_PLANES_SKYBOX;
	int const totplanes = numplanes + numplanes_skybox;
	struct ModelPlane *planes = Hunk_Alloc(mod_hunk, totplanes * sizeof(*planes));
	for (int i = 0; i != numplanes; ++i) {
		struct DataPlane const *_plane = &_planes[i];
		struct ModelPlane *plane = &planes[i];
		float const *_normal = &_plane->normal.x;
		float *normal = &plane->normal.x;
		for (int j = 0; j != NUM_AXES; ++j) {
			normal[j] = _normal[j];
		}

		int bits = 0;
		int const *n = (int const*) normal;
		for (int j = 0; j != NUM_AXES; ++j) {
			int const ax = n[j];
			if (ax & _SIGNMASK_) {
				bits |= (1 << j);
			}
		}

		plane->type = _plane->type;
		plane->dist = _plane->dist;
		plane->signbits = bits;
	}

	mod_loadmodel->numplanes = numplanes;
	mod_loadmodel->planes = planes;
}

static void Model_LoadTexture (struct Lump const *lump)
{
	struct InfoTexture *_textures = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_textures)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadTexture: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_textures);
	int const numtextures = count;
	int const numtextures_skybox = NUM_TEXTURES_SKYBOX;
	int const numtexs = numtextures + numtextures_skybox;
	struct ModelTexture *textures = Hunk_Alloc(mod_hunk, numtexs * sizeof(*textures));
	for (int i = 0; i != numtextures; ++i) {
		struct InfoTexture const *_texture = &_textures[i];
		struct ModelTexture *texture = &textures[i];
		float const *_flatvector = &_texture->vecs[0][0];
		float *flatvector = &texture->vecs[0][0];
		for (int j = 0; j != _FLATTENED_VECTOR_LENGTH_; ++j) {
			flatvector[j] = _flatvector[j];
		}
	}

	for (int i = 0; i != numtextures; ++i) {
		struct ModelTexture *texture = &textures[i];
		struct Vector const *u = (struct Vector const*) &texture->vecs[0][0];
		struct Vector const *v = (struct Vector const*) &texture->vecs[1][0];
		float const len_u = VectorLength(u);
		float const len_v = VectorLength(v);
		float const len = 0.5f * (len_u + len_v);
		if (len < 0.32f) {
			texture->mipAdjust = 4;
		} else if (len < 0.49f) {
			texture->mipAdjust = 3;
		} else if (len < 0.99f) {
			texture->mipAdjust = 2;
		} else {
			texture->mipAdjust = 1;
		}
	}

	for (int i = 0; i != numtextures; ++i) {
		struct InfoTexture const *_texture = &_textures[i];
		struct ModelTexture *texture = &textures[i];
		texture->flags = _texture->flags;
	}

	for (int i = 0; i != numtextures; ++i) {
		struct InfoTexture const *_texture = &_textures[i];
		struct ModelTexture *texture = &textures[i];
		int const next = _texture->nextInfoTexture;
		if (next > 0) {
			texture->next = &textures[next];
		} else {
			texture->next = NULL;
		}
	}

	for (int i = 0; i != numtextures; ++i) {
		struct InfoTexture const *_texture = &_textures[i];
		struct ModelTexture *texture = &textures[i];
		char const *image_name = _texture->name;
		char img_fullpath[MAX_QPATH];
		int rc = snprintf(img_fullpath, MAX_QPATH, "textures/%s.wal", image_name);
		if (rc >= MAX_QPATH) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadTexture: TruncationError\n");
			exit(EXIT_FAILURE);
		}
		texture->image = Refresh_FindImage(img_fullpath);
		if (!texture->image) {
			texture->image = r_notexture_mip;
			texture->flags = 0;
		}
	}

	for (int i = 0; i != numtextures; ++i) {
		struct ModelTexture *texture = &textures[i];
		texture->numframes = 1;
		struct ModelTexture *tex = NULL;
		for (tex = texture->next; tex && tex != texture; tex=tex->next) {
			++(texture->numframes);
		}
	}

	mod_loadmodel->numtextures = numtextures;
	mod_loadmodel->textures = textures;
}

// Determines the extent of the surface (width and height), the extent of the surface must
// be a multiple of 16 so the code guarantees that (need to find out the reason for this)
// To perform this calculation the code uses orthogonal axes local to the surface.
static void CalculateSurfaceExtents (struct ModelSurface *surface)
{
	int bmins[2];
	int bmaxs[2];
	float mins[2];
	float maxs[2];
	float const positive_infinity = _positive_infinity_.data;
	float const negative_infinity = _negative_infinity_.data;
	mins[0] = mins[1] = positive_infinity;
	maxs[0] = maxs[1] = negative_infinity;
	struct ModelTexture const *texture = surface->texture;
	int const firstedge = surface->firstedge;
	int const *surfaceEdgeNums = &mod_loadmodel->surfedges[firstedge];
	for (int i = 0; i != surface->numedges; ++i) {
		int const *surfaceEdgeNum = &surfaceEdgeNums[i];
		int const *surfEdgeNum = surfaceEdgeNum;
		int const edgenum = (*surfEdgeNum >= 0)? (*surfEdgeNum) : -(*surfEdgeNum);
		struct ModelEdge const *edge = &mod_loadmodel->edges[edgenum];
		unsigned short vertexnum;
		if (*surfEdgeNum >= 0) {
			vertexnum = edge->vertexnums[0];
		} else {
			vertexnum = edge->vertexnums[1];
		}
		struct ModelVertex *vertex = &mod_loadmodel->vertexes[vertexnum];

		// TODO: find out more about the texture offset
		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			struct Vector const *ax = (struct Vector const*)
						  &texture->vecs[axnum][0];
			float const offset = texture->vecs[axnum][3];
			struct Vector const *position = &vertex->position;
			float const value = DotProduct(position, ax) + offset;
			if (value < mins[axnum]) {
				mins[axnum] = value;
			}

			if (value > maxs[axnum]) {
				maxs[axnum] = value;
			}
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			bmaxs[axnum] = ceilf(0.0625f * maxs[axnum]); // 0.0625 = 1 / 16
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			bmins[axnum] = floorf(0.0625f * mins[axnum]);
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			surface->texturemins[axnum] = 16 * bmins[axnum];
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			surface->extents[axnum] = 16 * (bmaxs[axnum] - bmins[axnum]);
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			if (surface->extents[axnum] < 16) {
				surface->extents[axnum] = 16;
			}
		}

		for (int axnum = 0; axnum != _SURFACE_NUM_AXES_; ++axnum) {
			if (!(texture->flags & (SURFACE_WARP | SURFACE_SKY)) &&
			     (surface->extents[axnum] > 256)) {
				Q_Shutdown();
				fprintf(stderr,
					"CalculateSurfaceExtents: BadSurfaceExtents\n");
			}
		}
	}
}

static void Model_LoadSurfaces (struct Lump const *lump)
{
	struct DataSurface const *_surfaces = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_surfaces)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadSurfaces: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_surfaces);
	int const numsurfaces = count;
	int const numsurfaces_skybox = NUM_SURFACES_SKYBOX;
	int const totsurfaces = numsurfaces + numsurfaces_skybox;
	size_t const total = totsurfaces;
	struct ModelSurface *surfaces = Hunk_Alloc(mod_hunk, total * sizeof(*surfaces));
	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		unsigned short const planenum = _surface->planenum;
		surface->plane = &mod_loadmodel->planes[planenum];
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		surface->firstedge = _surface->firstedge;
		surface->numedges = _surface->numedges;
		if (surface->numedges < 3) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadSurfaces: MinNumEdgesError\n");
			exit(EXIT_FAILURE);
		}
		surface->flags = 0;
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		short const side = _surface->side;
		if (side) {
			surface->flags |= SURFACE_BACKPLANE;
		}
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		unsigned short const texturenum = _surface->texturenum;
		surface->texture = &mod_loadmodel->textures[texturenum];
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		size_t const bytes = _MAX_MAP_LIGHT_STYLES_ * sizeof(*(_surface->styles));
		memcpy(surface->styles, _surface->styles, bytes);
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct ModelSurface *surface = &surfaces[i];
		CalculateSurfaceExtents(surface);
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct DataSurface const *_surface = &_surfaces[i];
		struct ModelSurface *surface = &surfaces[i];
		int const lightofs = _surface->lightofs;
		if (lightofs == -1) {
			surface->samples = NULL;
		} else {
			int const lightnum = (lightofs / 3);
			surface->samples = &mod_loadmodel->lightdata[lightnum];
		}
	}

	for (int i = 0; i != numsurfaces; ++i) {
		struct ModelSurface *surface = &surfaces[i];
		if (!surface->texture->image) {
			continue;
		}

		if (surface->texture->flags & SURFACE_SKY) {
			surface->flags |= SURFACE_DRAW_SKY;
			continue;
		}

		if (surface->texture->flags & SURFACE_WARP) {
			surface->flags |= SURFACE_DRAW_TURBULENCE;
			surface->extents[0] = (0x4000);
			surface->extents[1] = (0x4000);
			surface->texturemins[0] = -(0x2000);
			surface->texturemins[1] = -(0x2000);
			continue;
		}

		if (surface->texture->flags & SURFACE_FLOWING) {
			surface->flags |= (SURFACE_DRAW_TURBULENCE | SURFACE_FLOW);
			surface->extents[0] = (0x4000);
			surface->extents[1] = (0x4000);
			surface->texturemins[0] = -(0x2000);
			surface->texturemins[1] = -(0x2000);
			continue;
		}

	}

	mod_loadmodel->numsurfaces = numsurfaces;
	mod_loadmodel->surfaces = surfaces;
}

static void Model_MarkSurfaces (struct Lump const *lump)
{
	short const *_surfacenums = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_surfacenums)) {
		Q_Shutdown();
		fprintf(stderr, "Model_MarkSurfaces: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_surfacenums);
	int const numMarkedSurfaces = count;
	int const total = numMarkedSurfaces;
	int const tot = total;
	struct ModelSurface **markedSurfaces = Hunk_Alloc(mod_hunk,
							  tot * sizeof(*markedSurfaces));
	for (int i = 0; i != numMarkedSurfaces; ++i) {
		short const *_surfacenum = &_surfacenums[i];
		int const surfacenum = *_surfacenum;
		if (surfacenum >= mod_loadmodel->numsurfaces) {
			Q_Shutdown();
			fprintf(stderr, "Model_MarkSurfaces: WrongSurfaceNumberError\n");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i != numMarkedSurfaces; ++i) {
		short const *_surfacenum = &_surfacenums[i];
		int const surfacenum = *_surfacenum;
		struct ModelSurface **markedSurface = &markedSurfaces[i];
		*markedSurface = &mod_loadmodel->surfaces[surfacenum];
	}

	mod_loadmodel->numMarkedSurfaces = numMarkedSurfaces;
	mod_loadmodel->markedSurfaces = markedSurfaces;
}

static void Model_LoadVisibility (struct Lump const *lump)
{
	struct DataVisibility *_visibility = (mod_base + lump->fileofs);
	if (!lump->filelen || (lump->filelen < ((int) sizeof(*_visibility)))) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadVisibility: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	// TODO: maybe check if the visibility offsets exceed the filelen

	struct DataVisibility *visibility = Hunk_Alloc(mod_hunk, lump->filelen);
	memcpy(visibility, _visibility, lump->filelen);

	int const numclusters = visibility->numclusters;
	fprintf(stdout, "Model_LoadVisibility: numclusters: %d\n", numclusters);
	if (numclusters >= _MAX_NUM_VISIBILITY_CLUSTERS_) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadVisibility: MaxNumVisibilityClustersError\n");
		exit(EXIT_FAILURE);
	}

	mod_loadmodel->visibility = visibility;
}

static void Model_LoadLeafs (struct Lump const *lump)
{
	struct DataLeaf const *_leafs = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_leafs)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadLeafs: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_leafs);
	int const numleafs = count;
	struct ModelLeaf *leafs = Hunk_Alloc(mod_hunk, numleafs * sizeof(*leafs));
	for (int i = 0; i != numleafs; ++i) {
		struct DataLeaf const *_leaf = &_leafs[i];
		struct ModelLeaf *leaf = &leafs[i];
		leaf->contents = _leaf->contents;
		leaf->cluster = _leaf->cluster;
		leaf->area = _leaf->area;
	}

	for (int i = 0; i != numleafs; ++i) {
		struct DataLeaf const *_leaf = &_leafs[i];
		struct ModelLeaf *leaf = &leafs[i];

		for (int j = 0; j != 3; ++j) {
			leaf->minmaxs[j] = (int) _leaf->mins[j];
		}

		for (int j = 0; j != 3; ++j) {
			leaf->minmaxs[3 + j] = (int) _leaf->maxs[j];
		}
	}

	for (int i = 0; i != numleafs; ++i) {
		struct DataLeaf const *_leaf = &_leafs[i];
		struct ModelLeaf *leaf = &leafs[i];
		unsigned int markedSurfaceNum = _leaf->firstLeafFace;
		unsigned int surfnum = markedSurfaceNum;
		struct ModelSurface **leafFace = &mod_loadmodel->markedSurfaces[surfnum];
		struct ModelSurface **firstMarkedSurface = leafFace;
		leaf->firstMarkedSurface = firstMarkedSurface;
		leaf->numMarkedSurfaces = _leaf->numLeafFaces;
	}

	mod_loadmodel->numleafs = numleafs;
	mod_loadmodel->leafs = leafs;
}

static void Model_LoadNodes (struct Lump const *lump)
{
	struct DataNode const *_nodes = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_nodes)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadNodes: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_nodes);
	int const numnodes = count;
	struct ModelNode *nodes = Hunk_Alloc(mod_hunk, numnodes * sizeof(*nodes));
	for (int i = 0; i != numnodes; ++i) {
		struct DataNode const *_node = &_nodes[i];
		struct ModelNode *node = &nodes[i];
		node->contents = CONTENTS_NODE;
		node->firstsurface = _node->firstsurface;
		node->numsurfaces = _node->numsurfaces;
	}
	
	for (int i = 0; i != numnodes; ++i) {
		struct DataNode const *_node = &_nodes[i];
		struct ModelNode *node = &nodes[i];

		for (int j = 0; j != 3; ++j) {
			node->minmaxs[j] = (int) _node->mins[j];
		}

		for (int j = 0; j != 3; ++j) {
			node->minmaxs[3 + j] = (int) _node->maxs[j];
		}
	}

	for (int i = 0; i != numnodes; ++i) {
		struct DataNode const *_node = &_nodes[i];
		struct ModelNode *node = &nodes[i];
		unsigned int planenum = _node->planenum;
		struct ModelPlane *plane = &mod_loadmodel->planes[planenum];
		node->plane = plane;
	}
	
	for (int i = 0; i != numnodes; ++i) {
		struct DataNode const *_node = &_nodes[i];
		struct ModelNode *node = &nodes[i];
		int const *children = _node->children;
		for (int j = 0; j != _NUM_CHILDREN_; ++j) {
			int const nodenum = children[j];
			int const leafnum = -1 - nodenum;
			struct ModelNode **child = &node->children[j];
			if (nodenum >= 0) {
				*child = &mod_loadmodel->nodes[nodenum];
			} else {
				struct ModelLeaf *leaf = &mod_loadmodel->leafs[leafnum];
				*child = (struct ModelNode*) leaf;
			}
		}
	}

	Model_SetParent(nodes, NULL);
	mod_loadmodel->numnodes = numnodes;
	mod_loadmodel->nodes = nodes;
}

static void Model_LoadSubModels (struct Lump const *lump)
{
	struct DataModel const *_submodels = (mod_base + lump->fileofs);
	if (lump->filelen % sizeof(*_submodels)) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadSubModels: WrongLumpSizeError\n");
		exit(EXIT_FAILURE);
	}

	int const count = lump->filelen / sizeof(*_submodels);
	int const numsubmodels = count;
	int const total = numsubmodels;
	int const bytes = total * sizeof(*_submodels);
	struct DataModel *submodels = Hunk_Alloc(mod_hunk, bytes);
	memcpy(submodels, _submodels, bytes);
	for (int i = 0; i != numsubmodels; ++i) {
		struct DataModel *submodel = &submodels[i];

		float *mins = (float*) &submodel->mins;
		for (int j = 0; j != 3; ++j) {
			float *min = &mins[j];
			*min -= 1.0f; 
		}

		float *maxs = (float*) &submodel->maxs;
		for (int j = 0; j != 3; ++j) {
			float *max = &maxs[j];
			*max += 1.0f; 
		}
	}

	mod_loadmodel->numsubmodels = numsubmodels;
	mod_loadmodel->submodels = submodels;
}

static void Model_LoadBrushModel (struct Model *model, void *buffer)
{
	if (!model) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadBrushModel: NullModelError\n");
		exit(EXIT_FAILURE);
	}

	if (model != mod_known) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadBrushModel: BrushModelAfterWorldModelError\n");
		exit(EXIT_FAILURE);
	}

	model->type = MOD_BRUSH;

	struct DataHeader *header = (struct DataHeader*) buffer;
	if (BSP_VERSION != header->version) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadBrushModel: BSPVersionError\n");
		exit(EXIT_FAILURE);
	}

	mod_base = (Byte*) header;

	Model_LoadVertexes(&header->lumps[LUMP_VERTEXES]);
	Model_LoadEdges(&header->lumps[LUMP_EDGES]);
	Model_LoadSurfaceEdges(&header->lumps[LUMP_SURFEDGES]);
	Model_LoadLighting(&header->lumps[LUMP_LIGHTING]);
	Model_LoadPlanes(&header->lumps[LUMP_PLANES]);
	Model_LoadTexture(&header->lumps[LUMP_TEXINFO]);
	Model_LoadSurfaces(&header->lumps[LUMP_FACES]);
	Model_MarkSurfaces(&header->lumps[LUMP_LEAFFACES]);
	Model_LoadVisibility(&header->lumps[LUMP_VISIBILITY]);
	Model_LoadLeafs(&header->lumps[LUMP_LEAFS]);
	Model_LoadNodes(&header->lumps[LUMP_NODES]);
	Model_LoadSubModels(&header->lumps[LUMP_MODELS]);

	r_numVisibleLeafs = 0;
	Model_NumberLeafs(model->nodes);

	// TODO: add missing code here to load the submodels

	for (int i = 0; i != model->numsubmodels; ++i) {
		struct Model *starmodel = &mod_inline[i];
		struct DataModel *submodel = &model->submodels[i];
		struct DataModel *brushmodel = submodel;
		struct DataModel *bm = brushmodel;
		*starmodel = *model;
		starmodel->firstModelSurface = bm->firstsurface;
		starmodel->numModelSurfaces = bm->numsurfaces;
		starmodel->firstnode = bm->headnode;
		if (starmodel->firstnode >= model->numnodes) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadBrushModel: WrongFirstNode\n");
			exit(EXIT_FAILURE);
		}

		VectorCopy(&bm->mins, &starmodel->mins);
		VectorCopy(&bm->maxs, &starmodel->maxs);

		if (i == 0) {
			*model = *starmodel;
		}
	}

	Model_InitSkyBox();
}

struct Model *Model_LoadModel (char const *model_name)
{
	if (!model_name[0]) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadModel: NullModelNameError\n");
		exit(EXIT_FAILURE);
	}

	if ('*' == model_name[0]) {
		if (!r_worldmodel) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: NullWorldModelError\n");
			exit(EXIT_FAILURE);
		}

		if (!model_name[1]) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: NullInlineModelNameError\n");
			exit(EXIT_FAILURE);
		}

		errno = 0;
		char *endptr = NULL;
		int const base = _DECIMAL_;
		int const submodnum = strtol(&model_name[1], &endptr, base);
		if (errno) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (model_name[1] == *endptr) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: InvalidInlineModelNameError\n");
			exit(EXIT_FAILURE);
		}

		if (submodnum < 1 || submodnum >= r_worldmodel->numsubmodels) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: InvalidInlineModelNumError\n");
			exit(EXIT_FAILURE);
		}

		return &mod_inline[submodnum];
	}

	int count = 0;
	struct Model *models = mod_known;
	for (count = 0; count != mod_numknown; ++count) {
		struct Model *model = models;
		if (!strcmp(model->name, model_name)) {
			return model;
		}
		++models;
	}

	if (count == mod_numknown) {
		if (MAX_NUM_KNOWN_MODELS == mod_numknown) {
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: MaxKnownModelsError\n");
			exit(EXIT_FAILURE);
		}
		++mod_numknown;
	}

	unsigned int *buf = NULL;
	mod_filelen = FS_FLoadFile(model_name, (void**) &buf);
	if (!buf) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadModel: MallocError\n");
		exit(EXIT_FAILURE);
	}

	struct Model *model = models;
	if (model != &mod_known[mod_numknown]) {
		Q_Shutdown();
		fprintf(stderr, "Model_LoadModel: ImplError\n");
		exit(EXIT_FAILURE);
	}

	strncpy(model->name, model_name, MAX_QPATH);

	switch (*buf) {
		case ID_BSP_HEADER:
			model->hunk = Hunk_Begin(0x1000000);
			mod_hunk = model->hunk;
			mod_loadmodel = model;
			Model_LoadBrushModel(model, buf);
			long const hunksize = Hunk_End(&model->hunk);
			// NOTE:
			// We have to load the whole model again because all pointers
			// point to the oldhunk addresses which have just been invalidated
			// by free(); this is possibly one of the reasons the Quake devs
			// use mmap().
			mod_hunk = model->hunk;
			mod_loadmodel = model;
			model->hunksize = hunksize;
			Model_LoadBrushModel(model, buf);
			break;
		default:
			Q_Shutdown();
			fprintf(stderr, "Model_LoadModel: ImplError\n");
			exit(EXIT_FAILURE);
	}

	buf = Util_Free(buf);
	return model;
}

struct ModelLeaf *Model_PointInLeaf (struct Vector const *r, struct Model *model)
{
	if (!model || !model->nodes) {
		Q_Shutdown();
		fprintf(stderr, "Model_PointInLeaf: BadModelError\n");
		exit(EXIT_FAILURE);
	}

	struct ModelNode *node = model->nodes;
	while (true) {
		if (node->contents != -1) {
			return (struct ModelLeaf*) node;
		}

		struct ModelPlane *plane = node->plane;
		if (!plane) {
			Q_Shutdown();
			fprintf(stderr, "Model_PointInLeaf: NullPlaneImpError\n");
			exit(EXIT_FAILURE);
		}

		float d = DotProduct(r, &plane->normal) - plane->dist;
		if (d > 0) {
			node = node->children[0];
		} else {
			node = node->children[1];
		}
	}

	Q_Shutdown();
	fprintf(stderr, "Model_PointInLeaf: ImpError\n");
	exit(EXIT_FAILURE);
	return NULL;
}

void Model_BeginRegistration (char const *model_name)
{
	char model_fullname[MAX_QPATH];
	if (strlen(model_name) + 9 >= MAX_QPATH) {
		Q_Shutdown();
		fprintf(stderr, "Model_BeginRegistration: InvalidMapName\n");
		exit(EXIT_FAILURE);
	}

	r_oldviewcluster = -1;
	snprintf(model_fullname, MAX_QPATH, "maps/%s.bsp", model_name);

	Driver_FlushCaches();
	flushmap = CVAR_GetCVar("flushmap", "1", 0);
	// as in the original source we are ensuring that the world map is mod_known[0]
	if (flushmap->data || strcmp(mod_known[0].name, model_fullname)) {
		Model_FreeModel(&mod_known[0]);
	}

	r_worldmodel = Refresh_RegisterModel(model_fullname);
	Refresh_NewMap();
}

void Model_EndRegistration (void)
{
	return;
}

struct Model *Model_RegisterModel (char const *model_name)
{
	Q_Shutdown();
	fprintf(stderr, "Model_RegisterModel: ImplError\n");
	exit(EXIT_FAILURE);
}

void Model_Init (void)
{
        memset(mod_novis, 255, MODEL_NOVIS_SIZE);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/graphics/model.c

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
