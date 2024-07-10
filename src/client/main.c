#include "system.h"
#include "common.h"
#include "client.h"

struct ClientStatic cls;
struct Client client;
struct CVar *cl_add_lights = NULL;
struct CVar *cl_add_entities = NULL;
struct CVar *cl_add_particles = NULL;
struct CVar *cl_add_blend = NULL;
struct CVar *cl_predict = NULL;
struct CVar *cl_paused = NULL;

void Client_AddTemporaryEntities (void)
{
	Client_AddTemporaryEntities();
}

void Client_AddParticles (void)
{
	Client_AddParticles();
}

void Client_AddDLights (void)
{
	Client_AddDLights();
}

void Client_AddLightStyles (void)
{
	Client_AddLightStyles();
}

void Client_AddPacketEntities (struct Frame *frame)
{
	Entity_AddPacketEntities(frame);
}

void Client_AddEntities (void)
{
	Entity_AddEntities();
}

void Client_CalculateViewValues (void)
{
	View_CalculateViewValues();
}

void Client_InitLocal (void)
{
	cls.state = CA_DISCONNECTED;
	cls.realtime = Sys_ClockMilliSeconds();

	cl_add_blend = CVAR_GetCVar("cl_add_blend", "1", 0);
	cl_add_entities = CVAR_GetCVar("cl_add_entities", "1", 0);
	cl_add_particles = CVAR_GetCVar("cl_add_particles", "1", 0);
	cl_add_lights = CVAR_GetCVar("cl_add_lights", "1", 0);
	cl_predict = CVAR_GetCVar("cl_predict", "1", 0);
	cl_paused = CVAR_GetCVar("cl_paused", "0", 0);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/main.c

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
