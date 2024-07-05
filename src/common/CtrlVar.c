#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "util.h"

static struct CVar *cvars = NULL;

static struct CVar *CVAR_FindCVar (char const *key)
{
	struct CVar *cvar = NULL;
	for (cvar = cvars; cvar; cvar = cvar->next) {
		if (!strcmp(cvar->key, key)) {
			return cvar;
		}
	}
	return cvar;
}

static char *CVAR_CopyString (char const *string)
{
	char *str = Util_CopyString(string);
	if (!str) {
		Q_Shutdown();
		fprintf(stderr, "CVAR_CopyString: CopyStringError\n");
		exit(EXIT_FAILURE);
	}
	return str;
}

static float CVAR_GetData (char const *value)
{
	errno = 0;
	char *endptr[] = {NULL};
	float data = strtof(value, endptr);
	if (**endptr == *value) {
		Q_Shutdown();
		fprintf(stderr, "CVAR_GetData: StringToFloatConversionError\n");
		exit(EXIT_FAILURE);
	}

	if (ERANGE == errno) {
		Q_Shutdown();
		fprintf(stderr, "CVAR_GetData: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return data;
}

struct CVar *CVAR_GetCVar (char const *key, char const *value, int const flags)
{
	struct CVar *cvar = CVAR_FindCVar(key);
	if (cvar) {
		cvar->flags |= flags;
		return cvar;
	}

	cvar = Util_Malloc(sizeof(struct CVar));
	if (!cvar) {
		Q_Shutdown();
		fprintf(stderr, "CVAR_GetCVar: MallocError\n");
		exit(EXIT_FAILURE);
	}

	memset(cvar, 0, sizeof(*cvar));

	cvar->next = cvars;
	cvar->key = CVAR_CopyString(key);
	cvar->value = CVAR_CopyString(value);
	cvar->data = CVAR_GetData(value);
	cvar->flags = flags;
	cvar->modified = true;

	cvars = cvar;

	return cvar;
}

void CVAR_Init (void)
{
	// TODO: register commands
	return;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/common/CtrlVar.c

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
