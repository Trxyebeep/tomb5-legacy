#include "../tomb5/pch.h"
#include "lighting.h"
#include "3dmath.h"
#include "d3dmatrix.h"
#include "../game/control.h"
#include "function_stubs.h"
#include "dxshell.h"
#include "polyinsert.h"
#include "output.h"
#include "drawroom.h"
#include "../game/delstuff.h"
#include "../game/camera.h"
#include "../game/effect2.h"
#include "../game/lara.h"
#include "../game/draw.h"

long NumSunLights;
long NumPointLights;
long NumShadowLights;
long NumSpotLights;
long NumFogBulbs;
long TotalNumLights;
long MaxRoomLights;
D3DMATRIX aLightMatrix;
SUNLIGHT_STRUCT SunLights[16];
POINTLIGHT_STRUCT PointLights[64];

ITEM_INFO* current_item;
FVECTOR lGlobalMeshPos;
long aAmbientR;
long aAmbientG;
long aAmbientB;

long bLaraUnderWater;
char bLaraInWater;

long StaticMeshShade;
static ITEM_INFO StaticMeshLightItem;
static long SetupLight_thing;
static long unused_bLaraUnderWater;
static D3DLIGHT_STRUCT* D3DLights;
static D3DLIGHT_STRUCT* D3DDynamics;

void InitObjectLighting(ITEM_INFO* item)
{
	PCLIGHT* light;
	long node_ambient;

	node_ambient = item->il.ambient;
	SetupLight_thing = 0;
	light = (PCLIGHT*)item->il.pCurrentLights;

	for (int i = 0; i < item->il.nCurrentLights; i++)
		if (light[i].Active)
			SuperSetupLight(&light[i], item, &node_ambient);

	light = (PCLIGHT*)item->il.pPrevLights;

	for (int i = 0; i < item->il.nPrevLights; i++)
		if (light[i].Active)
			SuperSetupLight(&light[i], item, &node_ambient);

	InitDynamicLighting(item);

	if (item == lara_item && bLaraInWater)
	{
		if (unused_bLaraUnderWater < 0)
		{
			node_ambient = LaraNodeAmbient[0];
			item->il.fcnt = 0;
			item->il.ambient = node_ambient;
		}
		else
			node_ambient = LaraNodeAmbient[1];
	}

	aAmbientR = CLRR(node_ambient);
	aAmbientG = CLRG(node_ambient);
	aAmbientB = CLRB(node_ambient);

	if (aAmbientR < 16)
		aAmbientR = 16;

	if (aAmbientG < 16)
		aAmbientG = 16;

	if (aAmbientB < 16)
		aAmbientB = 16;
}

void SuperSetupLight(PCLIGHT* light, ITEM_INFO* item, long* ambient)
{
	SUNLIGHT_STRUCT* sun;
	POINTLIGHT_STRUCT* point;
	float x, y, z, num, num2;
	long aR, aG, aB, val, val2;

	if (light->Type == LIGHT_SUN)
	{
		sun = &SunLights[NumSunLights];
		x = (float)light->inx;
		y = (float)light->iny;
		z = (float)light->inz;
		num = 1.0F / (float)(SQUARE(z) + SQUARE(y) + SQUARE(x));
		sun->vec.x = (aLightMatrix._11 * x + aLightMatrix._12 * y + aLightMatrix._13 * z) * num;
		sun->vec.y = (aLightMatrix._21 * x + aLightMatrix._22 * y + aLightMatrix._23 * z) * num;
		sun->vec.z = (aLightMatrix._31 * x + aLightMatrix._32 * y + aLightMatrix._33 * z) * num;
		sun->r = light->r * 255.0F;
		sun->g = light->g * 255.0F;
		sun->b = light->b * 255.0F;
		NumSunLights++;
		TotalNumLights++;
	}
	else if (light->Type == LIGHT_POINT)
	{
		x = light->x - lGlobalMeshPos.x;
		y = light->y - lGlobalMeshPos.y;
		z = light->z - lGlobalMeshPos.z;
		point = &PointLights[NumPointLights];
		num2 = sqrt(SQUARE(z) + SQUARE(y) + SQUARE(x));
		num = 1.0F / num2;
		point->vec.x = (aLightMatrix._11 * x + aLightMatrix._12 * y + aLightMatrix._13 * z) * num;
		point->vec.y = (aLightMatrix._21 * x + aLightMatrix._22 * y + aLightMatrix._23 * z) * num;
		point->vec.z = (aLightMatrix._31 * x + aLightMatrix._32 * y + aLightMatrix._33 * z) * num;
		point->r = light->r * 255.0F;
		point->g = light->g * 255.0F;
		point->b = light->b * 255.0F;
		point->rad = (light->Outer - num2) / light->Outer;

		NumPointLights++;
		TotalNumLights++;
	}
	else if (light->Type == LIGHT_SHADOW)
	{
		aR = CLRR(*ambient);
		aG = CLRG(*ambient);
		aB = CLRB(*ambient);
		val = phd_sqrt(light->Range);
		val2 = light->shadow >> 3;

		if (val >= light->Inner)
			val2 = (long)((val - light->Outer) / ((light->Outer - light->Inner) / -val2));

		val = val2 >> 1;
		aR -= val;
		aG -= val;
		aB -= val;

		if (aR < 0)
			aR = 0;

		if (aG < 0)
			aG = 0;

		if (aB < 0)
			aB = 0;

		*ambient = RGBONLY(aR, aG, aB);
		NumShadowLights++;
	}
}

void CreateLightList(ITEM_INFO* item)
{
	ROOM_INFO* r;
	PCLIGHT* current_lights;
	PCLIGHT* prev_lights;
	PCLIGHT_INFO* room_light;
	D3DVECTOR vec;
	void* bakPtr;
	long bakNum, dx, dy, dz, range;
	bool in_range;

	r = &room[item->room_number];

	if (item->il.room_number != item->room_number)
	{
		bakPtr = item->il.pCurrentLights;
		bakNum = item->il.nCurrentLights;
		item->il.pCurrentLights = item->il.pPrevLights;
		item->il.nCurrentLights = item->il.nPrevLights;
		item->il.pPrevLights = bakPtr;
		item->il.nPrevLights = bakNum;
		item->il.room_number = item->room_number;
		prev_lights = (PCLIGHT*)item->il.pPrevLights;

		for (int i = 0; i < item->il.nPrevLights; i++)
		{
			if (prev_lights[i].Active)
			{
				if (prev_lights[i].Type == LIGHT_SHADOW)
					prev_lights[i].iny = -prev_lights[i].shadow >> 3;
				else
				{
					prev_lights[i].rs = prev_lights[i].r * -0.125F;
					prev_lights[i].gs = prev_lights[i].g * -0.125F;
					prev_lights[i].bs = prev_lights[i].b * -0.125F;
					prev_lights[i].tr = 0.0F;
					prev_lights[i].tg = 0.0F;
					prev_lights[i].tb = 0.0F;
				}

				prev_lights[i].fcnt = 8;
			}
		}

		item->il.nCurrentLights = r->num_lights;
		current_lights = (PCLIGHT*)item->il.pCurrentLights;
		room_light = r->pclight;

		for (int i = 0; i < r->num_lights; i++)
		{
			current_lights->r = room_light->r;
			current_lights->g = room_light->g;
			current_lights->b = room_light->b;
			current_lights->tr = room_light->r;
			current_lights->tg = room_light->g;
			current_lights->tb = room_light->b;
			current_lights->x = room_light->x;
			current_lights->y = room_light->y;
			current_lights->z = room_light->z;
			current_lights->shadow = room_light->shadow << 3;
			current_lights->Inner = room_light->Inner;
			current_lights->Outer = room_light->Outer;
			current_lights->InnerAngle = room_light->InnerAngle;
			current_lights->OuterAngle = room_light->OuterAngle;
			current_lights->Cutoff = room_light->Cutoff;
			current_lights->inx = room_light->inx;
			current_lights->iny = room_light->iny;
			current_lights->inz = room_light->inz;
			current_lights->ix = room_light->ix;
			current_lights->iy = room_light->iy;
			current_lights->iz = room_light->iz;
			current_lights->nx = room_light->nx;
			current_lights->ny = room_light->ny;
			current_lights->nz = room_light->nz;
			current_lights->Type = room_light->Type;
			current_lights->Active = 0;

			if (room_light->Type == LIGHT_SHADOW)
				current_lights->inx = room_light->shadow;

			current_lights++;
			room_light++;
		}
	}

	current_lights = (PCLIGHT*)item->il.pCurrentLights;
	prev_lights = (PCLIGHT*)item->il.pPrevLights;

	for (int i = 0; i < item->il.nCurrentLights; i++)
	{
		in_range = 1;
		dx = current_lights[i].ix - item->il.item_pos.x;
		dy = current_lights[i].iy - item->il.item_pos.y;
		dz = current_lights[i].iz - item->il.item_pos.z;

#if 1
		range = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
#endif

		if (current_lights[i].Type == LIGHT_POINT || current_lights[i].Type == LIGHT_SHADOW)
		{
			range = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

			if (range > SQUARE(current_lights[i].Outer))
				in_range = 0;
		}
		else if (current_lights[i].Type == LIGHT_SPOT)
		{
			range = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

			if (range > SQUARE(current_lights[i].Cutoff))
				in_range = 0;
			else
			{
				vec.x = (float)-dx;
				vec.y = (float)-dy;
				vec.z = (float)-dz;
				D3DNormalise(&vec);

				if (current_lights[i].nx * vec.x + current_lights[i].ny * vec.y + current_lights[i].nz * vec.z <= current_lights[i].Outer)
					in_range = 0;
			}
		}

		current_lights[i].rlp.x = dx;
		current_lights[i].rlp.y = dy;
		current_lights[i].rlp.z = dz;
		current_lights[i].Range = range;

		if (in_range)
		{
			if (!current_lights[i].Active)
			{
				if (current_lights[i].Type == LIGHT_SHADOW)
				{
					current_lights[i].Active = 1;
					current_lights[i].iny = current_lights[i].inx;
					current_lights[i].shadow = 0;
				}
				else
				{
					current_lights[i].rs = current_lights[i].tr * 0.125F;
					current_lights[i].gs = current_lights[i].tg * 0.125F;
					current_lights[i].bs = current_lights[i].tb * 0.125F;
					current_lights[i].r = 0;
					current_lights[i].g = 0;
					current_lights[i].b = 0;
					current_lights[i].Active = 1;
				}

				current_lights[i].fcnt = 8;
			}
		}
		else
		{
			if (current_lights[i].Active && !current_lights[i].fcnt)
			{
				if (current_lights[i].Type == LIGHT_SHADOW)
					current_lights[i].iny = -current_lights[i].shadow >> 3;
				else
				{
					current_lights[i].rs = current_lights[i].r * -0.125F;
					current_lights[i].gs = current_lights[i].g * -0.125F;
					current_lights[i].bs = current_lights[i].b * -0.125F;
				}

				current_lights[i].fcnt = 8;
			}
		}
	}

	FadeLightList(current_lights, item->il.nCurrentLights);
	FadeLightList(prev_lights, item->il.nPrevLights);
}

void FadeLightList(PCLIGHT* lights, long nLights)
{
	for (int i = 0; i < nLights; i++)
	{
		if (lights[i].Active && lights[i].fcnt)
		{
			if (lights[i].Type == LIGHT_SHADOW)
				lights[i].shadow += lights[i].iny;
			else
			{
				lights[i].r += lights[i].rs;
				lights[i].g += lights[i].gs;
				lights[i].b += lights[i].bs;
			}

			lights[i].fcnt--;

			if (lights[i].Type == LIGHT_SHADOW)
			{
				if (lights[i].shadow <= 0)
					lights[i].Active = 0;
			}
			else
			{
				if (lights[i].r <= 0 && lights[i].g <= 0 && lights[i].b <= 0)
					lights[i].Active = 0;
			}
		}
	}
}

void SuperResetLights()
{
	D3DMATRIX view;
	D3DMATRIX cam;

	view = D3DMView;
	cam = D3DInvCameraMatrix;
	NumPointLights = 0;
	NumSunLights = 0;
	NumSpotLights = 0;
	NumShadowLights = 0;
	TotalNumLights = 0;
	NumFogBulbs = 0;
	aAmbientR = 0;
	aAmbientG = 0;
	aAmbientB = 0;
	D3DMultMatrix(&aLightMatrix, &view, &cam);
	lGlobalMeshPos.x = aCamera.pos.x + aLightMatrix._41;
	lGlobalMeshPos.y = aCamera.pos.y + aLightMatrix._42;
	lGlobalMeshPos.z = aCamera.pos.z + aLightMatrix._43;
}

void CalcAmbientLight(ITEM_INFO* item)
{
	ROOM_INFO* r;
	short room_number;

	room_number = item->room_number;
	GetFloor(item->il.item_pos.x, item->il.item_pos.y, item->il.item_pos.z, &room_number);
	r = &room[room_number];

	if (item->il.ambient != r->ambient)
	{
		if (item->il.fcnt == -1)
		{
			item->il.ambient = r->ambient;
			item->il.fcnt = 0;
		}
		else
		{
			if (!item->il.fcnt)
			{
				item->il.r = CLRR(item->il.ambient);
				item->il.g = CLRG(item->il.ambient);
				item->il.b = CLRB(item->il.ambient);
				item->il.rs = CLRR(r->ambient) - item->il.r;
				item->il.gs = CLRG(r->ambient) - item->il.g;
				item->il.bs = CLRB(r->ambient) - item->il.b;
				item->il.r <<= 3;
				item->il.g <<= 3;
				item->il.b <<= 3;
				item->il.fcnt = 8;
			}

			if (item->il.fcnt)
			{
				item->il.r += item->il.rs;
				item->il.g += item->il.gs;
				item->il.b += item->il.bs;
				item->il.ambient = RGBONLY(item->il.r >> 3, item->il.g >> 3, item->il.b >> 3);
				item->il.fcnt--;
			}
		}
	}
}

void S_CalculateStaticMeshLight(long x, long y, long z, long shade, ROOM_INFO* r)
{
	StaticMeshLightItem.il.ambient = r->ambient;
	StaticMeshLightItem.il.item_pos.x = x;
	StaticMeshLightItem.il.item_pos.y = y;
	StaticMeshLightItem.il.item_pos.z = z;
	StaticMeshLightItem.room_number = (short)CurrentRoom;
	StaticMeshShade = shade;
	current_item = &StaticMeshLightItem;
}

void SuperSetupDynamicLight(DYNAMIC* light, ITEM_INFO* item)
{
	POINTLIGHT_STRUCT* point;
	float dx, dy, dz, falloff, dist, val;

	dx = light->x - lGlobalMeshPos.x;
	dy = light->y - lGlobalMeshPos.y;
	dz = light->z - lGlobalMeshPos.z;
	falloff = (float)(light->falloff - (light->falloff >> 3));
	dist = sqrt(SQUARE(dz) + SQUARE(dy) + SQUARE(dx));
	point = &PointLights[NumPointLights];

	if (dist <= falloff)
	{
		val = 1.0F / dist;
		point->vec.x = val * (dx * aLightMatrix._11 + dy * aLightMatrix._12 + dz * aLightMatrix._13);
		point->vec.y = val * (dx * aLightMatrix._21 + dy * aLightMatrix._22 + dz * aLightMatrix._23);
		point->vec.z = val * (dx * aLightMatrix._31 + dy * aLightMatrix._32 + dz * aLightMatrix._33);
		point->r = light->r;
		point->g = light->g;
		point->b = light->b;
		point->rad = (falloff - dist) / falloff;
		NumPointLights++;
		TotalNumLights++;
	}
}

void InitDynamicLighting_noparams()
{

}

void InitDynamicLighting(ITEM_INFO* item)
{
	DYNAMIC* light;

	for (int i = 0; i < 32; i++)
	{
		light = &dynamics[i];

		if (light->on)
			SuperSetupDynamicLight(light, item);
	}

	aAmbientR = CLRR(item->il.ambient);
	aAmbientG = CLRG(item->il.ambient);
	aAmbientB = CLRB(item->il.ambient);
}

void ClearObjectLighting()
{

}

void ClearDynamicLighting()
{

}

void ApplyMatrix(long* matrix, PHD_VECTOR* start, PHD_VECTOR* dest)
{
	dest->x = (start->x * matrix[M00] + start->y * matrix[M01] + start->z * matrix[M02]) >> 14;
	dest->y = (start->x * matrix[M10] + start->y * matrix[M11] + start->z * matrix[M12]) >> 14;
	dest->z = (start->x * matrix[M20] + start->y * matrix[M21] + start->z * matrix[M22]) >> 14;
}

void ApplyTransposeMatrix(long* matrix, PHD_VECTOR* start, PHD_VECTOR* dest)
{
	dest->x = (start->x * matrix[M00] + start->y * matrix[M10] + start->z * matrix[M20]) >> 14;
	dest->y = (start->x * matrix[M01] + start->y * matrix[M11] + start->z * matrix[M21]) >> 14;
	dest->z = (start->x * matrix[M02] + start->y * matrix[M12] + start->z * matrix[M22]) >> 14;
}

void CreateD3DLights()
{

}

void FreeD3DLights()
{

}

void MallocD3DLights()
{
	if (MaxRoomLights > 21)
		Log(1, "MAX Room Lights of %d Exceeded - %d", 21, MaxRoomLights);

	MaxRoomLights *= 2;
	D3DLights = (D3DLIGHT_STRUCT*)game_malloc(sizeof(D3DLIGHT_STRUCT) * MaxRoomLights, 0);
	D3DDynamics = (D3DLIGHT_STRUCT*)game_malloc(sizeof(D3DLIGHT_STRUCT) * 32, 0);
}

void InitObjectFogBulbs()
{
	FOGBULB_STRUCT* fogbulb;
	FOGBULB_STRUCT* objbulb;
	FVECTOR box;
	FVECTOR fPos;
	float* bbox;
	float sqr, val;
	long goin;

	NumFogBulbs = 0;
	objbulb = FogBulbs;

	for (int i = 0; i < NumActiveFogBulbs; i++)
	{
		fogbulb = &ActiveFogBulbs[i];

		if (!fogbulb->visible)
			continue;
		
		goin = 1;

		if (fogbulb->sqlen >= fogbulb->sqrad)
		{
			goin = 0;
			bbox = aBoundingBox;

			for (int j = 0; j < 8; j++)
			{
				box.x = *bbox++;
				box.y = *bbox++;
				box.z = *bbox++;
				fPos.x = fogbulb->world.x - box.x;
				fPos.y = fogbulb->world.y - box.y;
				fPos.z = fogbulb->world.z - box.z;

				if (SQUARE(fPos.x) + SQUARE(fPos.y) + SQUARE(fPos.y) < fogbulb->sqrad)
				{
					goin = 1;
					break;
				}

				sqr = SQUARE(box.x) + SQUARE(box.y) + SQUARE(box.z);
				val = 1.0F / sqrt(sqr);
				fPos.x = box.x * val;
				fPos.y = box.y * val;
				fPos.z = box.z * val;
				val = fogbulb->pos.x * fPos.x + fogbulb->pos.y * fPos.y + fogbulb->pos.z * fPos.z;

				if (val > 0)
				{
					val = SQUARE(val);

					if (sqr > val && fogbulb->sqlen - val < fogbulb->sqrad)
					{
						goin = 1;
						break;
					}
				}
			}
		}

		if (goin)
		{
			objbulb->pos.x = fogbulb->pos.x;
			objbulb->pos.y = fogbulb->pos.y;
			objbulb->pos.z = fogbulb->pos.z;
			objbulb->sqlen = fogbulb->sqlen;
			objbulb->rad = fogbulb->rad;
			objbulb->sqrad = fogbulb->sqrad;
			objbulb->r = fogbulb->r;
			objbulb->g = fogbulb->g;
			objbulb->b = fogbulb->b;
			objbulb->d = fogbulb->d;
			objbulb++;
			NumFogBulbs++;
		}
	}
}

void SetupLight(D3DLIGHT_STRUCT* d3dlight, PCLIGHT* light, long* ambient)
{
	PHD_VECTOR d;
	PHD_VECTOR vec;
	PHD_VECTOR l;
	float fVal;
	long r, g, b, val, val2;;

	d.x = light->rlp.x;
	d.y = light->rlp.y;
	d.z = light->rlp.z;
	d3dlight->D3DLight2.dcvColor.r = light->r;
	d3dlight->D3DLight2.dcvColor.g = light->g;
	d3dlight->D3DLight2.dcvColor.b = light->b;
	ApplyMatrix(w2v_matrix, &d, &vec);
	ApplyTransposeMatrix(phd_mxptr, &vec, &d);
	d3dlight->D3DLight2.dvPosition.x = (float)d.x;
	d3dlight->D3DLight2.dvPosition.y = (float)d.y;
	d3dlight->D3DLight2.dvPosition.z = (float)d.z;

	if (light->Type == LIGHT_SUN || light->Type == LIGHT_SPOT)
	{
		d.x = light->inx;
		d.y = light->iny;
		d.z = light->inz;
		ApplyMatrix(w2v_matrix, &d, &vec);
		ApplyTransposeMatrix(phd_mxptr, &vec, &d);
		l.x = d.x;
		l.y = d.y;
		l.z = d.z;
	}

	d3dlight->D3DLight2.dwFlags = D3DLIGHT_ALL;

	switch (light->Type)
	{
	case LIGHT_SUN:
		d3dlight->D3DLight2.dltType = D3DLIGHT_DIRECTIONAL;
		d3dlight->D3DLight2.dvDirection.x = (float)l.x;
		d3dlight->D3DLight2.dvDirection.y = (float)l.y;
		d3dlight->D3DLight2.dvDirection.z = (float)l.z;
		break;

	case LIGHT_POINT:
		d3dlight->D3DLight2.dltType = D3DLIGHT_POINT;
		d3dlight->D3DLight2.dvAttenuation1 = 2;
		d3dlight->D3DLight2.dvRange = light->Outer;

		if (SetupLight_thing)
		{
			fVal = (light->Outer - phd_sqrt(light->Range)) / light->Outer;

			if (fVal < 1)
			{
				r = long(CLRR(*ambient) + (fVal * light->r * 255));
				g = long(CLRG(*ambient) + (fVal * light->g * 255));
				b = long(CLRB(*ambient) + (fVal * light->b * 255));

				if (r > 255)
					r = 255;

				if (g > 255)
					g = 255;

				if (b > 255)
					b = 255;

				*ambient = RGBONLY(r, g, b);
			}
		}

		break;

	case LIGHT_SPOT:
		d3dlight->D3DLight2.dltType = D3DLIGHT_SPOT;
		d3dlight->D3DLight2.dvDirection.x = (float)l.x;
		d3dlight->D3DLight2.dvDirection.y = (float)l.y;
		d3dlight->D3DLight2.dvDirection.z = (float)l.z;
		d3dlight->D3DLight2.dvFalloff = 1;

		if (SetupLight_thing)
			d3dlight->D3DLight2.dvAttenuation1 = 2;
		else
			d3dlight->D3DLight2.dvAttenuation1 = 1;

		d3dlight->D3DLight2.dvRange = light->Cutoff;
		d3dlight->D3DLight2.dvTheta = light->OuterAngle;
		d3dlight->D3DLight2.dvPhi = light->OuterAngle;
		break;

	case LIGHT_SHADOW:
		r = CLRR(*ambient);
		g = CLRG(*ambient);
		b = CLRB(*ambient);
		val = phd_sqrt(light->Range);
		val2 = light->shadow >> 3;

		if (val >= light->Inner)
			val2 = long((val - light->Outer) / ((light->Outer - light->Inner) / -val2));

		if (val2 < 0)
			val2 = 0;

		r -= val2;
		g -= val2;
		b -= val2;

		if (r < 0)
			r = 0;

		if (g < 0)
			g = 0;

		if (b < 0)
			b = 0;

		*ambient = RGBONLY(r, g, b);
		d3dlight->D3DLight2.dwFlags = D3DLIGHT_NO_SPECULAR;
		d3dlight->D3DLight2.dltType = D3DLIGHT_POINT;
		break;
	}

	DXAttempt(d3dlight->D3DLight->SetLight((LPD3DLIGHT)&d3dlight->D3DLight2));
}

#pragma warning(push)
#pragma warning(disable : 4244)
void ShowOmni(long x, long y, long z, long rad)
{
	PHD_VECTOR pos;
	FVECTOR p;
	D3DTLVERTEX v[64];
	float ang, zv, step;

	x -= camera.pos.x;
	y -= camera.pos.y;
	z -= camera.pos.z;

	phd_PushMatrix();
	phd_TranslateAbs(camera.pos.x, camera.pos.y, camera.pos.z);

	step = float(2 * M_PI) / float(64 - 1);	//2pi / (nVtx - 1)
	ang = 0;

	for (int i = 0; i < 64; i++)
	{
		p.x = sin(ang) * rad + x;
		p.y = y;
		p.z = cos(ang) * rad + z;
		pos.x = p.x * phd_mxptr[M00] + p.y * phd_mxptr[M01] + p.z * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = p.x * phd_mxptr[M10] + p.y * phd_mxptr[M11] + p.z * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = p.x * phd_mxptr[M20] + p.y * phd_mxptr[M21] + p.z * phd_mxptr[M22] + phd_mxptr[M23];
		zv = (float)phd_persp / (float)pos.z;
		v[i].sx = pos.x * zv + f_centerx;
		v[i].sy = pos.y * zv + f_centery;
		v[i].sz = pos.z;
		v[i].color = 0xFF00FF00;
		v[i].specular = 0xFF000000;
		ang += step;
	}

	S_DrawLine(64, v);

	for (int i = 0; i < 64; i++)
	{
		p.x = cos(ang) * rad + x;
		p.y = sin(ang) * rad + y;
		p.z = z;
		pos.x = p.x * phd_mxptr[M00] + p.y * phd_mxptr[M01] + p.z * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = p.x * phd_mxptr[M10] + p.y * phd_mxptr[M11] + p.z * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = p.x * phd_mxptr[M20] + p.y * phd_mxptr[M21] + p.z * phd_mxptr[M22] + phd_mxptr[M23];
		zv = (float)phd_persp / (float)pos.z;
		v[i].sx = pos.x * zv + f_centerx;
		v[i].sy = pos.y * zv + f_centery;
		v[i].sz = pos.z;
		v[i].color = 0xFFFF0000;
		v[i].specular = 0xFF000000;
		ang += step;
	}

	S_DrawLine(64, v);

	for (int i = 0; i < 64; i++)
	{
		p.x = x;
		p.y = sin(ang) * rad + y;
		p.z = cos(ang) * rad + z;
		pos.x = p.x * phd_mxptr[M00] + p.y * phd_mxptr[M01] + p.z * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = p.x * phd_mxptr[M10] + p.y * phd_mxptr[M11] + p.z * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = p.x * phd_mxptr[M20] + p.y * phd_mxptr[M21] + p.z * phd_mxptr[M22] + phd_mxptr[M23];
		zv = (float)phd_persp / (float)pos.z;
		v[i].sx = pos.x * zv + f_centerx;
		v[i].sy = pos.y * zv + f_centery;
		v[i].sz = pos.z;
		v[i].color = 0xFFFF0000;
		v[i].specular = 0xFF000000;
		ang += step;
	}

	S_DrawLine(64, v);

	phd_PopMatrix();
}
#pragma warning(pop)
