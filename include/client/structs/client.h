#ifndef GUARD_QUAKE_CLIENT_STRUCTS_CLIENT_H
#define GUARD_QUAKE_CLIENT_STRUCTS_CLIENT_H

#include <stdio.h>

#include "../defs/client.h"
#include "../../graphics/model/structs/model.h"
#include "../../graphics/local/structs/local.h"
#include "../../game/structs/game.h"
#include "../../common/defs/qcommon.h"

enum ConnectionState {
	CA_UNINITIALIZED,
	CA_DISCONNECTED,        
	CA_CONNECTING,          
	CA_CONNECTED,           
	CA_ACTIVE                       
}; // connstate_t

/* SFX: Special Effects */

struct CacheSFX {
        int length;
        int loopstart;
        int speed;
        int width;
        int stereo;
        Byte data[1];
}; // sfxcache_t

struct SFX {
        struct CacheSFX *cache;
        char *truename;
        char name[MAX_QPATH];
        int registration_sequence;
}; // sfx_t;

struct ClientInfo {
        struct Model *model;
        struct Model *weaponmodel[MAX_CLIENT_WEAPON_MODELS];
        struct Image *skin;
        struct Image *icon;
        char name[MAX_QPATH];
        char cinfo[MAX_QPATH];
        char iconname[MAX_QPATH];
}; // clientinfo_t

struct Frame {
        struct PlayerState playerstate;
        Byte areabits[MAX_MAP_AREAS / 8];
        int serverframe;
        int servertime;
        int deltaframe;
        int num_entities;
        int parse_entities;
        bool valid;
}; // frame_t

struct ClientStatic {
	long realtime;
	enum ConnectionState state;
};

struct Client {
	struct ClientInfo clientinfo[MAX_CLIENTS];
	struct Frame frames[UPDATE_BACKUP];
	struct UserCommand cmds[CMD_BACKUP];
	struct UserCommand cmd;
	struct ClientInfo baseclientinfo;
	struct Vector predicted_origin;
	struct Vector predicted_angles;
	struct Vector prediction_error;
	struct Vector viewangles;
	struct Vector forward;
	struct Vector right;
	struct Vector up;
	struct Refresh refresh;
	struct Frame frame;
	struct Model *model_draw[MAX_MODELS];
	struct CModel *model_clip[MAX_MODELS];
	struct SFX *sound_precache[MAX_SOUNDS];
	struct Image *image_precache[MAX_IMAGES];
	FILE *cinematic_file;
	char gamedir[MAX_QPATH];
	char configstrings[MAX_CONFIG_STRINGS][MAX_QPATH];
	char cinematicpalette[768];
	char layout[1024];
	int inventory[MAX_ITEMS];
	int cmd_time[CMD_BACKUP];
	short predicted_origins[CMD_BACKUP][3];
	float lerpfrac;
	float predicted_step;
	unsigned int predicted_step_time;
	int servercount;
	int playernum;
	int timeoutcount;
	int timedemo_frames;
	int timedemo_start;
	int parse_entities;
	int surpressCount;
	int cinematictime;
	int cinematicframe;
	int time;
	bool cinematicpalette_active;
	bool attractloop;
	bool refresh_prepped;
	bool sound_prepped;
	bool force_refresh;
}; // client_state_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/structs/client.h

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
