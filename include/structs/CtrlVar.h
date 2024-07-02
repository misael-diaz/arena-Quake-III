#ifndef GUARD_QUAKE_STRUCTS_CONTROL_VARIABLES_H
#define GUARD_QUAKE_STRUCTS_CONTROL_VARIABLES_H

#include <stdbool.h>

struct CVar {
	struct CVar *next;
	char *key;
	char *value;
	float data;
	int flags;
	bool modified;
	char pad[3];
	int: 32;
	long: 64;
	long: 64;
	long: 64;
}; // cvar_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/structs/CtrlVar.h

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
