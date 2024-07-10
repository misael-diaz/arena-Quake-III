#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "graphics.h"
#include "client.h"
#include "game.h"

extern struct CVar *cl_paused;
extern struct CVar *cl_predict;
extern struct CVar *cl_add_entities;
extern struct CVar *cl_add_particles;
extern struct CVar *cl_add_lights;
extern struct CVar *cl_add_blend;

extern struct Client client;
extern struct ViewRectangle screen_viewRectangle;

static struct Entity entities[MAX_NUM_ENTITIES];
static struct Particle particles[MAX_NUM_PARTICLES];
static struct DynamicLight dlights[MAX_NUM_DLIGHTS];
static struct LightStyle lightstyles[MAX_NUM_LIGHT_STYLES];

static int numdlights = 0;
static int numentities = 0;
static int numparticles = 0;

void View_CalculateViewValues (void)
{
	// ignore interp for now use prediction to not have to write to frame (tedius)

	if (!cl_predict->data) {
		Q_Shutdown();
		fprintf(stderr, "View_CalculateViewValues: NoLinearInterpolationError");
		exit(EXIT_FAILURE);
	}

	float lerp = client.lerpfrac;// == 1
	float backlerp = 1.0f - lerp;// == 0

	// cannot get at the old player state so ignoring the viewoffset
	VectorCopy(&client.predicted_origin, &client.refresh.vieworg);
	VectorCopy(&client.predicted_angles, &client.refresh.viewangles);

	// not smoothing stair climbing

	// not applying kick effects

	AngleVectors(&client.refresh.viewangles,
		     &client.forward,
		     &client.right,
		     &client.up);

	// cannot add view weapon because we are not writing to frame yet
}

void View_PrepRefresh (void)
{
	Screen_UpdateScreen();
}

void View_ClearScene (void)
{
	numdlights = 0;
	numentities = 0;
	numparticles = 0;
}

void View_RenderView (void) 
{
	if (!client.refresh_prepped) {
		return;
	}

	if (!client.frame.valid && (client.force_refresh || !cl_paused->data)) {

		client.force_refresh = false;

		View_ClearScene();
		Client_AddEntities();

		// TODO: add missing tests

		// missing stereo_separation code

		client.refresh.vieworg.x += 0.0625f;
		client.refresh.vieworg.y += 0.0625f;
		client.refresh.vieworg.z += 0.0625f;

		client.refresh.x = screen_viewRectangle.x;
		client.refresh.y = screen_viewRectangle.y;
		client.refresh.width = screen_viewRectangle.width;
		client.refresh.height = screen_viewRectangle.height;

		// HARDCODED FOV Field Of View (see what we do in Refresh_ViewChanged())
		client.refresh.fov_x = 90.0f;
		client.refresh.fov_y = 90.0f;

		// TODO find out what sets client.time and what's done to client.refresh.time
		client.refresh.time = 0.001f * ((float) client.time);

		client.refresh.areabits = client.frame.areabits;

		if (!cl_add_entities->data) {
			numentities = 0;
		}

		if (!cl_add_particles->data) {
			numparticles = 0;
		}

		if (!cl_add_lights->data) {
			numdlights = 0;
		}

		if (!cl_add_blend->data) {
			// NOTE: original source uses VectorClear and this would leave
			//       the last element of blend (blend[3]) unchanged, not
			//       sure if this was intentional. Have found that blend[3]
			//       stores "alpha". blend[3] is read by R_CalcPalette() it
			//       does different things depending on the actual value so
			//       find out more about it.
			size_t const bytes = 4 * sizeof(client.refresh.blend[0]);
			memset(&client.refresh.blend, 0, bytes);
		}

		client.refresh.num_entities = numentities;
		client.refresh.entities = entities;
		client.refresh.num_particles = numparticles;
		client.refresh.particles = particles;
		client.refresh.num_dlights = numdlights;
		client.refresh.dlights = dlights;
		client.refresh.lightstyles = lightstyles;

		client.refresh.RDFlags = client.frame.playerstate.RDFlags;

		// NOTE: we don't need to sort since we added no entities
	}

	Refresh_RenderFrame(&client.refresh);
	Screen_DrawCrossHair();
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/view.c

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
