#ifndef GUARD_QUAKE_COMMON_FILESYSTEM_H
#define GUARD_QUAKE_COMMON_FILESYSTEM_H

#include <stdio.h>
#include <stddef.h>

size_t FS_FOpenFile(char const *filename, FILE **file);
size_t FS_FLoadFile(char const *filename, void **buffer);
void FS_FCloseFile(FILE **file);
void FS_Init(void);

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/common/filesystem.h

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
