#include "common.h"
#include "video.h"
#include "graphics.h"
#include "client.h"

extern struct ClientStatic cls;
extern struct Client client;

void Entity_AddTemporaryEntities (void)
{
	return;
}

void Entity_AddParticles (void)
{
	return;
}

void Entity_AddDLights (void)
{
	return;
}

void Entity_AddLightStyles (void)
{
	return;
}

void Entity_AddPacketEntities (struct Frame *frame)
{
	// mostly effects bailing out on this one for now
	return;
}

void Entity_AddEntities (void)
{
	if (cls.state != CA_ACTIVE) {
		return;
	}

	// NOTE: there's no client-server lag so most likely this is what we should use
	client.lerpfrac = 1.0f;

	Client_CalculateViewValues();

	Client_AddPacketEntities(&client.frame);
	Client_AddTemporaryEntities();
	Client_AddParticles();
	Client_AddDLights();
	Client_AddLightStyles();
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/entities.c

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
