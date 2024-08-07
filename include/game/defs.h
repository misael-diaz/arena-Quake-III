#ifndef GUARD_QUAKE_GAME_DEFS_H
#define GUARD_QUAKE_GAME_DEFS_H

/* MY OWN DEFS */

#define NUM_AXES 3
#define NUM_SCREEN_EDGES 4
#define NUM_VIEW_CLIP_PLANES NUM_SCREEN_EDGES

/* QUAKE DEFS */

#define MAX_ENT_CLUSTERS 16
#define MAX_CLIENT_WEAPON_MODELS 20
#define MAX_TOUCH 32
#define MAX_STATS 32
#define MAX_QPATH 64
#define MAX_OSPATH 128
#define MAX_ITEMS 256
#define MAX_MODELS 256
#define MAX_SOUNDS 256
#define MAX_IMAGES 256
#define MAX_CLIENTS 256
#define MAX_LIGHT_STYLES 256
#define MAX_GENERAL 512
#define MAXHEIGHT 1200
#define MAXWIDTH 1600
#define MAX_MD2SKINS 32
#define MAXLIGHTMAPS 4
#define NUM_STACK_SURFACES 1000
#define MIN_NUM_SURFACES NUM_STACK_SURFACES
#define MIPLEVELS 4
#define MODEL_NOVIS_SIZE (MAX_MAP_LEAFS / 8)

#define CS_MODELS      32
#define CS_SOUNDS      (CS_MODELS + MAX_MODELS)
#define CS_IMAGES      (CS_SOUNDS + MAX_SOUNDS)
#define CS_LIGHTS      (CS_IMAGES + MAX_IMAGES)
#define CS_ITEMS       (CS_LIGHTS + MAX_LIGHT_STYLES)
#define CS_PLAYERSKINS (CS_ITEMS + MAX_ITEMS)
#define CS_GENERAL     (CS_PLAYERSKINS + MAX_CLIENTS)

#define MAX_CONFIG_STRINGS (CS_GENERAL + MAX_GENERAL)

#define RDF_UNDERWATER 1
#define RDF_NOWORLDMODEL 2
#define RDF_IRGOGGLES 4
#define RDF_UVGOGGLES 8

// 0 - 2 are axial planes
#define PLANE_X                 0
#define PLANE_Y                 1
#define PLANE_Z                 2

// 3 - 5 are non-axial planes snapped to the nearest
#define PLANE_ANYX              3
#define PLANE_ANYY              4
#define PLANE_ANYZ              5

#define CONTENTS_SOLID   0x00000001
#define CONTENTS_MONSTER 0x02000000
#define CONTENTS_NODE ((int) 0xffffffff)

#define SURFACE_SKY     0x4
#define SURFACE_WARP    0x8
#define SURFACE_FLOWING 0x40

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/game/defs.h

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
