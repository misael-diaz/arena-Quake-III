void View_RenderView (float stereo_separation)
{
	View_ClearScene();
	Client_AddEntities();
	View_TestEntities();

	cl.refdef.vieworg.x += 0.0625f;
	cl.refdef.vieworg.y += 0.0625f;
	cl.refdef.vieworg.z += 0.0625f;

	// screenViewRectangle == scr_vrect
	cl.refdef.x = screenViewRectangle.x;
	cl.refdef.y = screenViewRectangle.y;
	cl.refdef.width = screenViewRectangle.width;
	cl.refdef.height = screenViewRectangle.height;

	// NOTE: fov is Field Of View
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/client/view.c

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
