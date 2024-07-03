#include <stdio.h>
#include <string.h>

#include "graphics.h"
#include "game.h"

int mod_registration_sequence = 0;
Byte mod_novis[MODEL_NOVIS_SIZE];

void MOD_Init (void)
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
