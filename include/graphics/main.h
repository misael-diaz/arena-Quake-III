#ifndef GUARD_QUAKE_GRAPHICS_REFRESH_H
#define GUARD_QUAKE_GRAPHICS_REFRESH_H

void Refresh_Init(void);
void Refresh_BeginFrame(void);
void Refresh_RenderFrame(struct Refresh *refresh);
void Refresh_EndFrame(void);
void Refresh_Shutdown(void);
void Refresh_Free(void);
void Refresh_Register(void);
void Refresh_NewMap(void);
struct Image *Refresh_FindImage (char const *image_name);
struct Model *Refresh_RegisterModel(char const *model_name);
void Refresh_GammaCorrectAndSetPalette(Byte const *palette);

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/graphics/refresh.h

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
