#include <math.h>
#include "common.h"

#define PI ((float) M_PI)
#define DEG2RAD(x) (((x) * PI) / 180.f)

static float GetAngleYaw (const struct Vector *v)
{
	float const yaw = DEG2RAD(v->x);
	return yaw;
}

static float GetAnglePitch (const struct Vector *v)
{
	float const pitch = DEG2RAD(v->y);
	return pitch;
}


static float GetAngleRoll (const struct Vector *v)
{
	// disabling rolling for testing
	float const roll = DEG2RAD(v->z);
	return 0;
}

void VectorCopy (const struct Vector *u, struct Vector *v)
{
	v->x = u->x;
	v->y = u->y;
	v->z = u->z;
}

float DotProduct (struct Vector const *u, struct Vector const *v)
{
	float const x1 = u->x;
	float const y1 = u->y;
	float const z1 = u->z;

	float const x2 = v->x;
	float const y2 = v->y;
	float const z2 = v->z;

	return ((x1 * x2) + (y1 * y2) + (z1 * z2));
}

void VectorNormalize (struct Vector *vec)
{
	float const x = vec->x;
	float const y = vec->y;
	float const z = vec->z;
	float const v = sqrtf((x * x) + (y * y) + (z * z));
	float const v_inv = (1.0f / v);
	vec->x *= v_inv;
	vec->y *= v_inv;
	vec->z *= v_inv;
}

void VectorClear (struct Vector *vec)
{
	vec->x = 0.0f;
	vec->y = 0.0f;
	vec->z = 0.0f;
}

void AngleVectors (struct Vector const *angles,
		   struct Vector *forward,
		   struct Vector *right,
		   struct Vector *up)
{
	float const yaw = GetAngleYaw(angles);
	float const pitch = GetAnglePitch(angles);
	float const cos_y = cosf(yaw);
	float const sin_y = sinf(yaw);
	float const cos_p = cosf(pitch);
	float const sin_p = sinf(pitch);

	forward->x = cos_p * cos_y;
	forward->y = cos_p * sin_y;
	forward->z = -sin_p;

	right->x = sin_y;
	right->y = cos_y;
	right->z = 0.0f;

	up->x = sin_p * cos_y;
	up->y = sin_p * sin_y;
	up->z = cos_p;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/vector/vector.c

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
