#include <stdio.h>

#include "types/Vector.h"
#include "graphics/model.h"
#include "vector.h"

struct OldReferenceDefinition oldRefDef;

void Graphics_SetupFrame (void)
{
	struct ViewRectangle viewRectangle;
	Vector3DCopy(&oldRefDef.vieworg, &modelorg);
	Vector3DCopy(&oldRefDef.vieworg, &origin);
}

void Graphics_RenderFrame (struct ReferenceDefinition *RD)
{
	struct ReferenceDefinition newRD = *RD;
	Vector3DCopy(&RD->vieworg, &newRD.vieworg);
	Vector3DCopy(&RD->viewangles, &newRD.viewangles);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/main/main.c

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
