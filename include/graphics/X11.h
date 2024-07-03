#ifndef GUARD_QUAKE_GRAPHICS_X11_IMPLEMENTATION_H
#define GUARD_QUAKE_GRAPHICS_X11_IMPLEMENTATION_H

#include <stdbool.h>
#include "../common/types/Byte.h"
#include "local/enums/local.h"

void Graphics_ImpInit(void);
void Graphics_ImpBeginFrame(void);
void Graphics_ImpEndFrame(void);
void Graphics_ImpSetPalette(Byte const *palette);
unsigned int Graphics_RGB(unsigned long r, unsigned long g, unsigned long b);
enum GraphicsErrorType Graphics_ImpSetMode(int *W, int *H, int mode, bool fullscreen);
void Graphics_ImpShutdown(void);
void Graphics_ImpFree(void);

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/graphics/X11.h

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
