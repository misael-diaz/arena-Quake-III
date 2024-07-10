#ifndef GUARD_QUAKE_GAME_STRUCTS_GAME_H
#define GUARD_QUAKE_GAME_STRUCTS_GAME_H

#include "../../graphics/model/structs/model.h"
#include "../../common/structs/cmodel.h"

enum Solid {
	SOLID_NOT,
	SOLID_TRIGGER,
	SOLID_BBOX,
	SOLID_BSP
}; // solid_t

enum PredictMovementType {
	PM_NORMAL,
	PM_SPECTATOR,
	PM_DEAD,
	PM_GIB,
	PM_FREEZE
}; // pmtype_t

struct Link {
	struct Link *prev;
	struct Link *next;
}; // link_t

struct EntityState {
        struct Vector origin;
        struct Vector angles;
        struct Vector old_origin;
        int number;
        int modelindex;
        int modelindex2;
	int modelindex3;
	int modelindex4;
        int frame;
        int skinnum;
        unsigned int effects;
        int renderfx;
        int solid;
        int sound;
        int event;
}; // entity_state_t

struct UserCommand {
	struct Vector angles;
	float forwardmove;
	float sidemove;
	float upmove;
	Byte buttons;
	Byte lightlevel;
	Byte impulse;
	Byte msec;
}; // usercmd_t

/* we are not going to send anything over the network so can use floats */
struct PredictMovementState {
        struct Vector origin;
        struct Vector velocity;
        struct Vector delta_angles;
        float gravity;
	enum PredictMovementType pm_type;
        Byte pm_flags;
        Byte pm_time;
}; // pmove_state_t

struct PlayerState {
	struct PredictMovementState pms;
	struct Vector viewangles;
	struct Vector viewoffset;
	struct Vector kickangles;
	struct Vector gunangles;
	struct Vector gunoffset;
	float blend[4];
	float fov;
	int gunindex;
	int gunframe;
	int RDFlags;
	short stats[MAX_STATS];
}; // player_state_t

/* hiding what's not needed for now */

/*
struct GameClient {
        struct PlayerState ps;
}; // gclient_t

struct EntityDict {
	struct EntityState state;
	struct GameClient *client;
	struct Entity *owner;
	struct Vector mins;
	struct Vector maxs;
	struct Vector absmin;
	struct Vector absmax;
	struct Vector size;
	struct Link area;
	enum Solid solid;
	int clusternums[MAX_ENT_CLUSTERS];
	int linkcount;
	int numclusters;
	int headnode;
	int areanum;
	int areanum2;
	int svflags;
	int clipmask;
	bool inuse;
}; // edict_t

struct Trace {
	struct EntityDict *ent;
	struct CSurface *surface;
	struct Vector endpos;
	struct CPlane plane;
	float fraction;
	int contents;
	bool allsolid;
	bool startsolid;
}; // trace_t

struct PredictMovement {
        struct EntityDict *groundentity;
        struct EntityDict *touchEnts[MAX_TOUCH];
        struct PredictMovementState pms;
	struct UserCommand cmd;
        struct Vector viewangles;
        struct Vector mins;
	struct Vector maxs;
        float viewheight;
        int numtouch;
        int watertype;
        int waterlevel;
        struct Trace (*trace)(struct Vector *start,
			      struct Vector *mins,
			      struct Vector *maxs,
			      struct Vector *end);
        int (*pointcontents)(struct Vector *point);
        bool snapinitial;
}; // pmove_t
*/

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/game/structs/game.h

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
