#ifndef GUARD_QUAKE_COMMON_STRUCTS_QFILES_H
#define GUARD_QUAKE_COMMON_STRUCTS_QFILES_H

struct Lump {
	int fileofs;
	int filelen;
}; // lump_t

struct DataHeader {
	int ident;
	int version;
	struct Lump lumps[HEADER_LUMPS];
}; // dheader_t

struct DataLeaf {
	int contents;
	short cluster;
	short area;
	short mins[3];
	short maxs[3];
	unsigned short firstleafface;
	unsigned short numleaffaces;
	unsigned short firstleafbrush;
	unsigned short numleafbrushes;
}; // dleaf_t

struct DataPlane {
        struct Vector normal;
        float dist;
        int type;
}; // dplane_t

struct InfoTexture {
        float vecs[2][4];
        int flags;
        int value;
        char name[32];
        int nextInfoTexture;
}; // texinfo_t

struct DataBrush {
        int firstside;
        int numsides;
        int contents;
}; // dbrush_t

struct DataBrushSide {
	unsigned short planenum;
	short texinfo;
}; // dbrushside_t

struct DataModel {
	struct Vector mins;
	struct Vector maxs;
	struct Vector origin;
	int headnode;
	int firstface;
	int numfaces;
}; // dmodel_t

struct DataNode {
        int planenum;
        int children[2];
        short mins[3];
        short maxs[3];
        unsigned short firstface;
        unsigned short numfaces;
}; // dnode_t

struct DataArea {
	int numAreaPortals;
	int firstAreaPortal;
}; // darea_t

struct DataAreaPortal {
        int portalnum;
        int otherarea;
}; // dareaportal_t

struct DataVisibility {
        int numclusters;
        int bitofs[8][2];
}; // dvis_t

#endif

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: include/common/structs/qfiles.h

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
