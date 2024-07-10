#ifndef GUARD_QUAKE_COMMON_STRUCTS_CMODEL_H
#define GUARD_QUAKE_COMMON_STRUCTS_CMODEL_H

#include "../../common/defs/qfiles.h"

struct CSurface {
	char name[MAX_CSURFACE_NAME_SIZE];
	int flags;
	int value;
}; // csurface_t

struct MapSurface {
        struct CSurface csurface;
        char rname[MAX_MAPSURFACE_RNAME_SIZE];
}; // mapsurface_t

struct CLeaf {
        int contents;
        int cluster;
        int area;
        unsigned short firstleafbrush;
        unsigned short numleafbrushes;
}; // cleaf_t

struct CPlane {
        struct Vector normal;
        float dist;
        Byte type;
        Byte signbits;
        Byte pad[2];
}; // cplane_t

struct CBrush {
        int contents;
        int numsides;
        int firstbrushside;
        int checkcount;
}; // cbrush_t

struct CBrushSide {
        struct CPlane *plane;
        struct MapSurface *surface;
}; // cbrushside_t

struct CNode {
        struct CPlane *plane;
        int children[2];
}; // cnode_t

struct CArea {
        int numAreaPortals;
        int firstAreaPortal;
        int floodvalid;
        int floodnum;
}; // carea_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/common/structs/cmodel.h

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
