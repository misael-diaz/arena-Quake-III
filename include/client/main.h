#ifndef GUARD_QUAKE_CLIENT_MAIN_H
#define GUARD_QUAKE_CLIENT_MAIN_H

void Client_AddTemporaryEntities(void);
void Client_AddParticles(void);
void Client_AddDLights(void);
void Client_AddLightStyles(void);
void Client_AddPacketEntities(struct Frame *frame);
void Client_CalculateViewValues(void);
void Client_AddEntities(void);

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/main.h

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
