#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <md4.h>

#include "util.h"
#include "common.h"
#include "game.h"

#define IDPAKHEADER (('K' << 24) + ('C' << 16) + ('A' << 8) + ('P' << 0))
#define MAX_FILES_IN_PACK 4096
#define PAK0_CHECKSUM ((unsigned) 0x40e614e0)

struct DataPackFile {
        char name[56];
        int filepos;
	int filelen;
}; // dpackfile_t

struct PackHeader {
	int ident;
	int dirofs;
	int dirlen;
}; // dpackheader_t

struct PackFile {
	char name[MAX_QPATH];
	int filepos;
	int filelen;
}; // packfile_t

struct Pack {
	struct PackFile *files;
	char filename[MAX_OSPATH];
	int numfiles;
}; // pack_t

static struct Pack *pack = NULL;

static unsigned FS_BlockChecksum (void *buffer, int length)
{
	int digest[4];
	unsigned val;
	MD4_CTX ctx;

	MD4Init(&ctx);
	MD4Update(&ctx, (unsigned char*) buffer, length);
	MD4Final((unsigned char*) digest, &ctx);

	val = digest[0] ^ digest[1] ^ digest[2] ^ digest[3];

	return val;
}

static void FS_LoadPackFile (char const *packFilename)
{
	FILE *packhandle = fopen(packFilename, "r");
	if (!packhandle) {
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct PackHeader header;
	size_t bytes_read = fread(&header, 1, sizeof(header), packhandle);
	if (sizeof(header) != bytes_read) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: ReadPackHeaderError\n");
		exit(EXIT_FAILURE);
	}

	if (IDPAKHEADER != header.ident) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: BadPackFileError\n");
		exit(EXIT_FAILURE);
	}

	int const numpackfiles = header.dirlen / sizeof(struct DataPackFile);
	fprintf(stdout, "numpackfiles: %d\n", numpackfiles);

	if (numpackfiles > MAX_FILES_IN_PACK) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: NumPackFilesError\n");
		exit(EXIT_FAILURE);
	}

	size_t const bytes = numpackfiles * sizeof(struct PackFile);
	struct PackFile *packfiles = Util_Malloc(bytes);
	if (!packfiles) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: MallocError\n");
		exit(EXIT_FAILURE);
	}

	struct DataPackFile info[MAX_FILES_IN_PACK];
	memset(info, 0, sizeof(info));
	fseek(packhandle, header.dirofs, SEEK_SET);
	bytes_read = fread(info, 1, header.dirlen, packhandle);
	fprintf(stdout, "FS_LoadPackFile: sizeof(DataPackFiles): %zu\n", bytes_read);

	if (numpackfiles * sizeof(*info) != bytes_read) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: ReadDataPackFilesError\n");
		exit(EXIT_FAILURE);
	}

	unsigned checksum = FS_BlockChecksum(info, header.dirlen);
	fprintf(stdout, "FS_LoadPackFile: checksum: 0x%x\n", checksum);

	if (PAK0_CHECKSUM != checksum) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: CheckSumError\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i != numpackfiles; ++i) {
		strncpy(packfiles[i].name, info[i].name, MAX_QPATH);
		packfiles[i].filepos = info[i].filepos;
		packfiles[i].filelen = info[i].filelen;
		fprintf(stdout, "pack-filename: %s\n", packfiles[i].name);
	}

	pack = Util_Malloc(sizeof(struct Pack));
	if (!pack) {
		fclose(packhandle);
		Q_Shutdown();
		fprintf(stderr, "FS_LoadPackFile: MallocError\n");
		exit(EXIT_FAILURE);
	}

	pack->files = packfiles;
	pack->numfiles = numpackfiles;
	strncpy(pack->filename, packFilename, MAX_OSPATH);
	pack->filename[MAX_OSPATH - 1] = 0;
	fclose(packhandle);
}

void FS_FCloseFile (FILE **file)
{
	if (!file || !*file) {
		return;
	}

	fclose(*file);
	*file = NULL;
}

int FS_FOpenFile (char const *filename, FILE **file)
{
	if (!file) {
		Q_Shutdown();
		fprintf(stderr, "FS_FOpenFile: NullFileHandleError\n");
		exit(EXIT_FAILURE);
	}

	if (*file) {
		Q_Shutdown();
		// we expect the actual filehandle to be NULL
		fprintf(stderr, "FS_FOpenFile: FileHandleError\n");
		exit(EXIT_FAILURE);
	}

	if (!pack) {
		Q_Shutdown();
		fprintf(stderr, "FS_FOpenFile: PackFileError\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i != pack->numfiles; ++i) {
		if (!strcmp(pack->files[i].name, filename)) {
			*file = fopen(pack->filename, "r");
			if (!*file) {
				Q_Shutdown();
				fprintf(stderr, "FS_FOpenFile: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

			errno = 0;
			int ret = fseek(*file, pack->files[i].filepos, SEEK_SET);
			if (ret || errno) {
				Q_Shutdown();
				fprintf(stderr, "FS_FOpenFile: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			return pack->files[i].filelen;
		}
	}

	Q_Shutdown();
	// we want to stop here if the file is not found
	fprintf(stderr, "FS_FOpenFile: FileNotFoundError\n");
	exit(EXIT_FAILURE);
	return -1;
}

int FS_FLoadFile (char const *filename, void **buffer)
{
	FILE *file[] = {NULL};
	int const len = FS_FOpenFile(filename, file);
	if (!*file || len == -1) {
		Q_Shutdown();
		fprintf(stderr, "FS_FLoadFile: FileNotFoundError\n");
		exit(EXIT_FAILURE);
	}

	if (!buffer || *buffer) {
		Q_Shutdown();
		// we expect the actual buffer to be NULL
		fprintf(stderr, "FS_FLoadFile: BufferError\n");
		exit(EXIT_FAILURE);
	}

	size_t const size = len;
	*buffer = Util_Malloc(size);
	if (!*buffer) {
		Q_Shutdown();
		fprintf(stderr, "FS_FLoadFile: MallocError\n");
		exit(EXIT_FAILURE);
	}

	size_t const bytes_read = fread(*buffer, 1, size, *file);
	if (bytes_read != size) {
		Q_Shutdown();
		fprintf(stderr, "FS_FLoadFile: FileReadError\n");
		exit(EXIT_FAILURE);
	}

	FS_FCloseFile(file);
	fprintf(stdout,
		"FS_FLoadFile: loaded %zu bytes of %s succesfully\n",
		size,
		filename);
	return len;
}

void FS_Init (void)
{
	FS_LoadPackFile("demo/baseq2/pak0.pak");
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/common/filesystem.c

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
