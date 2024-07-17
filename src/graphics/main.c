#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "video.h"
#include "graphics.h"
#include "game.h"
#include "util.h"

extern Display *display;
extern struct CacheSurface *cs_rover;
extern struct CacheSurface *d_initial_rover;
extern Byte *d_viewbuffer;
extern unsigned int *d_8to24table;
extern float *d_scalemip;
extern struct Video Video;
extern int d_spanpixcount;
extern int d_aflatcolor;
extern int d_minmip;
extern bool d_roverwrapped;

extern struct Vector modelorg;
extern int mod_registration_sequence;

struct Entity *currentEntity = NULL;
struct CVar *graphics_mipcap = NULL;
struct CVar *graphics_mipscale = NULL;
struct CVar *graphics_maxsurfs = NULL;
struct CVar *r_lightlevel = NULL;
struct Image *r_notexture_mip = NULL;
struct Model *r_worldmodel = NULL;
struct ModelLeaf *r_viewleaf = NULL;
struct ModelSurface *r_skysurfaces = NULL;
struct ModelVertex *r_skyvertexes = NULL;
struct ModelEdge *r_skyedges = NULL;
int *r_skysurfedges = NULL;
struct CVar *fullbright = NULL;

struct GraphState GraphState;
struct OldRefresh oldRefDef;
struct Refresh refresh;
struct ModelPlane screenEdge[NUM_SCREEN_EDGES];
struct ClipPlane view_clipPlanes[NUM_VIEW_CLIP_PLANES];
struct LightStyle r_lightstyles[MAX_LIGHT_STYLES];
struct Vector r_origin;
struct Vector base_vpn;
struct Vector base_vright;
struct Vector base_vup;
struct Vector vpn;
struct Vector vright;
struct Vector vup;
int *pfrustum_indexes[NUM_VIEW_CLIP_PLANES];
int r_frustum_indexes[NUM_VIEW_CLIP_PLANES * (2 * NUM_AXES)];
int r_leaftovis[MAX_MAP_LEAFS];
int r_vistoleaf[MAX_MAP_LEAFS];
float xOrigin = 0;
float yOrigin = 0;
float xcenter = 0;
float ycenter = 0;
float xscale = 0;
float yscale = 0;
float xscaleinv = 0;
float yscaleinv = 0;
float xscaleshrink = 0;
float yscaleshrink = 0;
int c_faceclip = 0;
int r_polycount = 0;
int r_drawnpolycount = 0;
int r_wholepolycount = 0;
int r_amodels_drawn = 0;
int r_outofsurfaces = 0;
int r_outofedges = 0;
int r_framecount = 1;
int r_oldviewcluster = 0;
int r_viewcluster = 0;
int r_screenwidth = 0;
int r_cnumsurfs = 0;
int r_numVisibleLeafs = 0;

static struct CVar *r_novis = NULL;
static struct CVar *r_drawEntities = NULL;
static struct CVar *vid_gamma = NULL;
static struct CVar *vid_fullscreen = NULL;
static struct CVar *graphics_mode = NULL;
static struct ClipPlane view_clipplanes[4];
static float basemip[NUM_MIPS - 1] = {1.0, 0.5 * 0.8, 0.25 * 0.8};
static unsigned int curpalette[256];
static Byte r_notexture_buffer[1024];

struct Image *Refresh_FindImage (char const *image_name)
{
	return Image_FindImage(image_name);
}

struct Model *Refresh_RegisterModel (char const *model_name)
{
	return Model_RegisterModel(model_name);
}

void Refresh_BeginRegistration (char const *model_name)
{
	Model_BeginRegistration(model_name);
}

void Refresh_EndRegistration (void)
{
	Model_EndRegistration();
}

void Refresh_ViewChanged (struct ViewRectangle *VR)
{
	oldRefDef.viewRect = *VR;
	// hardcoded to the default Field-Of-View FOV of 90 degrees, ((*)) = tan(fov_x,y)
	oldRefDef.fHorizontalFieldOfView = 2.0f * ((1.0f));
	oldRefDef.fVerticalFieldOfView = 2.0f * ((Video.height / Video.width));

	oldRefDef.fViewRectX = (float) oldRefDef.viewRect.x;
	oldRefDef.fViewRectY = (float) oldRefDef.viewRect.y;

	oldRefDef.fViewRectX_adj = oldRefDef.fViewRectX - 0.5f;
	oldRefDef.fViewRectY_adj = oldRefDef.fViewRectY - 0.5f;

	oldRefDef.viewRectX_adj_shift20 = (oldRefDef.viewRect.x << 20) + ((1<<19) - 1);

	oldRefDef.viewRectRight = oldRefDef.viewRect.x + oldRefDef.viewRect.width;

	oldRefDef.fViewRectRight = (float) oldRefDef.viewRectRight;
	oldRefDef.fViewRectRight_adj = oldRefDef.fViewRectRight - 0.5f;

	oldRefDef.viewRectRight_adj_shift20 = (
			(oldRefDef.viewRectRight << 20) + ((1 << 19) - 1)
			);

	// setting the x, y, width, height, screenedge (plane) normals, and the rightedge
	// x-coordinate we are done defining the frustum; check my sketches and math
	oldRefDef.fViewRectRightEdge = oldRefDef.fViewRectRight - 0.99f;

	oldRefDef.viewRectBottom = oldRefDef.viewRect.y + oldRefDef.viewRect.height;
	oldRefDef.fViewRectBottom = (float) oldRefDef.viewRectBottom;
	oldRefDef.fViewRectBottom_adj = oldRefDef.fViewRectBottom - 0.5f;

	// NOTE: xOrigin = yOrigin = (1 - xOrigin) = (1 - yOrigin) = 0.5 (screenedges)
	xOrigin = oldRefDef.xOrigin;
	yOrigin = oldRefDef.yOrigin;

	xcenter = (((float) oldRefDef.viewRect.width) * XCENTERING) +
		  (((float) oldRefDef.viewRect.x) - 0.5f);
	ycenter = (((float) oldRefDef.viewRect.height) * YCENTERING) +
		  (((float) oldRefDef.viewRect.y) - 0.5f);

	// we end up with xscale = yscale anyways as in the original source
	xscale = ((float) oldRefDef.viewRect.width) / oldRefDef.fHorizontalFieldOfView;
	yscale = ((float) oldRefDef.viewRect.height) / oldRefDef.fVerticalFieldOfView;

	xscaleinv = 1.0f / xscale;
	yscaleinv = 1.0f / yscale;

	xscaleshrink = (oldRefDef.viewRect.width - 6) / oldRefDef.fHorizontalFieldOfView;
	yscaleshrink = xscaleshrink;

	// left side clip
	screenEdge[0].normal.x = -1.0f / (xOrigin * oldRefDef.fHorizontalFieldOfView);
	screenEdge[0].normal.y = +0.0f;
	screenEdge[0].normal.z = +1.0f;
	screenEdge[0].type = PLANE_ANYZ;
	
	// right side clip
	screenEdge[1].normal.x = +1.0f / (xOrigin * oldRefDef.fHorizontalFieldOfView);
	screenEdge[1].normal.y = +0.0f;
	screenEdge[1].normal.z = +1.0f;
	screenEdge[1].type = PLANE_ANYZ;
	
	// top side clip
	screenEdge[2].normal.x = +0.0f;
	screenEdge[2].normal.y = -1.0f / (yOrigin * oldRefDef.fVerticalFieldOfView);
	screenEdge[2].normal.z = +1.0f;
	screenEdge[2].type = PLANE_ANYZ;

	// bottom side clip
	screenEdge[3].normal.x = +0.0f;
	screenEdge[3].normal.y = +1.0f / (yOrigin * oldRefDef.fVerticalFieldOfView);
	screenEdge[3].normal.z = +1.0f;
	screenEdge[3].type = PLANE_ANYZ;

	for (int i = 0; i != NUM_SCREEN_EDGES; ++i) {
		VectorNormalize(&screenEdge[i].normal);
	}

	Driver_ViewChanged();
}

static void Refresh_BindFrustumIndexes (void)
{
	int *pindex = r_frustum_indexes;
	for (int i = 0; i != NUM_VIEW_CLIP_PLANES; ++i) {
		pfrustum_indexes[i] = pindex;
	}
}

// transforms A * u = v, where A is a matrix and u and v are vectors (see sketches)
void Refresh_TransformFrustum (void)
{
	for (int i = 0; i != NUM_VIEW_CLIP_PLANES; ++i) {
		struct Vector u;
		u.x = +screenEdge[i].normal.z;	// vpn (or vforward) is the new Z axis
		u.y = -screenEdge[i].normal.x;	// vright is the new -X axis
		u.z = +screenEdge[i].normal.y;	// vup is the new Y axis

		struct Vector v;
		v.x = u.x * vpn.x + u.y * vright.x + u.z * vup.x;
		v.y = u.x * vpn.y + u.y * vright.y + u.z * vup.y;
		v.z = u.x * vpn.z + u.y * vright.z + u.z * vup.z;

		VectorNormalize(&v);
		VectorCopy(&v, &view_clipPlanes[i].normal);
		view_clipPlanes[i].dist = DotProduct(&v, &modelorg);
	}
}

void Refresh_SetUpFrustumIndexes (void)
{
	int *pindex = r_frustum_indexes;
	for (int i = 0; i != NUM_VIEW_CLIP_PLANES; ++i) {
		float const *normal = (float const*) &view_clipPlanes[i].normal.x;
		for (int j = 0; j != NUM_AXES; ++j) {
			if (normal[j] < 0.0f) {
				pindex[j] = j;
				pindex[j + NUM_AXES] = (j + NUM_AXES);
			} else {
				pindex[j] = j + NUM_AXES;
				pindex[j + NUM_AXES] = j;
			}
		}
		pindex += (2 * NUM_AXES);
	}
}

void Refresh_MarkLeaves (void)
{
	if (r_oldviewcluster == r_viewcluster && !r_novis->data && r_viewcluster != -1) {
		return;
	}

	Q_Shutdown();
	// we don't expect this to be needed for now so we want to catch this later
	fprintf(stderr, "Refresh_MarkLeaves: ImpError\n");
	exit(EXIT_FAILURE);
}

void Refresh_PushDynamicLights (void)
{
	// dlights is zero so there's nothing to do at this point in dev so we skipped
	return;
}

void Refresh_EdgeDrawing (void)
{
	if (refresh.RDFlags & RDF_NOWORLDMODEL) {
		return;
	}

	Q_Shutdown();
	// we don't expect this to be needed for now so we want to catch this later
	fprintf(stderr, "Refresh_EdgeDrawing: ImpError\n");
	exit(EXIT_FAILURE);
}

void Refresh_DrawAlphaSurfaces (void)
{
	// test not in original but should be unless I missed something
	if (refresh.RDFlags & RDF_NOWORLDMODEL) {
		return;
	}

	Q_Shutdown();
	fprintf(stderr, "Refresh_DrawAlphaSurfaces: ImpError\n");
	exit(EXIT_FAILURE);
}

void Refresh_SetLightLevel (void)
{
	struct Entity *ent = currentEntity;
	if ((refresh.RDFlags & RDF_NOWORLDMODEL) || !r_drawEntities->data || !ent) {
		// NOTE: original code sets it directly contrary to what they say about
		// CVar's should not be changed by external functions
		CVAR_FullSetCVar("r_lightlevel", "150", 0);
		// maybe they meant not to trigger other parts of the code via modified
		// so I am setting it to false here, just a hunch
		r_lightlevel->modified = false;
		return;
	}

	Q_Shutdown();
	// we don't expect this to be needed for now so we want to catch this later
	fprintf(stderr, "Refresh_SetLightLevel: ImpError\n");
	exit(EXIT_FAILURE);
}

void Refresh_CalculatePalette (void)
{
	static bool modified = false;
	float alpha = refresh.blend[3]; // TODO: define RB_ALPHA or someting like that
	if (alpha <= 0) {
		if (modified) {
			Refresh_GammaCorrectAndSetPalette((Byte const*) d_8to24table);
			modified = false;
			return;
		}
		return;
	}

	Q_Shutdown();
	// we don't expect this to be needed for now so we want to catch this later
	fprintf(stderr, "Refresh_CalculatePalette: ImpError\n");
	exit(EXIT_FAILURE);
}

void Refresh_SetupFrame (void)
{
	if (fullbright->modified) {
		fullbright->modified = false;
		Driver_FlushCaches();
	}

	++r_framecount;

	struct ViewRectangle viewRectangle;
	VectorCopy(&oldRefDef.vieworg, &modelorg);
	VectorCopy(&oldRefDef.vieworg, &r_origin);

	AngleVectors(&oldRefDef.viewangles, &vpn, &vright, &vup);

	if (!(refresh.RDFlags & RDF_NOWORLDMODEL)) {
		r_viewleaf = Model_PointInLeaf(&r_origin, r_worldmodel);
		r_viewcluster = r_viewleaf->cluster;
	}

	// hardcoding ViewRectangle to the screen dimensions for simplicity;
	// if the users request a 100% viewsize that's what they would get anyways
	if (!refresh.width || !refresh.height) {
		Q_Shutdown();
		fprintf(stderr, "Refresh_SetupFrame: InitRefreshError\n");
		exit(EXIT_FAILURE);
	}

	if (!refresh.width || !refresh.height) {
		Q_Shutdown();
		fprintf(stderr, "Driver_ViewChanged: InitNewRefDefError\n");
		exit(EXIT_FAILURE);
	}

	viewRectangle.x = refresh.x;
	viewRectangle.y = refresh.y;
	viewRectangle.width = refresh.width;
	viewRectangle.height = refresh.height;

	d_viewbuffer = Video.buffer;
	r_screenwidth = Video.rowBytes;

	Refresh_ViewChanged(&viewRectangle);
	Refresh_TransformFrustum();
	Refresh_SetUpFrustumIndexes();

	VectorCopy(&vpn, &base_vpn);
	VectorCopy(&vright, &base_vright);
	VectorCopy(&vup, &base_vup);

	c_faceclip = 0;
	d_spanpixcount = 0;
	r_polycount = 0;
	r_drawnpolycount = 0;
	r_wholepolycount = 0;
	r_amodels_drawn = 0;
	r_outofsurfaces = 0;
	r_outofedges = 0;

	d_roverwrapped = false;
	d_initial_rover = cs_rover;

	d_minmip = graphics_mipcap->data;
	if (d_minmip > 3) {
		d_minmip = 3;
	} else if (d_minmip < 0) {
		d_minmip = 0;
	}

	for (int i = 0 ; i != (NUM_MIPS - 1); ++i) {
		d_scalemip[i] = basemip[i] * graphics_mipscale->data;
	}

	d_aflatcolor = 0;
}

void Refresh_RenderFrame (struct Refresh *RD)
{
	refresh = *RD;

	VectorCopy(&RD->vieworg, &oldRefDef.vieworg);
	VectorCopy(&RD->viewangles, &oldRefDef.viewangles);

	// TODO: there's some work to do here
	Refresh_SetupFrame();
	Refresh_MarkLeaves();
	Refresh_PushDynamicLights();
	Refresh_EdgeDrawing();
	Refresh_DrawAlphaSurfaces();
	Refresh_SetLightLevel();
	Refresh_CalculatePalette();
}

void Refresh_NewMap (void)
{
	r_viewcluster = -1;
	r_cnumsurfs = graphics_maxsurfs->data;

	if (r_cnumsurfs <= MIN_NUM_SURFACES) {
		r_cnumsurfs = MIN_NUM_SURFACES;
	}

	// TODO: implement
}

void Refresh_Register (void)
{
	fullbright = CVAR_GetCVar("fullbright", "0", 0);
	vid_gamma = CVAR_GetCVar("vid_gamma", "1", 0);
	vid_fullscreen = CVAR_GetCVar("vid_fullscreen", "0", 0);

	graphics_mode = CVAR_GetCVar("graphics_mode", "8", 0);
	graphics_mipcap = CVAR_GetCVar("graphics_mipcap", "0", 0);
	graphics_mipscale = CVAR_GetCVar("graphics_mipscale", "1", 0);
	graphics_maxsurfs = CVAR_GetCVar("graphics_maxsurfs", "0", 0);

	r_novis = CVAR_GetCVar("r_novis", "0", 0);
	r_drawEntities = CVAR_GetCVar("r_drawEntities", "1", 0);
	r_lightlevel = CVAR_GetCVar("r_lightlevel", "0", 0);

	vid_gamma->modified = true;
	graphics_mode->modified = true;
}

void Refresh_InitTextures (void)
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

void Refresh_InitImages (void)
{
	mod_registration_sequence = 1;
}

void Refresh_EndFrame (void)
{
	Graphics_ImpEndFrame();
}

void Refresh_Shutdown (void)
{
	Graphics_ImpShutdown();
}

void Refresh_Free (void)
{
	Graphics_ImpShutdown();
	if (display) {
		XCloseDisplay(display);
		display = NULL;
	}
}

void Refresh_GammaCorrectAndSetPalette (Byte const *palette)
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

void Refresh_InitGraphics (int width, int height)
{
	if (width == 0 || height == 0) {
		fprintf(stderr, "Refresh_InitGraphics: InitError");
		return;
	}

	Video.width = width;
	Video.height = height;

	Refresh_BindFrustumIndexes();
	Refresh_GammaCorrectAndSetPalette((Byte const*) curpalette);
}

void Refresh_BeginFrame (void)
{
	if (vid_gamma->modified) {
		Draw_BuildGammaTable();
		Refresh_GammaCorrectAndSetPalette((Byte const*) curpalette);
		vid_gamma->modified = false;
	}

	if (graphics_mode->modified) {
		enum GraphicsErrorType err = Graphics_ImpSetMode(&Video.width,
								 &Video.height,
								 graphics_mode->data,
								 vid_fullscreen->data);
		if (err) {
			fprintf(stderr, "Refresh_BeginFrame: SetModeError\n");
			Refresh_Shutdown();
			XCloseDisplay(display);
			Util_Clear();
			exit(EXIT_FAILURE);
		}
		Refresh_InitGraphics(Video.width, Video.height);
		GraphState.prev_mode = graphics_mode->data;
		vid_fullscreen->modified = false;
		graphics_mode->modified = false;
	}
}

void Refresh_Init (void)
{
	Refresh_InitImages();
	Model_Init();
	Draw_InitLocal();
	Refresh_InitTextures();

	view_clipplanes[0].leftedge = true;	view_clipplanes[0].rightedge = false;
	view_clipplanes[1].leftedge = false;	view_clipplanes[1].rightedge = true;
	view_clipplanes[2].leftedge = false;	view_clipplanes[2].rightedge = false;
	view_clipplanes[3].leftedge = false;	view_clipplanes[3].rightedge = false;

	oldRefDef.xOrigin = XCENTERING;
	oldRefDef.yOrigin = YCENTERING;

	Refresh_Register();
	Draw_GetPalette();
	Graphics_ImpInit();

	Refresh_BeginFrame();
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
