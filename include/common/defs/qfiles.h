#ifndef GUARD_QUAKE_COMMON_DEFS_QFILES_H
#define GUARD_QUAKE_COMMON_DEFS_QFILES_H

/* MYDEFS */

#define NUM_NODE_CHILDREN 2
#define MAX_CSURFACE_NAME_SIZE 16
#define MAX_MAPSURFACE_RNAME_SIZE 32

/* QUAKE DEFS */

#define ID_BSP_HEADER (('P' << 24) + ('S' << 16) + ('B' << 8) + ('I' << 0))
#define BSP_VERSION 38

#define MAX_MAP_AREAS         256
#define MAX_MAP_AREAPORTALS   1024
#define MAX_MAP_MODELS        1024
#define MAX_MAP_ENTITY_STRING 0x40000
#define MAX_MAP_TEXINFO       8192
#define MAX_MAP_BRUSHES       8192
#define MAX_MAP_LEAFS         65536
#define MAX_MAP_LEAF_BRUSHES  65536
#define MAX_MAP_PLANES        65536
#define MAX_MAP_BRUSHSIDES    65536
#define MAX_MAP_NODES         65536
#define MAX_MAP_VISIBILITY    0x100100

#define LUMP_ENTITIES     0
#define LUMP_PLANES       1
#define LUMP_VERTEXES     2
#define LUMP_VISIBILITY   3
#define LUMP_NODES        4
#define LUMP_TEXINFO      5
#define LUMP_FACES        6
#define LUMP_LIGHTING     7
#define LUMP_LEAFS        8
#define LUMP_LEAFFACES    9
#define LUMP_LEAF_BRUSHES 10
#define LUMP_EDGES        11
#define LUMP_SURFEDGES    12
#define LUMP_MODELS       13
#define LUMP_BRUSHES      14
#define LUMP_BRUSHSIDES   15
#define LUMP_POP          16
#define LUMP_AREAS        17
#define LUMP_AREAPORTALS  18
#define HEADER_LUMPS      19

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/common/defs/qfiles.h

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
