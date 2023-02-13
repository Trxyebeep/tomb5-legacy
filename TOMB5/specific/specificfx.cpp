#include "../tomb5/pch.h"
#include "specificfx.h"
#include "3dmath.h"
#include "../game/delstuff.h"
#include "../game/control.h"
#include "../specific/function_table.h"
#include "../game/objects.h"
#include "polyinsert.h"
#include "drawroom.h"
#include "d3dmatrix.h"
#include "function_stubs.h"
#include "../game/tomb4fx.h"
#include "winmain.h"
#include "profiler.h"
#include "alexstuff.h"
#include "../game/sphere.h"
#include "../game/lasers.h"
#include "../game/rope.h"
#include "../game/gameflow.h"
#include "output.h"
#include "file.h"
#include "texture.h"
#include "../game/camera.h"
#include "dxshell.h"
#include "../game/lara.h"
#include "../game/effects.h"
#include "../game/effect2.h"
#include "gamemain.h"
#include "../game/draw.h"

#define LINE_POINTS	4	//number of points in each grid line
#define POINT_HEIGHT_CORRECTION	196	//if the difference between the floor below Lara and the floor height below the point is greater than this value, point height is corrected to lara's floor level.
#define NUM_TRIS	14	//number of triangles needed to create the shadow (this depends on what shape you're doing)
#define GRID_POINTS	(LINE_POINTS * LINE_POINTS)	//number of points in the whole grid

long ShadowTable[NUM_TRIS * 3] =	//num of triangles * 3 points
{
	//shadow is split in 3 parts. top, middle, bottom, each part made of triangles. first 4 tris are the top part,
	//the following 6 are the middle part, and the last 4 are the bottom part.

	//tris for each part go left to right. i.e the first line for the top part is the leftmost tri, 4th one is the rightmost, and so on
	//but this isn't a hard rule at all, it's just how Core did it
/*
	the default shadow grid is 4 x 4 points
	0	1	2	3

	4	5	6	7

	8	9	10	11

	12	13	14	15

	the values here are which grid points the tri points are at.
	for example, the first tri, 4, 1, 5. connect the dots. 4 -> 1 -> 5
	which makes the top left tri.
	and so on.
*/
4, 1, 5,
5, 1, 6,	//top part
6, 1, 2,
6, 2, 7,
//
8, 4, 9,
9, 4, 5,
9, 5, 10,	//middle part
10, 5, 6,
10, 6, 11,
11, 6, 7,
//
13, 8, 9,
13, 9, 14,	//bottom part
14, 9, 10,
14, 10, 11
};

uchar TargetGraphColTab[48] =
{
	0, 0, 255,
	0, 0, 255,
	255, 255, 0,
	255, 255, 0,
	0, 0, 255,
	0, 0, 255,
	255, 255, 0,
	255, 255, 0,
	0, 0, 255,
	0, 0, 255,
	0, 0, 255,
	0, 0, 255,
	255, 255, 0,
	255, 255, 0,
	255, 255, 0,
	255, 255, 0
};

float SnowSizes[32]
{
	-24.0F, -24.0F, -24.0F, 24.0F, 24.0F, -24.0F, 24.0F, 24.0F, -12.0F, -12.0F, -12.0F, 12.0F, 12.0F, -12.0F, 12.0F, 12.0F,
	-8.0F, -8.0F, -8.0F, 8.0F, 8.0F, -8.0F, 8.0F, 8.0F, -6.0F, -6.0F, -6.0F, 6.0F, 6.0F, -6.0F, 6.0F, 6.0F
};

char flare_table[121] =
{
	//	r, g, b, size, XY?, sprite
	96, 80, 0, 6, 0, 31,
	48, 32, 32, 10, -6, 31,
	32, 24, 24, 18, -1, 31,
	80, 104, 64, 5, -3, 30,
	64, 64, 64, 20, 0, 32,
	96, 56, 56, 14, 0, 11,
	80, 40, 32, 9, 0, 29,
	16, 24, 40, 2, 5, 31,
	8, 8, 24, 7, 8, 31,
	8, 16, 32, 4, 10, 31,
	48, 24, 0, 2, 13, 31,
	40, 96, 72, 1, 16, 11,
	40, 96, 72, 3, 20, 11,
	32, 16, 0, 6, 22, 31,
	32, 16, 0, 9, 23, 30,
	32, 16, 0, 3, 24, 31,
	32, 48, 24, 4, 26, 31,
	8, 40, 112, 3, 27, 11,
	8, 16, 0, 10, 29, 30,
	16, 16, 24, 17, 31, 29,
	-1
};

static NODEOFFSET_INFO NodeOffsets[16] =
{
	{ -16, 40, 160, -14, 0 },
	{ -16, -8, 160, 0, 0 },
	{ 0, 0, 256, 8, 0 },
	{ 0, 0, 256, 17, 0 },
	{ 0, 0, 256, 26, 0 },
	{ 0, 144, 40, 10, 0 },
	{ -40, 64, 360, 14, 0 },
	{ 0, -600, -40, 0, 0 },
	{ 0, 32, 16, 9, 0 },

	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};

uchar SplashLinks[347]
{
	16, 18, 0, 2,
	18, 20, 2, 4,
	20, 22, 4, 6,
	22, 24, 6, 8,
	24, 26, 8, 10,
	26, 28, 10, 12,
	28, 30, 12, 14,
	30, 16, 14, 0,
	//links
	84, 111, 109, 98, 32, 82, 97, 105, 100, 101, 114, 32, 73, 86, 32, 45, 32, 84, 104, 101, 32, 76, 97, 115,
	116, 32, 82, 101, 118, 101, 108, 97, 116, 105, 111, 110, 32, 32, 45, 45, 32, 68, 101, 100, 105, 99, 97, 116,
	101, 100, 32, 116, 111, 32, 109, 121, 32, 102, 105, 97, 110, 99, 101, 32, 74, 97, 121, 32, 102, 111, 114, 32,
	112, 117, 116, 116, 105, 110, 103, 32, 117, 112, 32, 119, 105, 116, 104, 32, 116, 104, 105, 115, 32, 103, 97, 109,
	101, 32, 116, 97, 107, 105, 110, 103, 32, 111, 118, 101, 114, 32, 111, 117, 114, 32, 108, 105, 102, 101, 115, 44,
	109, 121, 32, 115, 116, 101, 112, 32, 115, 111, 110, 115, 32, 67, 114, 97, 105, 103, 44, 74, 97, 109, 105, 101,
	32, 38, 32, 65, 105, 100, 101, 110, 32, 40, 83, 104, 111, 119, 32, 116, 104, 105, 115, 32, 116, 111, 32, 121,
	111, 117, 114, 32, 109, 97, 116, 101, 115, 32, 97, 116, 32, 115, 99, 104, 111, 111, 108, 44, 32, 116, 104, 101,
	121, 39, 108, 108, 32, 98, 101, 108, 105, 101, 118, 101, 32, 121, 111, 117, 32, 110, 111, 119, 33, 33, 41, 44,
	97, 108, 115, 111, 32, 102, 111, 114, 32, 109, 121, 32, 100, 97, 117, 103, 104, 116, 101, 114, 115, 32, 83, 111,
	112, 104, 105, 101, 32, 97, 110, 100, 32, 74, 111, 100, 121, 32, 45, 32, 83, 101, 101, 32, 121, 111, 117, 32, 105,
	110, 32, 97, 110, 111, 116, 104, 101, 114, 32, 104, 101, 120, 32, 100, 117, 109, 112, 32, 45, 32, 82, 105, 99,
	104, 97, 114, 100, 32, 70, 108, 111, 119, 101, 114, 32, 49, 49, 47, 49, 49, 47, 49, 57, 57, 57, 0, 0, 0, 0
};

MAP_STRUCT Map[255];
long DoFade;
long snow_outside;

static MESH_DATA* targetMeshP;
static MESH_DATA* binocsMeshP;
static RAINDROPS Rain[2048];
static SNOWFLAKE Snow[2048];
static UWEFFECTS uwdust[256];
static PHD_VECTOR NodeVectors[16];
static float StarFieldPositions[1024];
static long StarFieldColors[256];
static long FadeVal;
static long FadeStep;
static long FadeCnt;
static long FadeEnd;
static short rain_count;
static short snow_count;
static short max_rain;
static short max_snow;

void S_PrintShadow(short size, short* box, ITEM_INFO* item)
{
	TEXTURESTRUCT Tex;
	D3DTLVERTEX v[3];
	PHD_VECTOR pos;
	float* sXYZ;
	long* hXZ;
	long* hY;
	float sxyz[GRID_POINTS * 3];
	long hxz[GRID_POINTS * 2];
	long hy[GRID_POINTS];
	long triA, triB, triC;
	float fx, fy, fz;
	long x, y, z, x1, y1, z1, x2, y2, z2, x3, y3, z3, xSize, zSize, xDist, zDist;
	short room_number;

	xSize = size * (box[1] - box[0]) / 192;	//x size of grid
	zSize = size * (box[5] - box[4]) / 192;	//z size of grid
	xDist = xSize / LINE_POINTS;			//distance between each point of the grid on X
	zDist = zSize / LINE_POINTS;			//distance between each point of the grid on Z
	x = -xDist - (xDist >> 1);				//idfk
	z = zDist + (zDist >> 1);
	sXYZ = sxyz;
	hXZ = hxz;

	for (int i = 0; i < LINE_POINTS; i++, z -= zDist)
	{
		for (int j = 0; j < LINE_POINTS; j++, sXYZ += 3, hXZ += 2, x += xDist)
		{
			sXYZ[0] = (float)x;		//fill shadow XYZ array with the points of the grid
			sXYZ[2] = (float)z;
			hXZ[0] = x;				//fill height XZ array with the points of the grid
			hXZ[1] = z;
		}

		x = -xDist - (xDist >> 1);
	}

	phd_PushUnitMatrix();

	if (item == lara_item)	//position the grid
	{
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;
		GetLaraJointPos(&pos, LM_TORSO);
		room_number = lara_item->room_number;
		y = GetHeight(GetFloor(pos.x, pos.y, pos.z, &room_number), pos.x, pos.y, pos.z);

		if (y == NO_HEIGHT)
			y = item->floor;
	}
	else
	{
		pos.x = item->pos.x_pos;
		y = item->floor;
		pos.z = item->pos.z_pos;
	}

	y -= 16;
	phd_TranslateRel(pos.x, y, pos.z);
	phd_RotY(item->pos.y_rot);	//rot the grid to correct Y
	hXZ = hxz;

	for (int i = 0; i < GRID_POINTS; i++, hXZ += 2)
	{
		x = hXZ[0];
		z = hXZ[1];
		hXZ[0] = (x * phd_mxptr[M00] + z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
		hXZ[1] = (x * phd_mxptr[M20] + z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;
	}

	phd_PopMatrix();

	hXZ = hxz;
	hY = hy;

	for (int i = 0; i < GRID_POINTS; i++, hXZ += 2, hY++)	//Get height on each grid point and store it in hy array
	{
		room_number = item->room_number;
		*hY = GetHeight(GetFloor(hXZ[0], item->floor, hXZ[1], &room_number), hXZ[0], item->floor, hXZ[1]);

		if (abs(*hY - item->floor) > POINT_HEIGHT_CORRECTION)
			*hY = item->floor;
	}

	phd_PushMatrix();
	phd_TranslateAbs(pos.x, y, pos.z);
	phd_RotY(item->pos.y_rot);
	sXYZ = sxyz;
	hY = hy;

	for (int i = 0; i < GRID_POINTS; i++, sXYZ += 3, hY++)
	{
		fx = sXYZ[0];
		fy = (float)(*hY - item->floor);
		fz = sXYZ[2];
		sXYZ[0] = aMXPtr[M00] * fx + aMXPtr[M01] * fy + aMXPtr[M02] * fz + aMXPtr[M03];
		sXYZ[1] = aMXPtr[M10] * fx + aMXPtr[M11] * fy + aMXPtr[M12] * fz + aMXPtr[M13];
		sXYZ[2] = aMXPtr[M20] * fx + aMXPtr[M21] * fy + aMXPtr[M22] * fz + aMXPtr[M23];
	}

	phd_PopMatrix();
	sXYZ = sxyz;

	for (int i = 0; i < NUM_TRIS; i++)	//draw triangles
	{
		triA = 3 * ShadowTable[(i * 3) + 0];	//get tri points
		triB = 3 * ShadowTable[(i * 3) + 1];
		triC = 3 * ShadowTable[(i * 3) + 2];
		x1 = (long)sXYZ[triA + 0];
		y1 = (long)sXYZ[triA + 1];
		z1 = (long)sXYZ[triA + 2];
		x2 = (long)sXYZ[triB + 0];
		y2 = (long)sXYZ[triB + 1];
		z2 = (long)sXYZ[triB + 2];
		x3 = (long)sXYZ[triC + 0];
		y3 = (long)sXYZ[triC + 1];
		z3 = (long)sXYZ[triC + 2];
		setXYZ3(v, x1, y1, z1, x2, y2, z2, x3, y3, z3, clipflags);
		v[0].color = 0x4F000000;
		v[1].color = 0x4F000000;
		v[2].color = 0x4F000000;

		if (item->after_death)
		{
			v[0].color = 0x80000000 - (item->after_death << 24);
			v[1].color = v[0].color;
			v[2].color = v[0].color;
		}

		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		Tex.flag = 0;
		Tex.tpage = 0;
		Tex.drawtype = 3;
		Tex.u1 = 0;
		Tex.v1 = 0;
		Tex.u2 = 0;
		Tex.v2 = 0;
		Tex.u3 = 0;
		Tex.v3 = 0;
		Tex.u4 = 0;
		Tex.v4 = 0;
		AddTriSorted(v, 0, 1, 2, &Tex, 1);
	}
}

void DrawLaserSightSprite()
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR vec;
	long* Z;
	short* pos;
	short* XY;
	float zv;
	short size;

	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	pos = (short*)&scratchpad[512];

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	pos[0] = short(LaserSightX - lara_item->pos.x_pos);
	pos[1] = short(LaserSightY - lara_item->pos.y_pos);
	pos[2] = short(LaserSightZ - lara_item->pos.z_pos);
	vec.x = phd_mxptr[M00] * pos[0] + phd_mxptr[M01] * pos[1] + phd_mxptr[M02] * pos[2] + phd_mxptr[M03];
	vec.y = phd_mxptr[M10] * pos[0] + phd_mxptr[M11] * pos[1] + phd_mxptr[M12] * pos[2] + phd_mxptr[M13];
	vec.z = phd_mxptr[M20] * pos[0] + phd_mxptr[M21] * pos[1] + phd_mxptr[M22] * pos[2] + phd_mxptr[M23];
	zv = f_persp / (float)vec.z;
	XY[0] = short(float(vec.x * zv + f_centerx));
	XY[1] = short(float(vec.y * zv + f_centery));
	Z[0] = vec.z >> 14;
	phd_PopMatrix();

	size = 2;
	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 14];

	setXY4(v, XY[0] - size, XY[1] - size, XY[0] + size, XY[1] - size, XY[0] - size,
		XY[1] + size, XY[0] + size, XY[1] + size, (long)f_mznear, clipflags);

	v[0].color = LaserSightCol ? 0x0000FF00 : 0x00FF0000;
	v[1].color = v[0].color;
	v[2].color = v[0].color;
	v[3].color = v[0].color;
	v[0].specular = 0xFF000000;
	v[1].specular = 0xFF000000;
	v[2].specular = 0xFF000000;
	v[3].specular = 0xFF000000;

	tex.drawtype = 2;
	tex.flag = 0;
	tex.tpage = sprite->tpage;
	tex.u1 = sprite->x2;
	tex.v1 = sprite->y2;
	tex.u2 = sprite->x1;
	tex.v2 = sprite->y2;
	tex.u3 = sprite->x1;
	tex.v3 = sprite->y1;
	tex.u4 = sprite->x2;
	tex.v4 = sprite->y1;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

	LaserSightCol = 0;
	LaserSightActive = 0;
}

void DrawFlatSky(ulong color, long zpos, long ypos, long drawtype)
{
	PHD_VECTOR vec[4];
	D3DTLVERTEX v[4];
	TEXTURESTRUCT Tex;
	short* clip;
	long x, y, z;

	phd_PushMatrix();

	if (gfCurrentLevel)
		phd_TranslateRel(zpos, ypos, 0);
	else
		phd_TranslateRel(0, ypos, 0);

	vec[0].x = -4864;
	vec[0].y = 0;
	vec[0].z = 4864;
	vec[1].x = 4864;
	vec[1].y = 0;
	vec[1].z = 4864;
	vec[2].x = 4864;
	vec[2].y = 0;
	vec[2].z = -4864;
	vec[3].x = -4864;
	vec[3].y = 0;
	vec[3].z = -4864;

	for (int i = 0; i < 4; i++)
	{
		x = vec[i].x;
		y = vec[i].y;
		z = vec[i].z;
		vec[i].x = (phd_mxptr[M00] * x + phd_mxptr[M01] * y + phd_mxptr[M02] * z + phd_mxptr[M03]) >> 14;
		vec[i].y = (phd_mxptr[M10] * x + phd_mxptr[M11] * y + phd_mxptr[M12] * z + phd_mxptr[M13]) >> 14;
		vec[i].z = (phd_mxptr[M20] * x + phd_mxptr[M21] * y + phd_mxptr[M22] * z + phd_mxptr[M23]) >> 14;
		v[i].color = color | 0xFF000000;
		v[i].specular = 0xFF000000;
		CalcColorSplit(color, &v[i].color);
	}

	clip = clipflags;
	ClipCheckPoint(&v[0], (float)vec[0].x, (float)vec[0].y, (float)vec[0].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[1], (float)vec[1].x, (float)vec[1].y, (float)vec[1].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[2], (float)vec[2].x, (float)vec[2].y, (float)vec[2].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[3], (float)vec[3].x, (float)vec[3].y, (float)vec[3].z, clip);	//the only one that survived
	Tex.drawtype = (ushort)drawtype;
	Tex.flag = 0;
	Tex.tpage = ushort(nTextures - 1);
	Tex.u1 = 0.0;
	Tex.v1 = 0.0;
	Tex.u2 = 1.0;
	Tex.v2 = 0.0;
	Tex.u3 = 1.0;
	Tex.v3 = 1.0;
	Tex.u4 = 0.0;
	Tex.v4 = 1.0;
	AddQuadSorted(v, 3, 2, 1, 0, &Tex, 1);
	phd_TranslateRel(-9728, 0, 0);
	vec[0].x = -4864;
	vec[0].y = 0;
	vec[0].z = 4864;
	vec[1].x = 4864;
	vec[1].y = 0;
	vec[1].z = 4864;
	vec[2].x = 4864;
	vec[2].y = 0;
	vec[2].z = -4864;
	vec[3].x = -4864;
	vec[3].y = 0;
	vec[3].z = -4864;

	for (int i = 0; i < 4; i++)
	{
		x = vec[i].x;
		y = vec[i].y;
		z = vec[i].z;
		vec[i].x = (phd_mxptr[M00] * x + phd_mxptr[M01] * y + phd_mxptr[M02] * z + phd_mxptr[M03]) >> 14;
		vec[i].y = (phd_mxptr[M10] * x + phd_mxptr[M11] * y + phd_mxptr[M12] * z + phd_mxptr[M13]) >> 14;
		vec[i].z = (phd_mxptr[M20] * x + phd_mxptr[M21] * y + phd_mxptr[M22] * z + phd_mxptr[M23]) >> 14;
		v[i].color |= 0xFF000000;
		v[i].specular = 0xFF000000;
		CalcColorSplit(color, &v[i].color);
	}

	clip = clipflags;
	ClipCheckPoint(&v[0], (float)vec[0].x, (float)vec[0].y, (float)vec[0].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[1], (float)vec[1].x, (float)vec[1].y, (float)vec[1].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[2], (float)vec[2].x, (float)vec[2].y, (float)vec[2].z, clip);	//originally inlined
	clip++;
	ClipCheckPoint(&v[3], (float)vec[3].x, (float)vec[3].y, (float)vec[3].z, clip);	//the only one that survived

	if (gfCurrentLevel)
		AddQuadSorted(v, 3, 2, 1, 0, &Tex, 1);

	phd_PopMatrix();
}

void S_DrawDarts(ITEM_INFO* item)
{
	D3DTLVERTEX v[2];
	long x1, y1, z1, x2, y2, z2, num, mxx, mxy, mxz, xx, yy, zz;
	float zv;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	zv = f_persp / (float)phd_mxptr[M23];
	x1 = (short)((float)(phd_mxptr[M03] * zv + f_centerx));
	y1 = (short)((float)(phd_mxptr[M13] * zv + f_centery));
	z1 = phd_mxptr[M23] >> 14;
	num = (-96 * phd_cos(item->pos.x_rot)) >> 14;
	mxx = (num * phd_sin(item->pos.y_rot)) >> 14;
	mxy = (96 * phd_sin(item->pos.x_rot)) >> 14;
	mxz = (num * phd_cos(item->pos.y_rot)) >> 14;
	xx = phd_mxptr[M00] * mxx + phd_mxptr[M01] * mxy + phd_mxptr[M02] * mxz + phd_mxptr[M03];
	yy = phd_mxptr[M10] * mxx + phd_mxptr[M11] * mxy + phd_mxptr[M12] * mxz + phd_mxptr[M13];
	zz = phd_mxptr[M20] * mxx + phd_mxptr[M21] * mxy + phd_mxptr[M22] * mxz + phd_mxptr[M23];
	zv = f_persp / (float)zz;
	x2 = (short)((float)(xx * zv + f_centerx));
	y2 = (short)((float)(yy * zv + f_centery));
	z2 = zz >> 14;

	if (ClipLine(x1, y1, z1, x2, y2, z2, phd_winxmin, phd_winymin, phd_winxmax, phd_winymax))
	{
		zv = f_mpersp / (float)z1 * f_moneopersp;
		v[0].color = 0xFF000000;
		v[1].color = 0xFF783C14;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[0].sx = (float)x1;
		v[1].sx = (float)x2;
		v[0].sy = (float)y1;
		v[1].sy = (float)y2;
		v[0].sz = f_a - zv * f_boo;
		v[1].sz = f_a - zv * f_boo;
		v[0].rhw = zv;
		v[1].rhw = zv;
		AddLineSorted(v, &v[1], 6);
	}

	phd_PopMatrix();
}

void DrawMoon()
{
	D3DTLVERTEX v[4];
	SPRITESTRUCT* sprite;
	TEXTURESTRUCT tex;
	SVECTOR vec;
	short* c;
	float x1, x2, y1, y2, z;
	ushort tpage;

	c = clipflags;
	sprite = &spriteinfo[objects[MISC_SPRITES].mesh_index + 3];
	tpage = sprite->tpage < nTextures ? sprite->tpage : 0;
	phd_PushMatrix();
	aSetViewMatrix();
	D3DMView._41 = 0;
	D3DMView._42 = 0;
	D3DMView._43 = 0;
	vec.x = 0;
	vec.y = -1024;
	vec.z = 1920;
	aTransformPerspSV(&vec, v, c, 1, 0);

	if (*c >= 0)
	{
		x1 = v[0].sx - 48.0F;
		x2 = v[0].sx + 48.0F;
		y1 = v[0].sy - 48.0F;
		y2 = v[0].sy + 48.0F;
		z = f_mzfar - 1024.0F;
		aSetXY4(v, x1, y1, x2, y1, x1, y2, x2, y2, z, c);
		v[0].color = 0xC0E0FF;
		v[1].color = 0xC0E0FF;
		v[2].color = 0xC0E0FF;
		v[3].color = 0xC0E0FF;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v2 = sprite->y1;
		tex.u3 = sprite->x2;
		tex.v3 = sprite->y2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		tex.tpage = tpage;
		tex.drawtype = 0;
		AddQuadZBuffer(v, 0, 1, 3, 2, &tex, 1);
	}

	phd_PopMatrix();
}

void DrawGasCloud(ITEM_INFO* item)
{
	GAS_CLOUD* cloud;
	GAS_CLOUD* sec;
	long num;

	if (!TriggerActive(item))
		return;

	if (item->trigger_flags >= 2)
	{
		item->item_flags[0] = 1;
		return;
	}

	cloud = (GAS_CLOUD*)item->data;

	if (!cloud->mTime)
		cloud->yo = -6144.0F;

	TriggerFogBulbFX(0, 128, 0, item->pos.x_pos, long(item->pos.y_pos + cloud->yo), item->pos.z_pos, 4096, 40);

	if (cloud->yo < -3584.0F)
		cloud->yo += 12.0F;
	else
	{
		if (cloud->sTime == 32)
		{
			do num = rand() & 7; while (num == cloud->num);
			cloud->num = num;
		}
		else if (cloud->sTime > 32)
		{
			num = cloud->sTime - 32;

			if (num > 128)
			{
				num = 256 - num;

				if (!num)
					cloud->sTime = 0;
			}

			num = 255 - (num << 1);

			if (num < 64)
				num = 64;

			sec = &cloud[cloud->num];
			TriggerFogBulbFX(0, 255, 0, item->pos.x_pos + sec->t.x, item->pos.y_pos + sec->t.y, item->pos.z_pos + sec->t.z, 1024, num);
		}

		cloud->sTime++;
	}

	cloud->mTime++;

	for (int i = 0; i < 8; i++, cloud++)
	{
		phd_PushMatrix();
		phd_TranslateAbs(item->pos.x_pos + cloud->t.x, item->pos.y_pos + cloud->t.y, item->pos.z_pos + cloud->t.z);
		phd_RotY(-CamRot.y << 4);
		phd_RotX(-4096);
		phd_PopMatrix();
	}
}

void DrawStarField()
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	static long first_time = 0;
	float* pPos;
	long* pCol;
	float x, y, z, fx, fy, fz, bx, by;
	long col;

	if (!first_time)
	{
		pPos = StarFieldPositions;
		pCol = StarFieldColors;

		for (int i = 0; i < 256; i++)
		{
			pPos[0] = ((rand() & 0x1FF) + 512.0F) * fSin(i * 512);
			pPos++;
			pPos[0] = (float)(-rand() % 1900);
			pPos++;
			pPos[0] = ((rand() & 0x1FF) + 512.0F) * fCos(i * 512);
			pPos++;
			pPos[0] = (rand() & 1) + 1.0F;
			pPos++;
			col = rand() & 0x7F;
			pCol[0] = RGBONLY(col + 128, col + 128, col + 192);
			pCol++;
		}

		first_time = 1;
	}

	tex.drawtype = 0;
	tex.tpage = 0;
	tex.flag = 0;
	phd_PushMatrix();
	phd_TranslateAbs(camera.pos.x, camera.pos.y, camera.pos.z);
	SetD3DViewMatrix();
	phd_PopMatrix();
	pPos = StarFieldPositions;
	pCol = StarFieldColors;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;

	for (int i = 0; i < 184; i++)
	{
		fx = pPos[0];
		pPos++;
		fy = pPos[0];
		pPos++;
		fz = pPos[0];
		pPos++;
		col = pCol[0];
		pCol++;
		x = fx * D3DMView._11 + fy * D3DMView._21 + fz * D3DMView._31;
		y = fx * D3DMView._12 + fy * D3DMView._22 + fz * D3DMView._32;
		z = fx * D3DMView._13 + fy * D3DMView._23 + fz * D3DMView._33;
		fy = pPos[0];
		pPos++;

		if (z >= f_mznear)
		{
			fz = f_mpersp / z;
			bx = fz * x + f_centerx;
			by = fz * y + f_centery;

			if (bx >= 0 && bx <= (float)phd_winxmax && by >= 0 && by <= (float)phd_winymax)
			{
				v[0].sx = bx;
				v[0].sy = by;
				v[0].color = col;
				v[0].specular = 0xFF000000;
				v[0].rhw = f_mpersp / f_mzfar * f_moneopersp;
				v[0].tu = 0;
				v[0].tv = 0;
				v[1].sx = bx + fy;
				v[1].sy = by;
				v[1].color = col;
				v[1].specular = 0xFF000000;
				v[1].rhw = f_mpersp / f_mzfar * f_moneopersp;
				v[1].tu = 0;
				v[1].tv = 0;
				v[2].sx = bx;
				v[2].sy = by + fy;
				v[2].color = col;
				v[2].specular = 0xFF000000;
				v[2].rhw = f_mpersp / f_mzfar * f_moneopersp;
				v[2].tu = 0;
				v[2].tv = 0;
				v[3].sx = bx + fy;
				v[3].sy = by + fy;
				v[3].color = col;
				v[3].specular = 0xFF000000;
				v[3].rhw = f_mpersp / f_mzfar * f_moneopersp;
				v[3].tu = 0;
				v[3].tv = 0;
				AddQuadZBuffer(v, 0, 1, 3, 2, &tex, 1);
			}
		}
	}
}

void setXYZ3(D3DTLVERTEX* v, long x1, long y1, long z1, long x2, long y2, long z2, long x3, long y3, long z3, short* clip)
{
	float zv;
	short clipFlag;

	clipFlag = 0;
	v->tu = (float)x1;
	v->tv = (float)y1;
	v->sz = (float)z1;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	v++;
	clip[0] = clipFlag;
	clipFlag = 0;
	v->tu = (float)x2;
	v->tv = (float)y2;
	v->sz = (float)z2;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	v++;
	clip[1] = clipFlag;
	clipFlag = 0;
	v->tu = (float)x3;
	v->tv = (float)y3;
	v->sz = (float)z3;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	clip[2] = clipFlag;
}

void setXYZ4(D3DTLVERTEX* v, long x1, long y1, long z1, long x2, long y2, long z2, long x3, long y3, long z3, long x4, long y4, long z4, short* clip)
{
	float zv;
	short clipFlag;

	clipFlag = 0;
	v->tu = (float)x1;
	v->tv = (float)y1;
	v->sz = (float)z1;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv= f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	v++;
	clip[0] = clipFlag;
	clipFlag = 0;
	v->tu = (float)x2;
	v->tv = (float)y2;
	v->sz = (float)z2;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	v++;
	clip[1] = clipFlag;
	clipFlag = 0;
	v->tu = (float)x3;
	v->tv = (float)y3;
	v->sz = (float)z3;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	v++;
	clip[2] = clipFlag;
	clipFlag = 0;
	v->tu = (float)x4;
	v->tv = (float)y4;
	v->sz = (float)z4;

	if (v->sz < f_mznear)
		clipFlag = -128;
	else
	{
		zv = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			clipFlag = 256;
			v->sz = f_zfar;
		}

		v->sx = zv * v->tu + f_centerx;
		v->sy = zv * v->tv + f_centery;
		v->rhw = f_moneopersp * zv;

		if (phd_winxmin > v->sx)
			clipFlag++;
		else if (phd_winxmax < v->sx)
			clipFlag += 2;

		if (phd_winymin > v->sy)
			clipFlag += 4;
		else if (phd_winymax < v->sy)
			clipFlag += 8;
	}

	clip[3] = clipFlag;
}

void setXY3(D3DTLVERTEX* v, long x1, long y1, long x2, long y2, long x3, long y3, long z, short* clip)
{
	float zv;
	short clipFlag;

	clipFlag = 0;
	zv = f_mpersp / (float)z;
	v->sx = (float)x1;
	v->sy = (float)y1;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[0] = clipFlag;
	v++;
	clipFlag = 0;
	v->sx = (float)x2;
	v->sy = (float)y2;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[1] = clipFlag;
	v++;
	clipFlag = 0;
	v->sx = (float)x3;
	v->sy = (float)y3;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[2] = clipFlag;
}

void setXY4(D3DTLVERTEX* v, long x1, long y1, long x2, long y2, long x3, long y3, long x4, long y4, long z, short* clip)
{
	float zv;
	short clipFlag;

	clipFlag = 0;
	zv = f_mpersp / (float)z;
	v->sx = (float)x1;
	v->sy = (float)y1;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[0] = clipFlag;
	v++;
	clipFlag = 0;
	v->sx = (float)x2;
	v->sy = (float)y2;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[1] = clipFlag;
	v++;
	clipFlag = 0;
	v->sx = (float)x3;
	v->sy = (float)y3;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[2] = clipFlag;
	v++;
	clipFlag = 0;
	v->sx = (float)x4;
	v->sy = (float)y4;
	v->sz = (float)z;
	v->rhw = f_moneopersp * zv;

	if (phd_winxmin > v->sx)
		clipFlag++;
	else if (phd_winxmax < v->sx)
		clipFlag += 2;

	if (phd_winymin > v->sy)
		clipFlag += 4;
	else if (phd_winymax < v->sy)
		clipFlag += 8;

	clip[3] = clipFlag;
}

void S_DrawDrawSparksNEW(SPARKS* sptr, long smallest_size, float* xyz)
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float x0, y0, x1, y1, x2, y2, x3, y3;
	float fs1, fs2, sin, cos, sinf1, sinf2, cosf1, cosf2;
	long s, scale, s1, s2;

	if (!(sptr->Flags & 8))
		return;

	if (xyz[2] <= f_mznear || xyz[2] >= f_mzfar)
	{
		if (xyz[2] >= f_mzfar)
			sptr->On = 0;
	}
	else
	{
		if (sptr->Flags & 2)
		{
			scale = sptr->Size << sptr->Scalar;
			s = ((phd_persp * sptr->Size) << sptr->Scalar) / (long)xyz[2];
			s1 = s;
			s2 = s;

			if (s > scale)
				s1 = scale;
			else if (s < smallest_size)
				s1 = smallest_size;

			if (s > scale)
				s1 = scale;
			else if (s < smallest_size)
				s2 = smallest_size;
		}
		else
		{
			s1 = sptr->Size;
			s2 = s1;
		}

		fs1 = (float)s1;
		fs2 = (float)s2;

		if ((fs1 * 2) + xyz[0] >= f_left && xyz[0] - (fs1 * 2) < f_right && (fs2 * 2) + xyz[1] >= f_top && xyz[1] - (fs2 * 2) < f_bottom)
		{
			fs1 *= 0.5F;
			fs2 *= 0.5F;

			if (sptr->Flags & 0x10)
			{
				sin = fSin(sptr->RotAng << 1);
				cos = fCos(sptr->RotAng << 1);
				sinf1 = sin * fs1;
				sinf2 = sin * fs2;
				cosf1 = cos * fs1;
				cosf2 = cos * fs2;
				x0 = cosf2 - sinf1 + xyz[0];
				y0 = xyz[1] - cosf1 - sinf2;
				x1 = sinf1 + cosf2 + xyz[0];
				y1 = cosf1 + xyz[1] - sinf2;
				x2 = sinf1 - cosf2 + xyz[0];
				y2 = cosf1 + xyz[1] + sinf2;
				x3 = -sinf1 - cosf2 + xyz[0];
				y3 = xyz[1] - cosf1 + sinf2;
				aSetXY4(v, x0, y0, x1, y1, x2, y2, x3, y3, xyz[2], clipflags);
			}
			else
			{
				x0 = xyz[0] - fs1;
				x1 = fs1 + xyz[0];
				y0 = xyz[1] - fs2;
				y1 = fs2 + xyz[1];
				aSetXY4(v, x0, y0, x1, y0, x1, y1, x0, y1, xyz[2], clipflags);
			}

			sprite = &spriteinfo[sptr->Def];
			v[0].color = RGBA(sptr->R, sptr->G, sptr->B, 0xFF);
			v[1].color = v[0].color;
			v[2].color = v[0].color;
			v[3].color = v[0].color;
			v[0].specular = 0xFF000000;
			v[1].specular = 0xFF000000;
			v[2].specular = 0xFF000000;
			v[3].specular = 0xFF000000;

			if (sptr->TransType)
				tex.drawtype = 2;
			else
				tex.drawtype = 1;

			tex.tpage = sprite->tpage;
			tex.u1 = sprite->x1;
			tex.v1 = sprite->y1;
			tex.u2 = sprite->x2;
			tex.v2 = sprite->y1;
			tex.u3 = sprite->x2;
			tex.v3 = sprite->y2;
			tex.u4 = sprite->x1;
			tex.v4 = sprite->y2;
			AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
		}
	}
}

void DoRain()
{
	RAINDROPS* rptr;
	ROOM_INFO* r;
	D3DTLVERTEX v[2];
	TEXTURESTRUCT tex;
	FVECTOR vec;
	FVECTOR vec2;
	short* clip;
	float ctop, cbottom, cright, cleft, zv, zz;
	long num_alive, rad, angle, rnd, x, z, x_size, y_size, clr;
	short room_number, clipFlag;

	num_alive = 0;

	for (int i = 0; i < rain_count; i++)
	{
		rptr = &Rain[i];

		if (outside && !rptr->x && num_alive < max_rain)
		{
			num_alive++;
			rad = GetRandomDraw() & 8191;
			angle = GetRandomDraw() & 8190;
			rptr->x = camera.pos.x + (rad * rcossin_tbl[angle] >> 12);
			rptr->y = camera.pos.y + -1024 - (GetRandomDraw() & 2047);
			rptr->z = camera.pos.z + (rad * rcossin_tbl[angle + 1] >> 12);

			if (IsRoomOutside(rptr->x, rptr->y, rptr->z) < 0)
			{
				rptr->x = 0;
				continue;
			}

			if (room[IsRoomOutsideNo].flags & ROOM_UNDERWATER)
			{
				rptr->x = 0;
				continue;
			}

			rptr->xv = (GetRandomDraw() & 7) - 4;
			rptr->yv = (GetRandomDraw() & 3) + 24;
			rptr->zv = (GetRandomDraw() & 7) - 4;
			rptr->room_number = IsRoomOutsideNo;
			rptr->life = 64 - rptr->yv;
		}

		if (rptr->x)
		{
			if (rptr->life > 240 || abs(CamPos.x - rptr->x) > 6000 || abs(CamPos.z - rptr->z) > 6000)
			{
				rptr->x = 0;
				continue;
			}

			rptr->x += rptr->xv + 4 * SmokeWindX;
			rptr->y += rptr->yv << 3;
			rptr->z += rptr->zv + 4 * SmokeWindZ;
			r = &room[rptr->room_number];
			x = r->x + 1024;
			z = r->z + 1024;
			x_size = r->x_size - 1;
			y_size = r->y_size - 1;

			if (rptr->y <= r->maxceiling || rptr->y >= r->minfloor || rptr->z <= z ||
				rptr->z >= r->z + (x_size << 10) || rptr->x <= x || rptr->x >= r->x + (y_size << 10))
			{
				room_number = rptr->room_number;
				GetFloor(rptr->x, rptr->y, rptr->z, &room_number);

				if (room_number == rptr->room_number || room[room_number].flags & ROOM_UNDERWATER)
				{
					TriggerSmallSplash(rptr->x, rptr->y, rptr->z, 1);
					rptr->x = 0;
					continue;
				}
				else
					rptr->room_number = room_number;
			}

			rnd = GetRandomDraw();

			if ((rnd & 3) != 3)
			{
				rptr->xv += (rnd & 3) - 1;

				if (rptr->xv < -4)
					rptr->xv = -4;
				else if (rptr->xv > 4)
					rptr->xv = 4;
			}

			rnd = (rnd >> 2) & 3;

			if (rnd != 3)
			{
				rptr->zv += (char)(rnd - 1);

				if (rptr->zv < -4)
					rptr->zv = -4;
				else if (rptr->zv > 4)
					rptr->zv = 4;
			}

			rptr->life -= 2;

			if (rptr->life > 240)
				rptr->x = 0;
		}
	}

	tex.drawtype = 2;
	tex.tpage = 0;
	tex.flag = 0;
	ctop = f_top;
	cleft = f_left + 4.0F;
	cbottom = f_bottom;
	cright = f_right - 4.0F;
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	SetD3DViewMatrix();

	for (int i = 0; i < rain_count; i++)
	{
		rptr = &Rain[i];

		if (rptr->x)
		{
			clipFlag = 0;
			clip = clipflags;
			vec.x = (float)(rptr->x - lara_item->pos.x_pos - 2 * SmokeWindX);
			vec.y = (float)(rptr->y - 8 * rptr->yv - lara_item->pos.y_pos);
			vec.z = (float)(rptr->z - lara_item->pos.z_pos - 2 * SmokeWindZ);
			vec2.x = vec.x * D3DMView._11 + vec.y * D3DMView._21 + vec.z * D3DMView._31 + D3DMView._41;
			vec2.y = vec.x * D3DMView._12 + vec.y * D3DMView._22 + vec.z * D3DMView._32 + D3DMView._42;
			vec2.z = vec.x * D3DMView._13 + vec.y * D3DMView._23 + vec.z * D3DMView._33 + D3DMView._43;
			zz = vec2.z;
			clr = (long)((1.0F - (f_mzfar - vec2.z) * (1.0F / f_mzfar)) * 8.0F + 8.0F);
			v[0].specular = 0xFF000000;
			v[0].color = RGBA(clr, clr, clr, 128);
			v[0].tu = vec2.x;
			v[0].tv = vec2.y;

			if (vec2.z < f_mznear)
				clipFlag = -128;
			else
			{
				if (vec2.z > FogEnd)
				{
					zz = f_zfar;
					clipFlag = 16;
				}

				zv = f_mpersp / zz;
				v[0].sx = zv * vec2.x + f_centerx;
				v[0].sy = zv * vec2.y + f_centery;
				v[0].rhw = f_moneopersp * zv;

				if (v[0].sx < cleft)
					clipFlag++;
				else if (v[0].sx > cright)
					clipFlag += 2;

				if (v[0].sy < ctop)
					clipFlag += 4;
				else if (v[0].sy > cbottom)
					clipFlag += 8;
			}

			v[0].sz = zz;
			clip[0] = clipFlag;
			clipFlag = 0;
			clip++;

			vec.x = (float)(rptr->x - lara_item->pos.x_pos);
			vec.y = (float)(rptr->y - lara_item->pos.y_pos);
			vec.z = (float)(rptr->z - lara_item->pos.z_pos);
			vec2.x = vec.x * D3DMView._11 + vec.y * D3DMView._21 + vec.z * D3DMView._31 + D3DMView._41;
			vec2.y = vec.x * D3DMView._12 + vec.y * D3DMView._22 + vec.z * D3DMView._32 + D3DMView._42;
			vec2.z = vec.x * D3DMView._13 + vec.y * D3DMView._23 + vec.z * D3DMView._33 + D3DMView._43;
			clr = (long)((1.0F - (f_mzfar - vec2.z) * (1.0F / f_mzfar)) * 16.0F + 16.0F);
			v[1].specular = 0xFF000000;
			v[1].color = RGBA(clr, clr, clr, 128);
			v[1].tu = vec2.x;
			v[1].tv = vec2.y;

			if (vec2.z < f_mznear)
				clipFlag = -128;
			else
			{
				if (vec2.z > FogEnd)
				{
					zz = f_zfar;
					clipFlag = 16;
				}

				zv = f_mpersp / zz;
				v[1].sx = zv * vec2.x + f_centerx;
				v[1].sy = zv * vec2.y + f_centery;
				v[1].rhw = f_moneopersp * zv;

				if (v[1].sx < cleft)
					clipFlag++;
				else if (v[1].sx > cright)
					clipFlag += 2;

				if (v[1].sy < ctop)
					clipFlag += 4;
				else if (v[1].sy > cbottom)
					clipFlag += 8;
			}

			v[1].sz = zz;
			clip[0] = clipFlag;

			if (!clipflags[0] && !clipflags[1])
				AddPolyLine(v, &tex);
		}
	}

	phd_PopMatrix();
}

void OutputSky()
{
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 1);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);
	DrawBuckets();
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 1);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 1);
	SortPolyList(SortCount, SortList);
	RestoreFPCW(FPCW);
#if 0
	MMXSetPerspecLimit(0);
	DrawSortList();
	MMXSetPerspecLimit(0x3F19999A);
#else
	DrawSortList();
#endif
	MungeFPCW(&FPCW);
	InitBuckets();
	InitialiseSortList();
}

void SetFade(long start, long end)
{
	DoFade = 1;
	FadeVal = start;
	FadeStep = (end - start) >> 3;
	FadeCnt = 0;
	FadeEnd = end;
}

void DoScreenFade()
{
	FadeVal += FadeStep;
	FadeCnt++;

	if (FadeCnt > 8)
		DoFade = 2;

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
}

void ClipCheckPoint(D3DTLVERTEX* v, float x, float y, float z, short* clip)
{
	float perspz;
	short clipdistance;

	v->tu = x;
	v->tv = y;
	v->sz = z;
	clipdistance = 0;

	if (v->sz < f_mznear)
		clipdistance = -128;
	else
	{
		perspz = f_mpersp / v->sz;

		if (v->sz > FogEnd)
		{
			v->sz = f_zfar;
			clipdistance = 256;
		}

		v->sx = perspz * v->tu + f_centerx;
		v->sy = perspz * v->tv + f_centery;
		v->rhw = perspz * f_moneopersp;

		if (v->sx < phd_winxmin)
			clipdistance++;
		else if (phd_winxmax < v->sx)
			clipdistance += 2;

		if (v->sy < phd_winymin)
			clipdistance += 4;
		else if (v->sy > phd_winymax)
			clipdistance += 8;
	}

	clip[0] = clipdistance;
}

void aTransformPerspSV(SVECTOR* vec, D3DTLVERTEX* v, short* c, long nVtx, long col)
{
	float x, y, z, vx, vy, vz, zv;
	short clip;

	for (int i = 0; i < nVtx; i++)
	{
		clip = 0;
		vx = vec->x;
		vy = vec->y;
		vz = vec->z;
		x = D3DMView._11 * vx + D3DMView._21 * vy + D3DMView._31 * vz + D3DMView._41;
		y = D3DMView._12 * vx + D3DMView._22 * vy + D3DMView._32 * vz + D3DMView._42;
		z = D3DMView._13 * vx + D3DMView._23 * vy + D3DMView._33 * vz + D3DMView._43;
		v->tu = x;
		v->tv = y;

		if (z < f_mznear)
			clip = -128;
		else
		{
			zv = f_mpersp / z;
			x = x * zv + f_centerx;
			y = y * zv + f_centery;
			v->rhw = f_moneopersp * zv;

			if (x < f_left)
				clip = 1;
			else if (x > f_right)
				clip = 2;

			if (y < f_top)
				clip += 4;
			else if (y > f_bottom)
				clip += 8;

			v->sx = x;
			v->sy = y;
		}

		v->sz = z;
		v->color = col;
		v->specular = 0xFF000000;
		*c++ = clip;
		v++;
		vec++;
	}
}

void DrawBinoculars()
{
	MESH_DATA* mesh;
	D3DTLVERTEX* v;
	TEXTURESTRUCT* tex;
	D3DTLVERTEX vtx[256];
	D3DTLVERTEX irVtx[4];
	TEXTURESTRUCT irTex;
	short* clip;
	short* quad;
	short* tri;
	ushort drawbak;
	short c;

	if (LaserSight || SniperOverlay)
		mesh = targetMeshP;
	else
		mesh = binocsMeshP;

	v = (D3DTLVERTEX*)mesh->aVtx;
	clip = clipflags;

	for (int i = 0; i < mesh->nVerts; i++)
	{
		c = 0;
		vtx[i] = v[i];
		vtx[i].sx = (vtx[i].sx * float(phd_winxmax / 512.0F)) + f_centerx;
		vtx[i].sy = (vtx[i].sy * float(phd_winymax / 240.0F)) + f_centery;

		if (vtx[i].sx < f_left)
			c = 1;
		else if (vtx[i].sx > f_right)
			c = 2;

		if (vtx[i].sy < f_top)
			c += 4;
		else if (vtx[i].sy > f_bottom)
			c += 8;

		*clip++ = c;
	}

	quad = mesh->gt4;
	tri = mesh->gt3;

	if (LaserSight || SniperOverlay)
	{
		for (int i = 0; i < mesh->ngt4; i++, quad += 6)
		{
			tex = &textinfo[quad[4] & 0x7FFF];
			drawbak = tex->drawtype;
			tex->drawtype = 0;

			if (quad[5] & 1)
			{
				vtx[quad[0]].color = 0xFF000000;
				vtx[quad[1]].color = 0xFF000000;
				vtx[quad[2]].color = 0;
				vtx[quad[3]].color = 0;
				tex->drawtype = 3;
			}

			AddQuadSorted(vtx, quad[0], quad[1], quad[2], quad[3], tex, 1);
			tex->drawtype = drawbak;
		}

		for (int i = 0, j = 0; i < mesh->ngt3; i++, tri += 5)
		{
			tex = &textinfo[tri[3] & 0x7FFF];
			drawbak = tex->drawtype;
			tex->drawtype = 0;

			if (tri[4] & 1)
			{
				vtx[tri[0]].color = TargetGraphColTab[j] << 24;
				vtx[tri[1]].color = TargetGraphColTab[j + 1] << 24;
				vtx[tri[2]].color = TargetGraphColTab[j + 2] << 24;
				tex->drawtype = 3;
				j += 3;
			}

			AddTriSorted(vtx, tri[0], tri[1], tri[2], tex, 1);
			tex->drawtype = drawbak;
		}
	}
	else
	{
		for (int i = 0; i < mesh->ngt4; i++, quad += 6)
		{
			tex = &textinfo[quad[4] & 0x7FFF];
			drawbak = tex->drawtype;
			tex->drawtype = 0;

			if (gfCurrentLevel == 9)
			{
				if (i < 14)
				{
					if (quad[5] & 1)
					{
						vtx[quad[0]].color = 0xFF000000;
						vtx[quad[1]].color = 0xFF000000;
						vtx[quad[2]].color = 0xFF000000;
						vtx[quad[3]].color = 0xFF000000;
						tex->drawtype = 3;
					}
				}
				else
				{
					if (quad[5] & 1)
					{
						vtx[quad[0]].color = 0;
						vtx[quad[1]].color = 0;
						vtx[quad[2]].color = 0xFF000000;
						vtx[quad[3]].color = 0xFF000000;
						tex->drawtype = 3;
					}
				}
			}
			else
			{
				if (quad[5] & 1)
				{
					vtx[quad[0]].color = 0xFF000000;
					vtx[quad[1]].color = 0xFF000000;
					vtx[quad[2]].color = 0;
					vtx[quad[3]].color = 0;
					tex->drawtype = 3;
				}
			}

			AddQuadSorted(vtx, quad[0], quad[1], quad[2], quad[3], tex, 1);
			tex->drawtype = drawbak;
		}

		for (int i = 0; i < mesh->ngt3; i++, tri += 5)
		{
			tex = &textinfo[tri[3] & 0x7FFF];
			drawbak = tex->drawtype;
			tex->drawtype = 0;

			if (gfCurrentLevel < 11)
			{
				if (tri[4] & 1)
				{
					vtx[tri[0]].color = 0;
					vtx[tri[1]].color = 0xFF000000;
					vtx[tri[2]].color = 0;
					tex->drawtype = 3;
				}
			}
			else
			{
				if (i < mesh->ngt3 - 2)
				{
					if (tri[4] & 1)
					{
						vtx[tri[0]].color = 0xFF000000;
						vtx[tri[1]].color = 0xFF000000;
						vtx[tri[2]].color = 0;
						tex->drawtype = 3;
					}
				}
				else if (i == mesh->ngt3 - 2)
				{
					if (tri[4] & 1)
					{
						vtx[tri[0]].color = 0;
						vtx[tri[1]].color = 0;
						vtx[tri[2]].color = 0xFF000000;
						tex->drawtype = 3;
					}
				}
				else
				{
					if (tri[4] & 1)
					{
						vtx[tri[0]].color = 0;
						vtx[tri[1]].color = 0xFF000000;
						vtx[tri[2]].color = 0;
						tex->drawtype = 3;
					}
				}
			}

			AddTriSorted(vtx, tri[0], tri[1], tri[2], tex, 1);
			tex->drawtype = drawbak;
		}

		if (InfraRed)
		{
			aSetXY4(irVtx, 0, 0, phd_winxmax, 0, 0, phd_winymax, phd_winxmax, phd_winymax, f_mznear + 1, clipflags);
			irVtx[0].color = 0x64FF0000;
			irVtx[1].color = 0x64FF0000;
			irVtx[2].color = 0x64FF0000;
			irVtx[3].color = 0x64FF0000;
			irVtx[0].specular = 0xFF000000;
			irVtx[1].specular = 0xFF000000;
			irVtx[2].specular = 0xFF000000;
			irVtx[3].specular = 0xFF000000;
			irTex.drawtype = 3;
			irTex.tpage = 0;
			AddQuadSorted(irVtx, 0, 1, 3, 2, &irTex, 1);
		}
	}
}

void aDrawWreckingBall(ITEM_INFO* item, long shade)
{
	SPRITESTRUCT* sprite;
	SVECTOR* vec;
	TEXTURESTRUCT tex;
	long x, z, s;

	aSetViewMatrix();
	vec = (SVECTOR*)&scratchpad[0];
	s = (400 * shade) >> 7;
	x = -s;

	for (int i = 0; i < 3; i++)
	{
		z = -s;

		for (int j = 0; j < 3; j++)
		{
			vec->x = (short)x;
			vec->y = -2;
			vec->z = (short)z;
			vec++;
			z += s;
		}

		x += s;
	}

	if (shade < 100)
		shade = 100;

	vec = (SVECTOR*)&scratchpad[0];
	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 11];
	aTransformPerspSV(vec, aVertexBuffer, clipflags, 9, shade << 24);
	tex.drawtype = 3;

	tex.tpage = sprite->tpage;
	tex.u1 = sprite->x1;
	tex.v1 = sprite->y1;
	tex.u2 = sprite->x2;
	tex.v2 = sprite->y1;
	tex.u3 = sprite->x2;
	tex.v3 = sprite->y2;
	tex.u4 = sprite->x1;
	tex.v4 = sprite->y2;
	AddQuadSorted(aVertexBuffer, 0, 2, 8, 6, &tex, 1);
}

void ClearFX()
{
	for (int i = 0; i < 2048; i++)
	{
		Rain[i].x = 0;
		Snow[i].x = 0;
	}
}

void AddPolyLine(D3DTLVERTEX* vtx, TEXTURESTRUCT* tex)
{
	D3DTLVERTEX v[4];
	float x0, y0, x1, y1, x2, y2, x3, y3;
	short cf0, cf1;

	x0 = vtx->sx;
	y0 = vtx->sy;
	x1 = vtx[1].sx;
	y1 = vtx[1].sy;
	cf0 = clipflags[0];
	cf1 = clipflags[1];

	if (fabs(x1 - x0) <= fabs(y1 - y0))
	{
		x2 = x0 + 2.0F;
		y2 = y0;
		x3 = x1 + 2.0F;
		y3 = y1;
	}
	else
	{
		x2 = x0;
		y2 = y0 + 2.0F;
		x3 = x1;
		y3 = y1 + 2.0F;
	}

	v[0].sx = x0;
	v[0].sy = y0;
	v[0].rhw = vtx->rhw;
	v[0].tu = vtx->tu;
	v[0].tv = vtx->tv;
	v[0].color = vtx->color;
	v[0].specular = 0xFF000000;

	v[1].sx = x2;
	v[1].sy = y2;
	v[1].rhw = vtx->rhw;
	v[1].tu = vtx->tu;
	v[1].tv = vtx->tv;
	v[1].color = vtx->color;
	v[1].specular = 0xFF000000;

	v[2].sx = x1;
	v[2].sy = y1;
	v[2].rhw = vtx->rhw;
	v[2].tu = vtx[1].tu;
	v[2].tv = vtx[1].tv;
	v[2].color = vtx[1].color;
	v[2].specular = 0xFF000000;

	v[3].sx = x3;
	v[3].sy = y3;
	v[3].rhw = vtx->rhw;
	v[3].tu = vtx[1].tu;
	v[3].tv = vtx[1].tv;
	v[3].color = vtx[1].color;
	v[3].specular = 0xFF000000;

	clipflags[0] = cf0;
	clipflags[1] = cf0;
	clipflags[2] = cf1;
	clipflags[3] = cf1;

	if (tex->drawtype == 3 || tex->drawtype == 2)
		AddQuadSorted(v, 0, 1, 2, 3, tex, 0);
	else
		AddQuadZBuffer(v, 0, 1, 2, 3, tex, 0);
}

void DoSnow()
{
	SNOWFLAKE* snow;
	ROOM_INFO* r;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float* pSize;
	float x, y, z, xv, yv, zv, vx, vy, xSize, ySize;
	long num_alive, rad, angle, ox, oy, oz, col;
	short room_number, clipFlag;

	num_alive = 0;

	for (int i = 0; i < snow_count; i++)
	{
		snow = &Snow[i];

		if (!snow->x)
		{
			if (!snow_outside || num_alive >= max_snow)
				continue;

			num_alive++;
			rad = GetRandomDraw() & 0x1FFF;
			angle = (GetRandomDraw() & 0xFFF) << 1;
			snow->x = camera.pos.x + (rad * rcossin_tbl[angle] >> 12);
			snow->y = camera.pos.y - 1024 - (GetRandomDraw() & 0x7FF);
			snow->z = camera.pos.z + (rad * rcossin_tbl[angle + 1] >> 12);

			if (IsRoomOutside(snow->x, snow->y, snow->z) < 0)
			{
				snow->x = 0;
				continue;
			}

			if (room[IsRoomOutsideNo].flags & ROOM_UNDERWATER)
			{
				snow->x = 0;
				continue;
			}

			snow->stopped = 0;
			snow->xv = (GetRandomDraw() & 7) - 4;
			snow->yv = ((GetRandomDraw() & 0xF) + 8) << 3;
			snow->zv = (GetRandomDraw() & 7) - 4;
			snow->room_number = IsRoomOutsideNo;
			snow->life = 112 - (snow->yv >> 2);
		}

		ox = snow->x;
		oy = snow->y;
		oz = snow->z;

		if (!snow->stopped)
		{
			snow->x += snow->xv;
			snow->y += (snow->yv >> 1) & 0xFC;
			snow->z += snow->zv;
			r = &room[snow->room_number];

			if (snow->y <= r->maxceiling || snow->y >= r->minfloor ||
				snow->z <= r->z + 1024 || snow->z >= (r->x_size << 10) + r->z - 1024 ||
				snow->x <= r->x + 1024 || snow->x >= (r->y_size << 10) + r->x - 1024)
			{
				room_number = snow->room_number;
				GetFloor(snow->x, snow->y, snow->z, &room_number);

				if (room_number == snow->room_number)
				{
					snow->x = 0;
					continue;
				}

				if (room[room_number].flags & ROOM_UNDERWATER)
				{
					snow->stopped = 1;
					snow->x = ox;
					snow->y = oy;
					snow->z = oz;

					if (snow->life > 16)
						snow->life = 16;
				}
				else
					snow->room_number = room_number;
			}
		}

		if (!snow->life)
		{
			snow->x = 0;
			continue;
		}

		if ((abs(CamPos.x - snow->x) > 6000 || abs(CamPos.z - snow->z) > 6000) && snow->life > 16)
			snow->life = 16;

		if (snow->xv < SmokeWindX << 2)
			snow->xv += 2;
		else if (snow->xv > SmokeWindX << 2)
			snow->xv -= 2;

		if (snow->zv < SmokeWindZ << 2)
			snow->zv += 2;
		else if (snow->zv > SmokeWindZ << 2)
			snow->zv -= 2;

		snow->life -= 2;

		if ((snow->yv & 7) != 7)
			snow->yv++;
	}

	mAddProfilerEvent(0xFF0000FF);
	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 10];
	tex.tpage = sprite->tpage;
	tex.drawtype = 2;
	tex.flag = 0;
	tex.u1 = sprite->x2;
	tex.v1 = sprite->y1;
	tex.u2 = sprite->x2;
	tex.v2 = sprite->y2;
	tex.v3 = sprite->y2;
	tex.u3 = sprite->x1;
	tex.u4 = sprite->x1;
	tex.v4 = sprite->y2;

	phd_PushMatrix();
	phd_TranslateAbs(camera.pos.x, camera.pos.y, camera.pos.z);
	SetD3DViewMatrix();

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;

	for (int i = 0; i < snow_count; i++)
	{
		snow = &Snow[i];

		if (!snow->x)
			continue;

		x = float(snow->x - camera.pos.x);
		y = float(snow->y - camera.pos.y);
		z = float(snow->z - camera.pos.z);
		zv = x * D3DMView._13 + y * D3DMView._23 + z * D3DMView._33 + D3DMView._43;

		if (zv < f_mznear)
			continue;

		col = 0;

		if ((snow->yv & 7) != 7)
			col = (snow->yv & 7) << 4;
		else if (snow->life > 32)
			col = 130;
		else
			col = snow->life << 3;

		col = RGBA(col, col, col, 0xFF);
		pSize = &SnowSizes[snow->yv & 24];
		xv = x * D3DMView._11 + y * D3DMView._21 + z * D3DMView._31 + D3DMView._41;
		yv = x * D3DMView._12 + y * D3DMView._22 + z * D3DMView._32 + D3DMView._42;
		zv = f_mpersp / zv;

		for (int j = 0; j < 4; j++)
		{
			xSize = pSize[0] * zv;
			ySize = pSize[1] * zv;
			pSize += 2;

			vx = xv * zv + xSize + f_centerx;
			vy = yv * zv + ySize + f_centery;
			clipFlag = 0;

			if (vx < f_left)
				clipFlag++;
			else if (vx > f_right)
				clipFlag += 2;

			if (vy < f_top)
				clipFlag += 4;
			else if (vy > f_bottom)
				clipFlag += 8;

			clipflags[j] = clipFlag;
			v[j].sx = vx;
			v[j].sy = vy;
			v[j].rhw = zv * f_moneopersp;
			v[j].tu = 0;
			v[j].tv = 0;
			v[j].color = col;
			v[j].specular = 0xFF000000;
		}

		AddTriSorted(v, 2, 0, 1, &tex, 1);
	}

	phd_PopMatrix();
}

void aInitFX()
{
	if (G_dxptr->Flags & 0x80)
	{
		snow_count = 2048;
		rain_count = 2048;
		max_snow = 128;
		max_rain = 128;
	}
#if 0
	else
	{
		snow_count = 256;
		rain_count = 256;
		max_snow = 8;
		max_rain = 8;
	}
#endif
}

void DoWeather()
{
	if (WeatherType == 1)
		DoRain();
	else if (WeatherType == 2)
		DoSnow();
}

void aSetXY4(D3DTLVERTEX* v, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float z, short* clip)
{
	float zv;
	short clipFlag;

	zv = f_mpersp / z * f_moneopersp;

	clipFlag = 0;

	if (x1 < f_left)
		clipFlag++;
	else if (x1 > f_right)
		clipFlag += 2;

	if (y1 < f_top)
		clipFlag += 4;
	else if (y1 > f_bottom)
		clipFlag += 8;

	clip[0] = clipFlag;
	v[0].sx = x1;
	v[0].sy = y1;
	v[0].sz = z;
	v[0].rhw = zv;

	clipFlag = 0;

	if (x2 < f_left)
		clipFlag++;
	else if (x2 > f_right)
		clipFlag += 2;

	if (y2 < f_top)
		clipFlag += 4;
	else if (y2 > f_bottom)
		clipFlag += 8;

	clip[1] = clipFlag;
	v[1].sx = x2;
	v[1].sy = y2;
	v[1].sz = z;
	v[1].rhw = zv;

	clipFlag = 0;

	if (x3 < f_left)
		clipFlag++;
	else if (x3 > f_right)
		clipFlag += 2;

	if (y3 < f_top)
		clipFlag += 4;
	else if (y3 > f_bottom)
		clipFlag += 8;

	clip[2] = clipFlag;
	v[2].sx = x3;
	v[2].sy = y3;
	v[2].sz = z;
	v[2].rhw = zv;

	clipFlag = 0;

	if (x4 < f_left)
		clipFlag++;
	else if (x4 > f_right)
		clipFlag += 2;

	if (y4 < f_top)
		clipFlag += 4;
	else if (y4 > f_bottom)
		clipFlag += 8;

	clip[3] = clipFlag;
	v[3].sx = x4;
	v[3].sy = y4;
	v[3].sz = z;
	v[3].rhw = zv;
}

void InitTarget()
{
	OBJECT_INFO* obj;
	ACMESHVERTEX* p;
	D3DTLVERTEX* v;

	obj = &objects[TARGET_GRAPHICS];

	if (!obj->loaded)
		return;

	targetMeshP = (MESH_DATA*)meshes[obj->mesh_index];
	p = targetMeshP->aVtx;
	targetMeshP->aVtx = (ACMESHVERTEX*)game_malloc(targetMeshP->nVerts * sizeof(ACMESHVERTEX), 0);
	v = (D3DTLVERTEX*)targetMeshP->aVtx;	//makes no sense otherwise

	for (int i = 0; i < targetMeshP->nVerts; i++)
	{
		v[i].sx = (p[i].x * 80.0F) / 96.0F;
		v[i].sy = (p[i].y * 60.0F) / 224.0F;
		v[i].sz = 0;
		v[i].rhw = f_mpersp / f_mznear * f_moneopersp;
		v[i].color = 0xFF000000;
		v[i].specular = 0xFF000000;
	}
}

void InitBinoculars()
{
	OBJECT_INFO* obj;
	ACMESHVERTEX* p;
	D3DTLVERTEX* v;

	obj = &objects[BINOCULAR_GRAPHICS];

	if (!obj->loaded)
		return;

	binocsMeshP = (MESH_DATA*)meshes[obj->mesh_index];
	p = binocsMeshP->aVtx;
	binocsMeshP->aVtx = (ACMESHVERTEX*)game_malloc(binocsMeshP->nVerts * sizeof(ACMESHVERTEX), 0);
	v = (D3DTLVERTEX*)binocsMeshP->aVtx;	//makes no sense otherwise

	for (int i = 0; i < binocsMeshP->nVerts; i++)
	{
		v[i].sx = (p[i].x * 32.0F) / 96.0F;
		v[i].sy = (p[i].y * 30.0F) / 224.0F;
		v[i].sz = 0;
		v[i].rhw = f_mpersp / f_mznear * f_moneopersp;
		v[i].color = 0xFF000000;
		v[i].specular = 0xFF000000;
	}
}

void SuperDrawBox(long* box)
{
	D3DVECTOR bounds[8];
	D3DTLVERTEX v[32];

	bounds[0].x = (float)box[0];
	bounds[0].y = (float)box[2] + 32;
	bounds[0].z = (float)box[4];

	bounds[1].x = (float)box[1];
	bounds[1].y = (float)box[2] + 32;
	bounds[1].z = (float)box[4];

	bounds[2].x = (float)box[0];
	bounds[2].y = (float)box[2] + 32;
	bounds[2].z = (float)box[5];

	bounds[3].x = (float)box[1];
	bounds[3].y = (float)box[2] + 32;
	bounds[3].z = (float)box[5];

	bounds[4].x = (float)box[0];
	bounds[4].y = (float)box[3] + 32;
	bounds[4].z = (float)box[4];

	bounds[5].x = (float)box[1];
	bounds[5].y = (float)box[3] + 32;
	bounds[5].z = (float)box[4];

	bounds[6].x = (float)box[0];
	bounds[6].y = (float)box[3] + 32;
	bounds[6].z = (float)box[5];

	bounds[7].x = (float)box[1];
	bounds[7].y = (float)box[3] + 32;
	bounds[7].z = (float)box[5];

	aTransformClip_D3DV(bounds, &v[0], 8, 0);
	aTransformClip_D3DV(bounds, &v[8], 8, 8);
	aTransformClip_D3DV(bounds, &v[16], 8, 16);
	aTransformClip_D3DV(bounds, &v[24], 8, 24);

	for (int i = 0; i < 32; i++)
	{
		v[i].rhw = f_mpersp / f_mznear * f_moneopersp;
		v[i].color = 0xFFFF0000;
		v[i].specular = 0xFF000000;
	}

	for (int i = 0; i < 4; i++)
	{
		v[i + 8].sx += 8.0F;
		v[i + 24].sx += 8.0F;
		v[i + 12].sx += 8.0F;
		v[i + 28].sx += 8.0F;
		AddQuadZBuffer(v, i, i + 8, i + 28, i + 20, textinfo, 1);
	}
}

void Draw2DSprite(long x, long y, long slot, long unused, long unused2)
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long x0, y0;

	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + slot];
	x0 = long(x + (sprite->width >> 8) * ((float)phd_centerx / 320.0F));
	y0 = long(y + 1 + (sprite->height >> 8) * ((float)phd_centery / 240.0F));
	setXY4(v, x, y, x0, y, x0, y0, x, y0, (long)f_mznear, clipflags);
	v[0].specular = 0xFF000000;
	v[1].specular = 0xFF000000;
	v[2].specular = 0xFF000000;
	v[3].specular = 0xFF000000;
	v[0].color = 0xFFFFFFFF;
	v[1].color = 0xFFFFFFFF;
	v[2].color = 0xFFFFFFFF;
	v[3].color = 0xFFFFFFFF;
	tex.drawtype = 1;
	tex.flag = 0;
	tex.tpage = sprite->tpage;
	tex.u1 = sprite->x1;
	tex.v1 = sprite->y1;
	tex.u2 = sprite->x2;
	tex.v2 = sprite->y1;
	tex.u3 = sprite->x2;
	tex.v3 = sprite->y2;
	tex.u4 = sprite->x1;
	tex.v4 = sprite->y2;
	AddQuadClippedSorted(v, 0, 1, 2, 3, &tex, 0);
}

void DrawBikeSpeedo(long ux, long uy, long vel, long maxVel, long turboVel, long size, long unk)
{
	D3DTLVERTEX v[2];
	float x, y, x0, y0, x1, y1;
	long rSize, rVel, rMVel, rTVel, angle;

	x = (float)phd_winxmax / 512.0F * 448.0F;
	y = (float)phd_winymax / 240.0F * 224.0F;
	rSize = (7 * size) >> 3;
	rVel = abs(vel >> 1);

	if (rVel)
	{
		rVel += (((rVel - 4096) >> 5) * phd_sin((GlobalCounter & 7) << 13)) >> 14;

		if (rVel < 0)
			rVel = 0;
	}

	rMVel = maxVel >> 1;
	rTVel = turboVel >> 1;
	angle = -0x4000;

	for (int i = 0; i <= rTVel; i += 2048)
	{
		x0 = ((rSize * (phd_sin(angle + i)) >> 13) - ((rSize * phd_sin(angle + i)) >> 15)) * ((float)phd_winxmax / 512.0F);
		y0 = (-(rSize * phd_cos(angle + i)) >> 14) * (float)phd_winymax / 240.0F;
		x1 = ((size * (phd_sin(angle + i)) >> 13) - ((size * phd_sin(angle + i)) >> 15)) * ((float)phd_winxmax / 512.0F);
		y1 = (-(size * phd_cos(angle + i)) >> 14) * (float)phd_winymax / 240.0F;

		v[0].sx = x + x0;
		v[0].sy = y + y0;
		v[0].sz = f_mznear;
		v[0].rhw = f_moneoznear;

		v[1].sx = x + x1;
		v[1].sy = y + y1;
		v[1].sz = f_mznear;
		v[1].rhw = f_moneoznear;

		if (i > rMVel)
		{
			v[0].color = 0xFFFF0000;
			v[1].color = 0xFFFF0000;
		}
		else
		{
			v[0].color = 0xFFFFFFFF;
			v[1].color = 0xFFFFFFFF;
		}

		v[1].specular = v[0].specular;
		AddLineSorted(v, &v[1], 6);
	}

	size -= size >> 4;
	x0 = ((-4 * (phd_sin(angle + rVel)) >> 13) - ((-4 * phd_sin(angle + rVel)) >> 15)) * ((float)phd_winxmax / 512.0F);
	y0 = (-(-4 * phd_cos(angle + rVel)) >> 14) * (float)phd_winymax / 240.0F;
	x1 = ((size * (phd_sin(angle + rVel)) >> 13) - ((size * phd_sin(angle + rVel)) >> 15)) * ((float)phd_winxmax / 512.0F);
	y1 = (-(size * phd_cos(angle + rVel)) >> 14) * (float)phd_winymax / 240.0F;

	v[0].sx = x + x0;
	v[0].sy = y + y0;
	v[0].sz = f_mznear;
	v[0].rhw = f_moneoznear;

	v[1].sx = x + x1;
	v[1].sy = y + y1;
	v[1].sz = f_mznear;
	v[1].rhw = f_moneoznear;

	v[1].color = v[0].color;
	v[1].specular = v[0].specular;
	AddLineSorted(v, &v[1], 6);
}

void DrawShockwaves()
{
	SHOCKWAVE_STRUCT* wave;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX vtx[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR p1, p2, p3;
	long* Z;
	short* XY;
	short* offsets;
	long v, x1, y1, x2, y2, x3, y3, x4, y4, r, g, b, c;
	short rad;

	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 8];
	offsets = (short*)&scratchpad[768];

	for (int i = 0; i < 16; i++)
	{
		wave = &ShockWaves[i];

		if (!wave->life)
			continue;

		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];
		phd_PushMatrix();
		phd_TranslateAbs(wave->x, wave->y, wave->z);
		phd_RotX(wave->XRot);
		offsets[1] = 0;
		offsets[5] = 0;
		offsets[9] = 0;
		rad = wave->OuterRad;

		for (int j = 0; j < 2; j++)
		{
			offsets[0] = (rad * phd_sin(0)) >> 14;
			offsets[2] = (rad * phd_cos(0)) >> 14;
			offsets[4] = (rad * phd_sin(0x1000)) >> 14;
			offsets[6] = (rad * phd_cos(0x1000)) >> 14;
			offsets[8] = (rad * phd_sin(0x2000)) >> 14;
			offsets[10] = (rad * phd_cos(0x2000)) >> 14;

			for (int k = 1; k < 7; k++)
			{
				v = k * 0x3000;

				p1.x = (offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
				p1.y = (offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
				p1.z = (offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

				p2.x = (offsets[4] * phd_mxptr[M00] + offsets[5] * phd_mxptr[M01] + offsets[6] * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
				p2.y = (offsets[4] * phd_mxptr[M10] + offsets[5] * phd_mxptr[M11] + offsets[6] * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
				p2.z = (offsets[4] * phd_mxptr[M20] + offsets[5] * phd_mxptr[M21] + offsets[6] * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

				p3.x = (offsets[8] * phd_mxptr[M00] + offsets[9] * phd_mxptr[M01] + offsets[10] * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
				p3.y = (offsets[8] * phd_mxptr[M10] + offsets[9] * phd_mxptr[M11] + offsets[10] * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
				p3.z = (offsets[8] * phd_mxptr[M20] + offsets[9] * phd_mxptr[M21] + offsets[10] * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

				offsets[0] = (rad * phd_sin(v)) >> 14;
				offsets[2] = (rad * phd_cos(v)) >> 14;
				offsets[4] = (rad * phd_sin(v + 0x1000)) >> 14;
				offsets[6] = (rad * phd_cos(v + 0x1000)) >> 14;
				offsets[8] = (rad * phd_sin(v + 0x2000)) >> 14;
				offsets[10] = (rad * phd_cos(v + 0x2000)) >> 14;

				XY[0] = (short)p1.x;
				XY[1] = (short)p1.y;
				Z[0] = p1.z;

				XY[2] = (short)p2.x;
				XY[3] = (short)p2.y;
				Z[1] = p2.z;

				XY[4] = (short)p3.x;
				XY[5] = (short)p3.y;
				Z[2] = p3.z;

				XY += 6;
				Z += 3;
			}

			rad = wave->InnerRad;
		}

		phd_PopMatrix();
		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		for (int j = 0; j < 16; j++)
		{
			x1 = XY[0];
			y1 = XY[1];
			x2 = XY[2];
			y2 = XY[3];
			x3 = XY[36];
			y3 = XY[37];
			x4 = XY[38];
			y4 = XY[39];
			setXYZ4(vtx, x1, y1, Z[0], x2, y2, Z[1], x4, y4, Z[19], x3, y3, Z[18], clipflags);

			r = wave->r;
			g = wave->g;
			b = wave->b;

			if (wave->life < 8)
			{
				r = (r * wave->life) >> 3;
				g = (g * wave->life) >> 3;
				b = (b * wave->life) >> 3;
			}

			c = RGBA(b, g, r, 0xFF);
			vtx[0].color = c;
			vtx[1].color = c;
			vtx[2].color = c;
			vtx[3].color = c;
			vtx[0].specular = 0xFF000000;
			vtx[1].specular = 0xFF000000;
			vtx[2].specular = 0xFF000000;
			vtx[3].specular = 0xFF000000;

			tex.drawtype = 2;
			tex.flag = 0;
			tex.tpage = sprite->tpage;
			tex.u1 = sprite->x1;
			tex.v1 = sprite->y2;
			tex.u2 = sprite->x2;
			tex.v2 = sprite->y2;
			tex.u3 = sprite->x2;
			tex.v3 = sprite->y1;
			tex.u4 = sprite->x1;
			tex.v4 = sprite->y1;
			AddQuadSorted(vtx, 0, 1, 2, 3, &tex, 1);

			XY += 2;
			Z++;
		}
	}
}

void DrawPsxTile(long x_y, long height_width, long color, long u0, long u1)
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float x, y, z, rhw, w, h;
	long col;
	ushort drawtype;

	nPolyType = 6;

	if ((color & 0xFF000000) == 0x62000000)
	{
		drawtype = 3;
		col = color << 24;
	}
	else
	{
		drawtype = 2;
		col = color | 0xFF000000;
	}

	if (!gfCurrentLevel)
	{
		z = f_znear + 10;
		rhw = f_moneoznear + 50;
	}
	else
	{
		z = f_znear;
		rhw = f_moneoznear;
	}

	x = float(x_y >> 16);
	y = float(x_y & 0xFFFF);
	w = float(height_width & 0xFFFF);
	h = float(height_width >> 16);

	v[0].sx = x;
	v[0].sy = y;
	v[0].sz = z;
	v[0].rhw = rhw;
	v[0].color = col;
	v[0].specular = 0xFF000000;

	v[1].sx = x + w + 1;
	v[1].sy = y;
	v[1].sz = z;
	v[1].rhw = rhw;
	v[1].color = col;
	v[1].specular = 0xFF000000;

	v[2].sx = x + w + 1;
	v[2].sy = y + h + 1;
	v[2].sz = z;
	v[2].rhw = rhw;
	v[2].color = col;
	v[2].specular = 0xFF000000;

	v[3].sx = x;
	v[3].sy = y + h + 1;
	v[3].sz = z;
	v[3].rhw = rhw;
	v[3].color = col;
	v[3].specular = 0xFF000000;

	tex.drawtype = drawtype;
	tex.flag = 0;
	tex.tpage = 0;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
}

void DrawRedTile(long x_y, long height_width)
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float x, y, z, rhw, w, h;

	nPolyType = 6;

	if (!gfCurrentLevel)
	{
		z = f_znear + 10;
		rhw = f_moneoznear + 50;
	}
	else
	{
		z = f_znear;
		rhw = f_moneoznear;
	}

	x = float(x_y >> 16);
	y = float(x_y & 0xFFFF);
	w = float(height_width & 0xFFFF);
	h = float(height_width >> 16);

	v[0].sx = x;
	v[0].sy = y;
	v[0].sz = z;
	v[0].rhw = rhw;
	v[0].color = 0xFFFF0000;
	v[0].specular = 0xFF000000;

	v[1].sx = x + w + 1;
	v[1].sy = y;
	v[1].sz = z;
	v[1].rhw = rhw;
	v[1].color = 0xFFFF0000;
	v[1].specular = 0xFF000000;

	v[2].sx = x + w + 1;
	v[2].sy = y + h + 1;
	v[2].sz = z;
	v[2].rhw = rhw;
	v[2].color = 0xFFFF0000;
	v[2].specular = 0xFF000000;

	v[3].sx = x;
	v[3].sy = y + h + 1;
	v[3].sz = z;
	v[3].rhw = rhw;
	v[3].color = 0xFFFF0000;
	v[3].specular = 0xFF000000;

	tex.drawtype = 0;
	tex.flag = 0;
	tex.tpage = 0;
	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
}

void DrawFlash()
{
	long r, g, b;

	r = ((FlashFadeR * FlashFader) >> 5) & 0xFF;
	g = ((FlashFadeG * FlashFader) >> 5) & 0xFF;
	b = ((FlashFadeB * FlashFader) >> 5) & 0xFF;
	DrawPsxTile(0, phd_winwidth | (phd_winheight << 16), RGBA(r, g, b, 0x62), 1, 0);
	DrawPsxTile(0, phd_winwidth | (phd_winheight << 16), RGBA(r, g, b, 0xFF), 2, 0);
}

void SuperShowLogo()
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long x, y, w;

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	nPolyType = 4;
	x = long(phd_winxmin - float((phd_winxmax / 640.0F) * -64));
	w = long(float((phd_winxmax / 640.0F) * 256));
	y = long(phd_winymin + float((phd_winymax / 480.0F) * 256));

	v[0].sx = (float)x;
	v[0].sy = (float)phd_winymin;
	v[0].sz = 0;
	v[0].rhw = f_moneoznear;
	v[0].color = 0xFFFFFFFF;
	v[0].specular = 0xFF000000;

	v[1].sx = float(w + x);
	v[1].sy = (float)phd_winymin;
	v[1].sz = 0;
	v[1].rhw = f_moneoznear;
	v[1].color = 0xFFFFFFFF;
	v[1].specular = 0xFF000000;

	v[2].sx = float(w + x);
	v[2].sy = (float)y;
	v[2].sz = 0;
	v[2].rhw = f_moneoznear;
	v[2].color = 0xFFFFFFFF;
	v[2].specular = 0xFF000000;

	v[3].sx = (float)x;
	v[3].sy = (float)y;
	v[3].sz = 0;
	v[3].rhw = f_moneoznear;
	v[3].color = 0xFFFFFFFF;
	v[3].specular = 0xFF000000;

	tex.drawtype = 1;
	tex.flag = 0;
	tex.tpage = ushort(nTextures - 5);
	tex.u1 = 0.00390625F;
	tex.v1 = 0.00390625F;
	tex.u2 = 0.99609375F;
	tex.v2 = 0.00390625F;
	tex.u3 = 0.99609375F;
	tex.v3 = 0.99609375F;
	tex.u4 = 0.00390625F;
	tex.v4 = 0.99609375F;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

	v[0].sx = float(w + x);
	v[0].sy = (float)phd_winymin;
	v[0].sz = 0;
	v[0].rhw = f_moneoznear;
	v[0].color = 0xFFFFFFFF;
	v[0].specular = 0xFF000000;

	v[1].sx = float(2 * w + x);
	v[1].sy = (float)phd_winymin;
	v[1].sz = 0;
	v[1].rhw = f_moneoznear;
	v[1].color = 0xFFFFFFFF;
	v[1].specular = 0xFF000000;

	v[2].sx = float(2 * w + x);
	v[2].sy = (float)y;
	v[2].sz = 0;
	v[2].rhw = f_moneoznear;
	v[2].color = 0xFFFFFFFF;
	v[2].specular = 0xFF000000;

	v[3].sx = float(w + x);
	v[3].sy = (float)y;
	v[3].sz = 0;
	v[3].rhw = f_moneoznear;
	v[3].color = 0xFFFFFFFF;
	v[3].specular = 0xFF000000;

	tex.drawtype = 1;
	tex.flag = 0;
	tex.tpage = ushort(nTextures - 4);
	tex.u1 = 0.00390625F;
	tex.v1 = 0.00390625F;
	tex.u2 = 0.99609375F;
	tex.v2 = 0.00390625F;
	tex.u3 = 0.99609375F;
	tex.v3 = 0.99609375F;
	tex.u4 = 0.00390625F;
	tex.v4 = 0.99609375F;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 1);
}

void DrawDebris()
{
	DEBRIS_STRUCT* dptr;
	TEXTURESTRUCT* tex;
	D3DTLVERTEX v[3];
	long* Z;
	short* XY;
	short* offsets;
	long r, g, b, c;
	ushort drawbak;

	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];

	for (int i = 0; i < 256; i++)
	{
		dptr = &debris[i];

		if (!dptr->On)
			continue;

		phd_PushMatrix();
		phd_TranslateAbs(dptr->x, dptr->y, dptr->z);
		phd_RotY(dptr->YRot << 8);
		phd_RotX(dptr->XRot << 8);

		offsets[0] = dptr->XYZOffsets1[0];
		offsets[1] = dptr->XYZOffsets1[1];
		offsets[2] = dptr->XYZOffsets1[2];
		XY[0] = short((phd_mxptr[M03] + phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2]) >> 14);
		XY[1] = short((phd_mxptr[M13] + phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2]) >> 14);
		Z[0] = (phd_mxptr[M23] + phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2]) >> 14;

		offsets[0] = dptr->XYZOffsets2[0];
		offsets[1] = dptr->XYZOffsets2[1];
		offsets[2] = dptr->XYZOffsets2[2];
		XY[2] = short((phd_mxptr[M03] + phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2]) >> 14);
		XY[3] = short((phd_mxptr[M13] + phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2]) >> 14);
		Z[1] = (phd_mxptr[M23] + phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2]) >> 14;

		offsets[0] = dptr->XYZOffsets3[0];
		offsets[1] = dptr->XYZOffsets3[1];
		offsets[2] = dptr->XYZOffsets3[2];
		XY[4] = short((phd_mxptr[M03] + phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2]) >> 14);
		XY[5] = short((phd_mxptr[M13] + phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2]) >> 14);
		Z[2] = (phd_mxptr[M23] + phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2]) >> 14;

		setXYZ3(v, XY[0], XY[1], Z[0], XY[2], XY[3], Z[1], XY[4], XY[5], Z[2], clipflags);
		phd_PopMatrix();

		c = dptr->color1 & 0xFF;
		r = ((c * dptr->r) >> 8) + CLRR(dptr->ambient);
		g = ((c * dptr->g) >> 8) + CLRG(dptr->ambient);
		b = ((c * dptr->b) >> 8) + CLRB(dptr->ambient);

		if (r > 255)
			r = 255;

		if (g > 255)
			g = 255;

		if (b > 255)
			b = 255;

		c = RGBONLY(r, g, b);
		CalcColorSplit(c, &v[0].color);

		c = dptr->color2 & 0xFF;
		r = ((c * dptr->r) >> 8) + CLRR(dptr->ambient);
		g = ((c * dptr->g) >> 8) + CLRG(dptr->ambient);
		b = ((c * dptr->b) >> 8) + CLRB(dptr->ambient);

		if (r > 255)
			r = 255;

		if (g > 255)
			g = 255;

		if (b > 255)
			b = 255;

		c = RGBONLY(r, g, b);
		CalcColorSplit(c, &v[1].color);

		c = dptr->color3 & 0xFF;
		r = ((c * dptr->r) >> 8) + CLRR(dptr->ambient);
		g = ((c * dptr->g) >> 8) + CLRG(dptr->ambient);
		b = ((c * dptr->b) >> 8) + CLRB(dptr->ambient);

		if (r > 255)
			r = 255;

		if (g > 255)
			g = 255;

		if (b > 255)
			b = 255;

		c = RGBONLY(r, g, b);
		CalcColorSplit(c, &v[2].color);

		v[0].color |= 0xFF000000;
		v[1].color |= 0xFF000000;
		v[2].color |= 0xFF000000;
		v[0].specular |= 0xFF000000;
		v[1].specular |= 0xFF000000;
		v[2].specular |= 0xFF000000;

		tex = &textinfo[(long)dptr->TextInfo & 0x7FFF];
		drawbak = tex->drawtype;

		if (dptr->flags & 1)
			tex->drawtype = 2;

		if (!tex->drawtype)
			AddTriZBuffer(v, 0, 1, 2, tex, 1);
		else if (tex->drawtype <= 2)
			AddTriSorted(v, 0, 1, 2, tex, 1);

		tex->drawtype = drawbak;
	}
}

void DrawBlood()
{
	BLOOD_STRUCT* bptr;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	ulong r, col;
	long size, s, c;
	long dx, dy, dz, x1, y1, x2, y2, x3, y3, x4, y4;
	short ang;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 15];
	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];

	for (int i = 0; i < 32; i++)
	{
		bptr = &blood[i];

		if (!bptr->On)
			continue;

		dx = bptr->x - lara_item->pos.x_pos;
		dy = bptr->y - lara_item->pos.y_pos;
		dz = bptr->z - lara_item->pos.z_pos;

		if (dx < -0x5000 || dx > 0x5000 || dy < -0x5000 || dy > 0x5000 || dz < -0x5000 || dz > 0x5000)
			continue;

		offsets[0] = (short)dx;
		offsets[1] = (short)dy;
		offsets[2] = (short)dz;
		pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
		perspz = f_persp / (float)pos.z;
		XY[0] = short(float(pos.x * perspz + f_centerx));
		XY[1] = short(float(pos.y * perspz + f_centery));
		Z[0] = pos.z >> 14;

		if (Z[0] <= 0 || Z[0] >= 0x5000)
			continue;

		size = ((phd_persp * bptr->Size) << 1) / Z[0];

		if (size > (bptr->Size << 1))
			size = (bptr->Size << 1);
		else if (size < 4)
			size = 4;

		size <<= 1;
		ang = bptr->RotAng << 1;
		s = (size * rcossin_tbl[ang]) >> 12;
		c = (size * rcossin_tbl[ang + 1]) >> 12;
		x1 = c + XY[0] - s;
		y1 = XY[1] - c - s;
		x2 = s + c + XY[0];
		y2 = c + XY[1] - s;
		x3 = s + XY[0] - c;
		y3 = s + XY[1] + c;
		x4 = XY[0] - c - s;
		y4 = XY[1] - c + s;
		setXY4(v, x1, y1, x2, y2, x3, y3, x4, y4, Z[0], clipflags);

		if (Z[0] <= 0x3000)
			col = RGBA(bptr->Shade, 0, 0, 0xFF);
		else
		{
			r = ((0x5000 - Z[0]) * bptr->Shade) >> 13;
			col = RGBA(r, 0, 0, 0xFF);
		}

		v[0].color = col;
		v[1].color = col;
		v[2].color = col;
		v[3].color = col;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v2 = sprite->y1;
		tex.u3 = sprite->x2;
		tex.v3 = sprite->y2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
	}

	phd_PopMatrix();
}

void DrawDrips()
{
	DRIP_STRUCT* drip;
	PHD_VECTOR vec;
	D3DTLVERTEX v[3];
	long* Z;
	short* XY;
	short* pos;
	float perspz;
	long x0, y0, z0, x1, y1, z1, r, g, b;

	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	pos = (short*)&scratchpad[512];

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

	for (int i = 0; i < 32; i++)
	{
		drip = &Drips[i];

		if (!drip->On)
			continue;

		pos[0] = short(drip->x - lara_item->pos.x_pos);
		pos[1] = short(drip->y - lara_item->pos.y_pos);
		pos[2] = short(drip->z - lara_item->pos.z_pos);

		if (pos[0] < -0x5000 || pos[0] > 0x5000 || pos[1] < -0x5000 || pos[1] > 0x5000 || pos[2] < -0x5000 || pos[2] > 0x5000)
			continue;

		vec.x = pos[0] * phd_mxptr[M00] + pos[1] * phd_mxptr[M01] + pos[2] * phd_mxptr[M02] + phd_mxptr[M03];
		vec.y = pos[0] * phd_mxptr[M10] + pos[1] * phd_mxptr[M11] + pos[2] * phd_mxptr[M12] + phd_mxptr[M13];
		vec.z = pos[0] * phd_mxptr[M20] + pos[1] * phd_mxptr[M21] + pos[2] * phd_mxptr[M22] + phd_mxptr[M23];

		perspz = f_persp / (float)vec.z;
		XY[0] = short(float(vec.x * perspz + f_centerx));
		XY[1] = short(float(vec.y * perspz + f_centery));
		Z[0] = vec.z >> 14;

		pos[1] -= drip->Yvel >> 6;

		if (room[drip->RoomNumber].flags & ROOM_NOT_INSIDE)
		{
			pos[0] -= short(SmokeWindX >> 1);
			pos[1] -= short(SmokeWindZ >> 1);
		}

		vec.x = pos[0] * phd_mxptr[M00] + pos[1] * phd_mxptr[M01] + pos[2] * phd_mxptr[M02] + phd_mxptr[M03];
		vec.y = pos[0] * phd_mxptr[M10] + pos[1] * phd_mxptr[M11] + pos[2] * phd_mxptr[M12] + phd_mxptr[M13];
		vec.z = pos[0] * phd_mxptr[M20] + pos[1] * phd_mxptr[M21] + pos[2] * phd_mxptr[M22] + phd_mxptr[M23];

		perspz = f_persp / (float)vec.z;
		XY[2] = short(float(vec.x * perspz + f_centerx));
		XY[3] = short(float(vec.y * perspz + f_centery));
		Z[1] = vec.z >> 14;

		if (!Z[0])
			continue;

		if (Z[0] > 0x5000)
		{
			drip->On = 0;
			continue;
		}

		x0 = XY[0];
		y0 = XY[1];
		z0 = Z[0];
		x1 = XY[2];
		y1 = XY[3];
		z1 = Z[1];

		if (ClipLine(x0, y0, z0, x1, y1, z1, phd_winxmin, phd_winymin, phd_winxmax, phd_winymax))
		{
			r = drip->R << 2;
			g = drip->G << 2;
			b = drip->B << 2;

			v[0].sx = (float)x0;
			v[0].sy = (float)y0;
			v[0].sz = (float)z0;
			v[0].rhw = f_mpersp / v[0].sz * f_moneopersp;
			v[0].color = RGBA(r, g, b, 0xFF);
			v[0].specular = 0xFF000000;

			r >>= 1;
			g >>= 1;
			b >>= 1;

			v[1].sx = (float)x1;
			v[1].sy = (float)y1;
			v[1].sz = (float)z1;
			v[1].rhw = f_mpersp / v[1].sz * f_moneopersp;
			v[1].color = RGBA(r, g, b, 0xFF);
			v[1].specular = 0xFF000000;

			AddLineSorted(v, &v[1], 6);
		}
	}

	phd_PopMatrix();
}

void DoUwEffect()
{
	UWEFFECTS* p;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[3];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	long num_alive, rad, ang, x, y, z, size, col, yv;

	num_alive = 0;

	for (int i = 0; i < 256; i++)
	{
		p = &uwdust[i];

		if (!p->x && num_alive < 16)
		{
			num_alive++;
			rad = GetRandomDraw() & 0xFFF;
			ang = GetRandomDraw() & 0x1FFE;
			x = (rad * rcossin_tbl[ang]) >> 12;
			y = (GetRandomDraw() & 0x7FF) - 1024;
			z = (rad * rcossin_tbl[ang + 1]) >> 12;
			p->x = lara_item->pos.x_pos + x;
			p->y = lara_item->pos.y_pos + y;
			p->z = lara_item->pos.z_pos + z;

			if (IsRoomOutside(p->x, p->y, p->z) < 0 || !(room[IsRoomOutsideNo].flags & ROOM_UNDERWATER))
			{
				p->x = 0;
				continue;
			}

			p->stopped = 1;
			p->life = (GetRandomDraw() & 7) + 16;
			p->xv = GetRandomDraw() & 3;

			if (p->xv == 2)
				p->xv = -1;

			p->yv = ((GetRandomDraw() & 7) + 8) << 3;
			p->zv = GetRandomDraw() & 3;

			if (p->zv == 2)
				p->zv = -1;
		}

		p->x += p->xv;
		p->y += (p->yv & ~7) >> 6;
		p->z += p->zv;

		if (!p->life)
		{
			p->x = 0;
			continue;
		}

		p->life--;

		if ((p->yv & 7) < 7)
			p->yv++;
	}

	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 15];
	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

	for (int i = 0; i < 256; i++)
	{
		p = &uwdust[i];

		if (!p->x)
			continue;

		x = p->x - lara_item->pos.x_pos;
		y = p->y - lara_item->pos.y_pos;
		z = p->z - lara_item->pos.z_pos;
		offsets[0] = (short)x;
		offsets[1] = (short)y;
		offsets[2] = (short)z;
		pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
		perspz = f_persp / (float)pos.z;
		XY[0] = short(float(pos.x * perspz + f_centerx));
		XY[1] = short(float(pos.y * perspz + f_centery));
		Z[0] = pos.z >> 14;

		if (Z[0] < 32)
		{
			if (p->life > 16)
				p->life = 16;

			continue;
		}

		if (XY[0] < phd_winxmin || XY[0] > phd_winxmax || XY[1] < phd_winymin || XY[1] > phd_winymax)
			continue;

		size = (phd_persp * (p->yv >> 3)) / (Z[0] >> 4);

		if (size < 1)
			size = 6;
		else if (size > 12)
			size = 12;

		size = (0x5556 * size) >> 16;

		if (phd_winwidth > 512)
			size = long(float(phd_winwidth / 512.0F) * (float)size);

		if ((p->yv & 7) == 7)
		{
			if (p->life > 18)
				col = 0xFF404040;
			else
				col = (p->life | ((p->life | ((p->life | 0xFFFFFFC0) << 8)) << 8)) << 2;	//decipher me
		}
		else
		{
			yv = (p->yv & 7) << 2;
			col = (yv | ((yv | ((yv | 0xFFFFFF80) << 8)) << 8)) << 1;	//decipher me
		}

		setXY3(v, XY[0] + size, XY[1] - (size << 1), XY[0] + size, XY[1] + size, XY[0] - (size << 1), XY[1] + size, Z[0], clipflags);
		v[0].color = col;
		v[1].color = col;
		v[2].color = col;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x2;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v2 = sprite->y2;
		tex.u3 = sprite->x1;
		tex.v3 = sprite->y2;
		AddTriSorted(v, 0, 1, 2, &tex, 0);
	}

	phd_PopMatrix();
}

void DrawWraithTrail(ITEM_INFO* item)
{
	WRAITH_STRUCT* wraith;
	PHD_VECTOR pos;
	D3DTLVERTEX v[2];
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	ulong r, g, b;
	long c0, c1, x0, y0, z0, x1, y1, z1;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	for (int i = 0; i < 5; i++)
	{
		if (!i)
			phd_RotY(-1092);
		else if (i == 2)
			phd_RotY(1092);
		else if (i == 3)
			phd_RotZ(-1092);
		else if (i == 4)
			phd_RotZ(1092);

		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];
		offsets = (short*)&scratchpad[512];
		wraith = (WRAITH_STRUCT*)item->data;

		for (int j = 0; j < 8; j++, XY += 2, Z += 2, wraith++)
		{
			offsets[0] = short(wraith->pos.x - item->pos.x_pos);
			offsets[1] = short(wraith->pos.y - item->pos.y_pos);
			offsets[2] = short(wraith->pos.z - item->pos.z_pos);
			pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
			pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
			pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
			perspz = f_persp / (float)pos.z;
			XY[0] = short(float(pos.x * perspz + f_centerx));
			XY[1] = short(float(pos.y * perspz + f_centery));
			Z[0] = pos.z >> 14;

			if (!j || j == 7)
				Z[1] = 0;
			else
				Z[1] = RGBONLY(wraith->r, wraith->g, wraith->b);
		}

		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		for (int j = 0; j < 7; j++, XY += 2, Z += 2)
		{
			if (Z[0] <= f_mznear || Z[0] >= 0x5000)
				continue;

			if (Z[0] > 0x3000)
			{
				r = ((Z[1] & 0xFF) * (0x3000 - Z[0])) >> 13;
				g = (((Z[1] >> 8) & 0xFF) * (0x3000 - Z[0])) >> 13;
				b = (((Z[1] >> 16) & 0xFF) * (0x3000 - Z[0])) >> 13;
				c0 = RGBA(r, g, b, 0xFF);
				r = ((Z[3] & 0xFF) * (0x3000 - Z[0])) >> 13;
				g = (((Z[3] >> 8) & 0xFF) * (0x3000 - Z[0])) >> 13;
				b = (((Z[3] >> 16) & 0xFF) * (0x3000 - Z[0])) >> 13;
				c1 = RGBA(r, g, b, 0xFF);
			}
			else
			{
				c0 = Z[1];
				c1 = Z[3];
			}

			x0 = XY[0];
			y0 = XY[1];
			z0 = Z[0];
			x1 = XY[2];
			y1 = XY[3];
			z1 = Z[2];

			if (ClipLine(x0, y0, z0, x1, y1, z1, phd_winxmin, phd_winymin, phd_winxmax, phd_winymax))
			{
				v[0].sx = (float)x0;
				v[0].sy = (float)y0;
				v[0].sz = (float)z0;
				v[0].rhw = f_mpersp / v[0].sz * f_moneopersp;
				v[0].color = c0;
				v[0].specular = 0xFF000000;

				v[1].sx = (float)x1;
				v[1].sy = (float)y1;
				v[1].sz = (float)z1;
				v[1].rhw = f_mpersp / v[1].sz * f_moneopersp;
				v[1].color = c1;
				v[1].specular = 0xFF000000;

				AddLineSorted(v, &v[1], 6);
			}
		}
	}

	phd_PopMatrix();
}

void DrawTrainFloorStrip(long x, long z, TEXTURESTRUCT* tex, long y_and_flags)
{
	SVECTOR* offsets;
	D3DTLVERTEX v[4];
	PHD_VECTOR p1, p2, p3;
	long* Z;
	short* XY;
	long num, z1, z2, z3, z4, spec;
	short x1, y1, x2, y2, x3, y3, x4, y4;

	num = 0;
	offsets = (SVECTOR*)&scratchpad[984];
	offsets[0].z = (short)z;
	offsets[1].z = short(z + 512);
	offsets[2].z = short(z + 1024);

	if (y_and_flags & 0x1000000)
	{
		offsets[1].z += 1024;
		offsets[2].z += 2048;
	}

	offsets[0].y = ((y_and_flags >> 16) & 0xFF) << 4;
	offsets[1].y = ((y_and_flags >> 8) & 0xFF) << 4;
	offsets[2].y = (y_and_flags & 0xFF) << 4;

	offsets[0].x = (short)x;
	offsets[1].x = (short)x;
	offsets[2].x = (short)x;

	for (int i = 0; i < 2; i++)
	{
		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[492];
		XY -= 6;
		Z -= 3;

		for (int j = 0; j < 41; j++)
		{
			p1.x = (offsets[0].x * phd_mxptr[M00] + offsets[0].y * phd_mxptr[M01] + offsets[0].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p1.y = (offsets[0].x * phd_mxptr[M10] + offsets[0].y * phd_mxptr[M11] + offsets[0].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p1.z = (offsets[0].x * phd_mxptr[M20] + offsets[0].y * phd_mxptr[M21] + offsets[0].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			p2.x = (offsets[1].x * phd_mxptr[M00] + offsets[1].y * phd_mxptr[M01] + offsets[1].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p2.y = (offsets[1].x * phd_mxptr[M10] + offsets[1].y * phd_mxptr[M11] + offsets[1].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p2.z = (offsets[1].x * phd_mxptr[M20] + offsets[1].y * phd_mxptr[M21] + offsets[1].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			p3.x = (offsets[2].x * phd_mxptr[M00] + offsets[2].y * phd_mxptr[M01] + offsets[2].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p3.y = (offsets[2].x * phd_mxptr[M10] + offsets[2].y * phd_mxptr[M11] + offsets[2].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p3.z = (offsets[2].x * phd_mxptr[M20] + offsets[2].y * phd_mxptr[M21] + offsets[2].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			offsets[0].x += 512;
			offsets[1].x += 512;
			offsets[2].x += 512;
			XY += 6;
			Z += 3;

			XY[0] = (short)p1.x;
			XY[1] = (short)p1.y;
			Z[0] = p1.z;

			XY[2] = (short)p2.x;
			XY[3] = (short)p2.y;
			Z[1] = p2.z;

			XY[4] = (short)p3.x;
			XY[5] = (short)p3.y;
			Z[2] = p3.z;
		}

		offsets[0].x -= 512;
		offsets[1].x -= 512;
		offsets[2].x -= 512;
		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[492];

		for (int j = num; j < num + 20; j++, XY += 12, Z += 6)
		{
			z1 = Z[0];
			z2 = Z[2];
			z3 = Z[6];
			z4 = Z[8];

			if (!(z1 | z2 | z3 | z4))
				continue;

			x1 = XY[0];
			y1 = XY[1];
			x2 = XY[4];
			y2 = XY[5];
			x3 = XY[12];
			y3 = XY[13];
			x4 = XY[16];
			y4 = XY[17];

			if (j < 7)
				spec = (j + 1) * 0x101010;
			else if (j >= 33)
				spec = (40 - j) * 0x101010;
			else
				spec = 0x808080;

			setXYZ4(v, x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3, clipflags);
			spec = ((spec & 0xFF) - 1) << 25;
			v[0].color = 0xFFFFFFFF;
			v[1].color = 0xFFFFFFFF;
			v[2].color = 0xFFFFFFFF;
			v[3].color = 0xFFFFFFFF;
			v[0].specular = spec;
			v[1].specular = spec;
			v[2].specular = spec;
			v[3].specular = spec;
			AddQuadSorted(v, 0, 1, 2, 3, tex, 0);
		}

		num += 20;
	}
}

void DrawTrainStrips()
{
	DrawTrainFloorStrip(-20480, -5120, &textinfo[aranges[7]], 0x1101010);
	DrawTrainFloorStrip(-20480, 3072, &textinfo[aranges[7]], 0x1101010);
	DrawTrainFloorStrip(-20480, -2048, &textinfo[aranges[5]], 0x100800);
	DrawTrainFloorStrip(-20480, 2048, &textinfo[aranges[6]], 0x810);
	DrawTrainFloorStrip(-20480, -1024, &textinfo[aranges[3]], 0);
	DrawTrainFloorStrip(-20480, 1024, &textinfo[aranges[4]], 0);
	DrawTrainFloorStrip(-20480, 0, &textinfo[aranges[2]], 0);
}

void DrawBubbles()
{
	BUBBLE_STRUCT* bubble;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	long dx, dy, dz, size, x1, y1, x2, y2;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	bubble = Bubbles;

	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];

	for (int i = 0; i < 40; i++)
	{
		if (!bubble->size)
		{
			bubble++;
			continue;
		}

		dx = bubble->pos.x - lara_item->pos.x_pos;
		dy = bubble->pos.y - lara_item->pos.y_pos;
		dz = bubble->pos.z - lara_item->pos.z_pos;

		if (dx < -0x5000 || dx > 0x5000 || dy < -0x5000 || dy > 0x5000 || dz < -0x5000 || dz > 0x5000)
		{
			bubble->size = 0;
			bubble++;
			continue;
		}

		offsets[0] = (short)dx;
		offsets[1] = (short)dy;
		offsets[2] = (short)dz;
		pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
		perspz = f_persp / (float)pos.z;
		XY[0] = short(float(pos.x * perspz + f_centerx));
		XY[1] = short(float(pos.y * perspz + f_centery));
		Z[0] = pos.z >> 14;

		if (Z[0] < 32)
		{
			bubble++;
			continue;
		}

		if (Z[0] > 0x5000)
		{
			bubble->size = 0;
			bubble++;
			continue;
		}

		size = phd_persp * (bubble->size >> 1) / Z[0];

		if (size > 128)
		{
			bubble->size = 0;
			continue;
		}

		if (size < 4)
			size = 4;

		size >>= 1;

		x1 = XY[0] - size;
		y1 = XY[1] - size;
		x2 = XY[0] + size;
		y2 = XY[1] + size;

		if (x2 < phd_winxmin || x1 >= phd_winxmax || y2 < phd_winymin || y1 >= phd_winymax)
		{
			bubble++;
			continue;
		}

		sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 13];
		setXY4(v, x1, y1, x2, y1, x2, y2, x1, y2, Z[0], clipflags);
		v[0].color = RGBA(bubble->shade, bubble->shade, bubble->shade, 0xFF);
		v[1].color = RGBA(bubble->shade, bubble->shade, bubble->shade, 0xFF);
		v[2].color = RGBA(bubble->shade, bubble->shade, bubble->shade, 0xFF);
		v[3].color = RGBA(bubble->shade, bubble->shade, bubble->shade, 0xFF);
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v3 = sprite->y2;
		tex.v2 = sprite->y1;
		tex.u3 = sprite->x2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
		bubble++;
	}

	phd_PopMatrix();
}

void DrawSprite(long x, long y, long slot, long col, long size, long z)
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long s;

	s = long(float(phd_winwidth / 640.0F) * (size << 1));

	if (z)
		setXY4(v, x - s, y - s, x + s, y - s, x - s, y + s, x + s, y + s, long(z + f_mznear), clipflags);
	else
		setXY4(v, x - s, y - s, x + s, y - s, x - s, y + s, x + s, y + s, (long)f_mzfar, clipflags);

	sprite = &spriteinfo[slot + objects[DEFAULT_SPRITES].mesh_index];
	v[0].specular = 0xFF000000;
	v[1].specular = 0xFF000000;
	v[2].specular = 0xFF000000;
	v[3].specular = 0xFF000000;
	CalcColorSplit(col, &v[0].color);
	v[1].color = v[0].color | 0xFF000000;
	v[2].color = v[0].color | 0xFF000000;
	v[3].color = v[0].color | 0xFF000000;
	tex.drawtype = 2;
	tex.flag = 0;
	tex.tpage = sprite->tpage;
	tex.u1 = sprite->x1;
	tex.v1 = sprite->y1;
	tex.u2 = sprite->x2;
	tex.v2 = sprite->y1;
	tex.u3 = sprite->x2;
	tex.v3 = sprite->y2;
	tex.u4 = sprite->x1;
	tex.v4 = sprite->y2;
	AddQuadSorted(v, 0, 1, 3, 2, &tex, 0);
}

void SetUpLensFlare(long x, long y, long z, GAME_VECTOR* lfobj)
{
	PHD_VECTOR pos;
	GAME_VECTOR start;
	GAME_VECTOR target;
	long* Z;
	short* vec;
	short* XY;
	float perspz;
	long dx, dy, dz, r, g, b, r2, g2, b2, los, num, flash;
	short rn;

	los = 0;

	if (lfobj)
	{
		pos.x = lfobj->x;
		pos.y = lfobj->y;
		pos.z = lfobj->z;
		dx = abs(pos.x - lara_item->pos.x_pos);
		dy = abs(pos.y - lara_item->pos.y_pos);
		dz = abs(pos.z - lara_item->pos.z_pos);

		if (dx > 0x8000 || dy > 0x8000 || dz > 0x8000)
			return;

		r = 255;
		g = 255;
		b = 255;
		rn = lfobj->room_number;
	}
	else
	{
		if (room[camera.pos.room_number].flags & ROOM_NO_LENSFLARE)
			return;

		r = (uchar)gfLensFlareColour.r;
		g = (uchar)gfLensFlareColour.g;
		b = (uchar)gfLensFlareColour.b;
		pos.x = x;
		pos.y = y;
		pos.z = z;

		while (abs(pos.x) > 0x36000 || abs(pos.y) > 0x36000 || abs(pos.z) > 0x36000)
		{
			pos.x -= (x - camera.pos.x) >> 4;
			pos.y -= (y - camera.pos.y) >> 4;
			pos.z -= (z - camera.pos.z) >> 4;
		}

		dx = (pos.x - camera.pos.x) >> 4;
		dy = (pos.y - camera.pos.y) >> 4;
		dz = (pos.z - camera.pos.z) >> 4;

		while (abs(pos.x - camera.pos.x) > 0x8000 || abs(pos.y - camera.pos.y) > 0x8000 || abs(pos.z - camera.pos.z) > 0x8000)
		{
			pos.x -= dx;
			pos.y -= dy;
			pos.z -= dz;
		}

		dx = (pos.x - camera.pos.x) >> 4;
		dy = (pos.y - camera.pos.y) >> 4;
		dz = (pos.z - camera.pos.z) >> 4;

		for (int i = 0; i < 16; i++)
		{
			IsRoomOutsideNo = 255;
			IsRoomOutside(pos.x, pos.y, pos.z);
			rn = IsRoomOutsideNo;

			if (rn != 255)
				break;

			pos.x -= dx;
			pos.y -= dy;
			pos.z -= dz;
		}
	}

	if (rn != 255)
	{
		if (room[rn].flags & ROOM_NOT_INSIDE || lfobj)
		{
			start.y = camera.pos.y;
			start.z = camera.pos.z;
			start.x = camera.pos.x;
			start.room_number = camera.pos.room_number;
			target.x = pos.x;
			target.y = pos.y;
			target.z = pos.z;
			los = LOS(&start, &target);
		}
	}

	if (!los && lfobj)	//can't see object, don't bother
		return;

	vec = (short*)&scratchpad[0];
	XY = (short*)&scratchpad[16];
	Z = (long*)&scratchpad[32];

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

	if (lfobj)
	{
		vec[0] = short(pos.x - lara_item->pos.x_pos);
		vec[1] = short(pos.y - lara_item->pos.y_pos);
		vec[2] = short(pos.z - lara_item->pos.z_pos);
	}
	else
	{
		pos.x = x - lara_item->pos.x_pos;
		pos.y = y - lara_item->pos.y_pos;
		pos.z = z - lara_item->pos.z_pos;

		while (pos.x > 0x7F00 || pos.x < -0x7F00 || pos.y > 0x7F00 || pos.y < -0x7F00 || pos.z > 0x7F00 || pos.z < -0x7F00)
		{
			pos.x >>= 1;
			pos.y >>= 1;
			pos.z >>= 1;
		}

		vec[0] = (short)pos.x;
		vec[1] = (short)pos.y;
		vec[2] = (short)pos.z;
	}

	pos.x = phd_mxptr[M00] * vec[0] + phd_mxptr[M01] * vec[1] + phd_mxptr[M02] * vec[2] + phd_mxptr[M03];
	pos.y = phd_mxptr[M10] * vec[0] + phd_mxptr[M11] * vec[1] + phd_mxptr[M12] * vec[2] + phd_mxptr[M13];
	pos.z = phd_mxptr[M20] * vec[0] + phd_mxptr[M21] * vec[1] + phd_mxptr[M22] * vec[2] + phd_mxptr[M23];
	perspz = f_persp / (float)pos.z;
	XY[0] = short(float(pos.x * perspz + f_centerx));
	XY[1] = short(float(pos.y * perspz + f_centery));
	Z[0] = pos.z >> 14;
	phd_PopMatrix();
	num = 0;

	if (lfobj)
		num += 6;

	if (Z[0] > 0)
	{
		dx = XY[0] - phd_centerx;
		dy = XY[1] - phd_centery;
		dz = phd_sqrt(SQUARE(dx) + SQUARE(dy));

		if (dz < 640)
		{
			dz = 640 - dz;

			if (los)
			{
				flash = dz - 544;

				if (flash > 0)
				{
					FlashFader = 32;
					FlashFadeR = short((r * flash) / 640);
					FlashFadeG = short((g * flash) / 640);
					FlashFadeB = short((b * flash) / 640);
				}
			}

			while (flare_table[num] != -1)
			{
				if (num)
				{
					r2 = dz * flare_table[num] / 640;
					g2 = dz * flare_table[num + 1] / 640;
					b2 = dz * flare_table[num + 2] / 640;
				}
				else
				{
					if (lfobj)
						continue;

					r2 = flare_table[0] + (GetRandomDraw() & 8);
					g2 = flare_table[1];
					b2 = flare_table[2] + (GetRandomDraw() & 8);
				}

				r2 = (r * r2) >> 8;
				g2 = (g * g2) >> 8;
				b2 = (b * b2) >> 8;

				if (r2 | g2 | b2)
				{
					pos.x = XY[0] - ((dx * flare_table[num + 4]) >> 4);
					pos.y = XY[1] - ((dy * flare_table[num + 4]) >> 4);
					DrawSprite(pos.x, pos.y, flare_table[num + 5], RGBONLY(r2, g2, b2), flare_table[num + 3] << 1, num);
				}

				if (!los)
					return;

				num += 6;
			}
		}
	}
}

bool ClipLine(long& x1, long& y1, long z1, long& x2, long& y2, long z2, long xMin, long yMin, long w, long h)
{
	float clip;

	if (z1 < 20 || z2 < 20)
		return 0;

	if (x1 < xMin && x2 < xMin || y1 < yMin && y2 < yMin)
		return 0;

	if (x1 > w && x2 > w || y1 > h && y2 > h)
		return 0;

	if (x1 > w)
	{
		clip = ((float)w - x2) / float(x1 - x2);
		x1 = w;
		y1 = long((y1 - y2) * clip + y2);
	}

	if (x2 > w)
	{
		clip = ((float)w - x1) / float(x2 - x1);
		x2 = w;
		y2 = long((y2 - y1) * clip + y1);
	}

	if (x1 < xMin)
	{
		clip = ((float)xMin - x1) / float(x2 - x1);
		x1 = xMin;
		y1 = long((y2 - y1) * clip + y1);
	}

	if (x2 < xMin)
	{
		clip = ((float)xMin - x2) / float(x1 - x2);
		x2 = xMin;
		y2 = long((y1 - y2) * clip + y2);
	}

	if (y1 > h)
	{
		clip = ((float)h - y2) / float(y1 - y2);
		y1 = h;
		x1 = long((x1 - x2) * clip + x2);
	}

	if (y2 > h)
	{
		clip = ((float)h - y1) / float(y2 - y1);
		y2 = h;
		x2 = long((x2 - x1) * clip + x1);
	}

	if (y1 < yMin)
	{
		clip = ((float)yMin - y1) / float(y2 - y1);
		y1 = yMin;
		x1 = long((x2 - x1) * clip + x1);
	}

	if (y2 < yMin)
	{
		clip = ((float)yMin - y2) / float(y1 - y2);
		y2 = yMin;
		x2 = long((x1 - x2) * clip + x2);
	}

	return 1;
}

void S_DrawSparks()
{
	SPARKS* sptr;
	FX_INFO* fx;
	ITEM_INFO* item;
	D3DTLVERTEX v[2];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	FVECTOR fpos;
	float fX, fY, fZ, zv;
	float p[8];
	long x, y, z, smallest_size, r, g, b, c0, c1;

	smallest_size = 0;	//uninitialized
	tex.drawtype = 2;
	tex.tpage = 0;
	tex.flag = 0;

	for (int i = 0; i < 16; i++)
		NodeOffsets[i].GotIt = 0;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	aSetViewMatrix();

	for (int i = 0; i < 1024; i++)
	{
		sptr = &spark[i];

		if (!sptr->On)
			continue;

		if (sptr->Flags & 0x40)
		{
			fx = &effects[sptr->FxObj];
			x = sptr->x + fx->pos.x_pos;
			y = sptr->y + fx->pos.y_pos;
			z = sptr->z + fx->pos.z_pos;

			if (sptr->sLife - sptr->Life > (GetRandomDraw() & 7) + 4)
			{
				sptr->x = x;
				sptr->y = y;
				sptr->z = z;
				sptr->Flags &= ~0x40;
			}
		}
		else if (sptr->Flags & 0x80)
		{
			item = &items[sptr->FxObj];

			if (sptr->Flags & 0x1000)
			{
				if (NodeOffsets[sptr->NodeNumber].GotIt)
				{
					pos.x = NodeVectors[sptr->NodeNumber].x;
					pos.y = NodeVectors[sptr->NodeNumber].y;
					pos.z = NodeVectors[sptr->NodeNumber].z;
				}
				else
				{
					pos.x = NodeOffsets[sptr->NodeNumber].x;
					pos.y = NodeOffsets[sptr->NodeNumber].y;
					pos.z = NodeOffsets[sptr->NodeNumber].z;

					if (NodeOffsets[sptr->NodeNumber].mesh_num < 0)
						GetLaraJointPos(&pos, -NodeOffsets[sptr->NodeNumber].mesh_num);
					else
						GetJointAbsPosition(item, &pos, NodeOffsets[sptr->NodeNumber].mesh_num);

					NodeOffsets[sptr->NodeNumber].GotIt = 1;
					NodeVectors[sptr->NodeNumber].x = pos.x;
					NodeVectors[sptr->NodeNumber].y = pos.y;
					NodeVectors[sptr->NodeNumber].z = pos.z;
				}

				x = sptr->x + pos.x;
				y = sptr->y + pos.y;
				z = sptr->z + pos.z;

				if (sptr->sLife - sptr->Life > (GetRandomDraw() & 3) + 8)
				{
					sptr->x = x;
					sptr->y = y;
					sptr->z = z;
					sptr->Flags &= ~0x1080;
				}
			}
			else
			{
				x = sptr->x + item->pos.x_pos;
				y = sptr->y + item->pos.y_pos;
				z = sptr->z + item->pos.z_pos;
			}
		}
		else
		{
			x = sptr->x;
			y = sptr->y;
			z = sptr->z;
		}

		fX = float(x - lara_item->pos.x_pos);
		fY = float(y - lara_item->pos.y_pos);
		fZ = float(z - lara_item->pos.z_pos);
		fpos.x = fX * D3DMView._11 + fY * D3DMView._21 + D3DMView._31 * fZ + D3DMView._41;
		fpos.y = fX * D3DMView._12 + fY * D3DMView._22 + D3DMView._32 * fZ + D3DMView._42;
		fpos.z = fX * D3DMView._13 + fY * D3DMView._23 + D3DMView._33 * fZ + D3DMView._43;

		clipflags[0] = 0;
		clipflags[1] = 0;
		clipflags[2] = 0;
		clipflags[3] = 0;

		if (fpos.z < f_mznear)
			continue;

		zv = f_mpersp / fpos.z;
		p[0] = zv * fpos.x + f_centerx;
		p[1] = zv * fpos.y + f_centery;
		p[2] = fpos.z;
		p[3] = f_moneopersp * zv;

		if (p[0] < f_left || p[0] > f_right || p[1] < f_top || p[1] > f_bottom)
			continue;

		if (sptr->Flags & 8)
		{
			if (sptr->Flags & 2)
				smallest_size = 4;

			S_DrawDrawSparksNEW(sptr, smallest_size, p);
		}
		else
		{
			fX -= float(sptr->Xvel >> 4);
			fY -= float(sptr->Yvel >> 4);
			fZ -= float(sptr->Zvel >> 4);
			fpos.x = fX * D3DMView._11 + fY * D3DMView._21 + D3DMView._31 * fZ + D3DMView._41;
			fpos.y = fX * D3DMView._12 + fY * D3DMView._22 + D3DMView._32 * fZ + D3DMView._42;
			fpos.z = fX * D3DMView._13 + fY * D3DMView._23 + D3DMView._33 * fZ + D3DMView._43;

			if (fpos.z < f_mznear)
				continue;

			zv = f_mpersp / fpos.z;
			p[4] = zv * fpos.x + f_centerx;
			p[5] = zv * fpos.y + f_centery;
			p[6] = fpos.z;
			p[7] = f_moneopersp * zv;

			if (p[4] < f_left || p[4] > f_right || p[5] < f_top || p[5] > f_bottom)
				continue;

			z = (long)fpos.z;

			if (z <= 0x3000)
			{
				c0 = RGBA(sptr->R, sptr->G, sptr->B, 0xFF);
				c1 = c0;
			}
			else
			{
				z = 0x5000 - z;
				r = (z * sptr->R) >> 13;
				g = (z * sptr->G) >> 13;
				b = (z * sptr->B) >> 13;
				c0 = RGBA(r, g, b, 0xFF);
				c1 = RGBA(r >> 1, g >> 1, b >> 1, 0xFF);
			}

			v[0].sx = p[0];
			v[0].sy = p[1];
			v[0].sz = p[2];
			v[0].rhw = p[3];
			v[0].color = c0;
			v[0].specular = 0xFF000000;

			v[1].sx = p[4];
			v[1].sy = p[5];
			v[1].sz = p[6];
			v[1].rhw = p[7];
			v[1].color = c1;
			v[1].specular = 0xFF000000;

			AddPolyLine(v, &tex);
		}
	}

	phd_PopMatrix();
}

void DrawLasers(ITEM_INFO* item)
{
	LASER_STRUCT* laser;
	SPRITESTRUCT* sprite;
	LASER_VECTOR* vtx;
	LASER_VECTOR* vtx2;
	D3DTLVERTEX* vbuf;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	TEXTURESTRUCT tex2;
	short* rand;
	float val, hStep, fx, fy, fz, mx, my, mz, zv;
	long lp, lp2, x, y, z, xStep, yStep, zStep, c1, c2, c3, c4;
	short* c;
	short clip[36];
	short clipFlag;

	if (!TriggerActive(item) || (item->trigger_flags & 1 && !InfraRed && !item->item_flags[3]))
		return;

	laser = (LASER_STRUCT*)item->data;
	sprite = &spriteinfo[objects[MISC_SPRITES].mesh_index + 3];

	tex.drawtype = 2;
	tex.tpage = 0;

	tex2.drawtype = 2;
	tex2.tpage = sprite->tpage;

	val = float((GlobalCounter >> 1) & 0x1F) * (1.0F / 256.0F) + sprite->y1;
	tex2.u1 = sprite->x1 + (1.0F / 512.0F);
	tex2.v1 = val;
	tex2.u2 = sprite->x1 + (31.0F / 256.0F) - (1.0F / 512.0F);
	tex2.v2 = val;
	tex2.u3 = sprite->x1 + (31.0F / 256.0F) - (1.0F / 512.0F);
	tex2.u4 = sprite->x1 + (1.0F / 512.0F);
	tex2.v4 = val + (1.0F / 512.0F);
	tex2.v3 = val + (1.0F / 512.0F);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	aSetViewMatrix();

	for (int i = 0; i < 3; i++)
	{
		vtx = (LASER_VECTOR*)&scratchpad[0];
		x = laser->v1[i].x;
		y = laser->v1[i].y;
		z = laser->v1[i].z;
		xStep = (laser->v4[i].x - x) >> 3;
		yStep = laser->v4[i].y - y;
		zStep = (laser->v4[i].z - z) >> 3;
		rand = laser->Rand;

		for (lp = 0; lp < 2; lp++)
		{
			for (lp2 = 0; lp2 < 9; lp2++)
			{
				vtx->num = 1.0F;
				vtx->x = (float)x;
				vtx->y = (float)y;
				vtx->z = (float)z;

				if (!lp2 || lp2 == 8)
					vtx->color = 0;
				else if (!(item->trigger_flags & 1) || InfraRed)
					vtx->color = (item->item_flags[3] >> 1) + abs(phd_sin((*rand << i) + (GlobalCounter << 9)) >> 8);
				else
					vtx->color = (item->item_flags[3] * abs(phd_sin((*rand << i) + (GlobalCounter << 9)) >> 8)) >> 6;

				x += xStep;
				z += zStep;
				vtx++;
				rand++;
			}

			x = laser->v1[i].x;
			y += yStep;
			z = laser->v1[i].z;
		}

		hStep = float(yStep >> 1);
		vtx2 = (LASER_VECTOR*)&scratchpad[0];

		for (lp = 0; lp < 9; lp++)
		{
			vtx->x = vtx2->x;
			vtx->y = vtx2->y + hStep;
			vtx->z = vtx2->z;
			vtx->num = 1.0F;
			vtx->color = vtx2->color;
			vtx++;
			vtx2++;
		}

		for (lp = 0; lp < 9; lp++)
		{
			vtx->x = vtx2->x;
			vtx->y = vtx2->y - hStep;
			vtx->z = vtx2->z;
			vtx->num = 1.0F;
			vtx->color = vtx2->color;
			vtx++;
			vtx2++;
		}

		vtx = (LASER_VECTOR*)&scratchpad[0];
		vbuf = aVertexBuffer;

		for (lp = 0; lp < 36; lp++)
		{
			fx = vtx->x;
			fy = vtx->y;
			fz = vtx->z;

			mx = fx * D3DMView._11 + fy * D3DMView._21 + fz * D3DMView._31 + D3DMView._41;
			my = fx * D3DMView._12 + fy * D3DMView._22 + fz * D3DMView._32 + D3DMView._42;
			mz = fx * D3DMView._13 + fy * D3DMView._23 + fz * D3DMView._33 + D3DMView._43;

			vbuf->tu = mx;
			vbuf->tv = my;

			clipFlag = 0;

			if (mz < f_mznear)
				clipFlag = -128;
			else
			{
				zv = f_mpersp / mz;
				mx = mx * zv + f_centerx;
				my = my * zv + f_centery;
				vbuf->rhw = f_moneopersp * zv;

				if (mx < f_left)
					clipFlag++;
				else if (mx > f_right)
					clipFlag += 2;

				if (my < f_top)
					clipFlag += 4;
				else if (my > f_bottom)
					clipFlag += 8;

				vbuf->sx = mx;
				vbuf->sy = my;
			}

			vbuf->sz = mz;
			clip[lp] = clipFlag;
			vbuf++;
			vtx++;
		}

		vtx = (LASER_VECTOR*)&scratchpad[0];
		vbuf = aVertexBuffer;
		c = clip;

		for (lp = 0; lp < 8; lp++)
		{
			c1 = vtx[0].color;
			c2 = vtx[1].color;
			c3 = vtx[9].color;
			c4 = vtx[10].color;

			v[0].sx = vbuf[0].sx;
			v[0].sy = vbuf[0].sy;
			v[0].sz = vbuf[0].sz;
			v[0].rhw = vbuf[0].rhw;
			v[0].tu = vbuf[0].tu;
			v[0].tv = vbuf[0].tv;

			v[1].sx = vbuf[1].sx;
			v[1].sy = vbuf[1].sy;
			v[1].sz = vbuf[1].sz;
			v[1].rhw = vbuf[1].rhw;
			v[1].tu = vbuf[1].tu;
			v[1].tv = vbuf[1].tv;

			v[2].sx = vbuf[18].sx;
			v[2].sy = vbuf[18].sy;
			v[2].sz = vbuf[18].sz;
			v[2].rhw = vbuf[18].rhw;
			v[2].tu = vbuf[18].tu;
			v[2].tv = vbuf[18].tv;

			v[3].sx = vbuf[19].sx;
			v[3].sy = vbuf[19].sy;
			v[3].sz = vbuf[19].sz;
			v[3].rhw = vbuf[19].rhw;
			v[3].tu = vbuf[19].tu;
			v[3].tv = vbuf[19].tv;

			if (item->trigger_flags & 2)
			{
				v[0].color = RGBA(c1, 0, 0, 0xFF);
				v[1].color = RGBA(c2, 0, 0, 0xFF);
			}
			else
			{
				v[0].color = RGBA(0, c1, 0, 0xFF);
				v[1].color = RGBA(0, c2, 0, 0xFF);
			}

			v[2].color = 0;
			v[3].color = 0;

			v[0].specular = vbuf[0].specular;
			v[1].specular = vbuf[1].specular;
			v[2].specular = vbuf[18].specular;
			v[3].specular = vbuf[19].specular;

			clipflags[0] = c[0];
			clipflags[1] = c[1];
			clipflags[2] = c[18];
			clipflags[3] = c[19];

			AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);

			v[0].sx = vbuf[9].sx;
			v[0].sy = vbuf[9].sy;
			v[0].sz = vbuf[9].sz;
			v[0].rhw = vbuf[9].rhw;
			v[0].tu = vbuf[9].tu;
			v[0].tv = vbuf[9].tv;

			v[1].sx = vbuf[10].sx;
			v[1].sy = vbuf[10].sy;
			v[1].sz = vbuf[10].sz;
			v[1].rhw = vbuf[10].rhw;
			v[1].tu = vbuf[10].tu;
			v[1].tv = vbuf[10].tv;

			v[2].sx = vbuf[27].sx;
			v[2].sy = vbuf[27].sy;
			v[2].sz = vbuf[27].sz;
			v[2].rhw = vbuf[27].rhw;
			v[2].tu = vbuf[27].tu;
			v[2].tv = vbuf[27].tv;

			v[3].sx = vbuf[28].sx;
			v[3].sy = vbuf[28].sy;
			v[3].sz = vbuf[28].sz;
			v[3].rhw = vbuf[28].rhw;
			v[3].tu = vbuf[28].tu;
			v[3].tv = vbuf[28].tv;

			if (item->trigger_flags & 2)
			{
				v[0].color = RGBA(c3, 0, 0, 0xFF);
				v[1].color = RGBA(c4, 0, 0, 0xFF);
			}
			else
			{
				v[0].color = RGBA(0, c3, 0, 0xFF);
				v[1].color = RGBA(0, c4, 0, 0xFF);
			}

			v[2].color = 0;
			v[3].color = 0;

			v[0].specular = vbuf[9].specular;
			v[1].specular = vbuf[10].specular;
			v[2].specular = vbuf[27].specular;
			v[3].specular = vbuf[28].specular;

			clipflags[0] = c[9];
			clipflags[1] = c[10];
			clipflags[2] = c[27];
			clipflags[3] = c[28];

			AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);

			v[0] = vbuf[0];
			v[1] = vbuf[1];
			v[2] = vbuf[9];
			v[3] = vbuf[10];

			clipflags[0] = c[0];
			clipflags[1] = c[1];
			clipflags[2] = c[9];
			clipflags[3] = c[10];

			if (item->trigger_flags & 2)
			{
				v[0].color = RGBA(c1, 0, 0, 0xFF);
				v[1].color = RGBA(c2, 0, 0, 0xFF);
				v[2].color = RGBA(c3, 0, 0, 0xFF);
				v[3].color = RGBA(c4, 0, 0, 0xFF);
			}
			else
			{
				v[0].color = RGBA(0, c1, 0, 0xFF);
				v[1].color = RGBA(0, c2, 0, 0xFF);
				v[2].color = RGBA(0, c3, 0, 0xFF);
				v[3].color = RGBA(0, c4, 0, 0xFF);
			}

			AddQuadSorted(v, 0, 1, 3, 2, &tex2, 1);

			vtx++;
			vbuf++;
			c++;
		}
	}

	phd_PopMatrix();
}

void DrawSteamLasers(ITEM_INFO* item)
{
	STEAMLASER_STRUCT* laser;
	SPRITESTRUCT* sprite;
	LASER_VECTOR* vtx;		//original uses SVECTOR
	D3DTLVERTEX* vbuf;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	TEXTURESTRUCT tex2;
	short* rand;
	short* c;
	float fx, fy, fz, mx, my, mz, zv, val;
	long on, x, y, z, xStep, yStep, zStep, col, lp, lp2, c1, c2, c3, c4;
	short clip[36];
	short clipFlag;

	if (!TriggerActive(item) || !SteamLasers[(GlobalCounter >> 5) & 7][item->trigger_flags])
		return;

	on = IsSteamOn(item);

	if (!on && !InfraRed && !item->item_flags[3])
		return;

	laser = (STEAMLASER_STRUCT*)item->data;
	sprite = &spriteinfo[objects[MISC_SPRITES].mesh_index + 3];

	tex.drawtype = 2;
	tex.tpage = 0;

	tex2.drawtype = 2;
	tex2.tpage = sprite->tpage;
	tex2.u1 = sprite->x1 + (1.0F / 512.0F);
	tex2.u2 = sprite->x1 + (31.0F / 256.0F) - (1.0F / 512.0F);
	tex2.u3 = sprite->x1 + (31.0F / 256.0F) - (1.0F / 512.0F);
	tex2.u4 = sprite->x1 + (1.0F / 512.0F);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos + item->item_flags[0], item->pos.z_pos);
	aSetViewMatrix();

	for (int i = 0; i < 2; i++)
	{
		vtx = (LASER_VECTOR*)&scratchpad[0];
		x = laser->v1[i].x;
		y = laser->v1[i].y;
		z = laser->v1[i].z;
		xStep = (laser->v4[i].x - x) >> 3;
		yStep = (laser->v4[i].y - y) >> 1;
		zStep = (laser->v4[i].z - z) >> 3;
		rand = laser->Rand;

		for (lp = 0; lp < 3; lp++)
		{
			for (lp2 = 0; lp2 < 9; lp2++)
			{
				vtx->x = (float)x;
				vtx->y = (float)y;
				vtx->z = (float)z;

				if (!lp2 || lp == 8)
					vtx->color = 0;
				else
					vtx->color = abs(phd_sin(*rand + (GlobalCounter << 9)) >> 8);

				if (on)
					col = GetSteamMultiplier(item, y, z) << 1;
				else
					col = 0;

				col += item->item_flags[3];

				if (InfraRed)
					col += 64;

				vtx->color = (vtx->color * col) >> 6;

				x += xStep;
				z += zStep;
				vtx++;
				rand++;
			}

			x = laser->v1[i].x;
			y += yStep;
			z = laser->v1[i].z;
		}

		vbuf = aVertexBuffer;
		vtx = (LASER_VECTOR*)&scratchpad[0];

		for (lp = 0; lp < 27; lp++)
		{
			fx = vtx->x;
			fy = vtx->y;
			fz = vtx->z;

			mx = fx * D3DMView._11 + fy * D3DMView._21 + fz * D3DMView._31 + D3DMView._41;
			my = fx * D3DMView._12 + fy * D3DMView._22 + fz * D3DMView._32 + D3DMView._42;
			mz = fx * D3DMView._13 + fy * D3DMView._23 + fz * D3DMView._33 + D3DMView._43;

			vbuf->tu = mx;
			vbuf->tv = my;

			clipFlag = 0;

			if (mz < f_mznear)
				clipFlag = -128;
			else
			{
				zv = f_mpersp / mz;
				mx = mx * zv + f_centerx;
				my = my * zv + f_centery;
				vbuf->rhw = zv * f_moneopersp;

				if (mx < f_left)
					clipFlag++;
				else if (mx > f_right)
					clipFlag += 2;

				if (my < f_top)
					clipFlag += 4;
				else if (my > f_bottom)
					clipFlag += 8;
			}

			clip[lp] = clipFlag;
			vbuf->sx = mx;
			vbuf->sy = my;
			vbuf->sz = mz;

			vbuf++;
			vtx++;
		}

		vbuf = aVertexBuffer;
		vtx = (LASER_VECTOR*)&scratchpad[0];
		c = clip;

		val = float((item->item_flags[0] >> 2) & 0x1F) * (1.0F / 256.0F) + sprite->y1;
		tex2.v1 = val;
		tex2.v2 = val;
		tex2.v3 = val + (31.0F / 256.0F);
		tex2.v4 = val + (31.0F / 256.0F);

		for (lp = 0; lp < 16; lp++)
		{
			c1 = vtx[0].color;
			c2 = vtx[1].color;
			c3 = vtx[9].color;
			c4 = vtx[10].color;

			if (lp < 8)
			{
				v[0].sx = vbuf[0].sx;
				v[0].sy = vbuf[0].sy;
				v[0].rhw = vbuf[0].rhw;
				v[0].tu = vbuf[0].tu;
				v[0].tv = vbuf[0].tv;

				v[1].sx = vbuf[1].sx;
				v[1].sy = vbuf[1].sy;
				v[1].rhw = vbuf[1].rhw;
				v[1].tu = vbuf[1].tu;
				v[1].tv = vbuf[1].tv;

				v[2].sx = vbuf[9].sx;
				v[2].sy = (3 * vbuf[0].sy + vbuf[9].sy) * 0.25F;
				v[2].rhw = vbuf[9].rhw;
				v[2].tu = vbuf[9].tu;
				v[2].tv = vbuf[9].tv;

				v[3].sx = vbuf[10].sx;
				v[3].sy = (3 * vbuf[1].sy + vbuf[10].sy) * 0.25F;
				v[3].rhw = vbuf[10].rhw;
				v[3].tu = vbuf[10].tu;
				v[3].tv = vbuf[10].tv;

				c3 = 0;
				c4 = 0;
			}
			else
			{
				v[0].sx = vbuf[0].sx;
				v[0].sy = vbuf[9].sy;
				v[0].rhw = vbuf[0].rhw;
				v[0].tu = vbuf[0].tu;
				v[0].tv = vbuf[0].tv;

				v[1].sx = vbuf[1].sx;
				v[1].sy = vbuf[10].sy;
				v[1].rhw = vbuf[1].rhw;
				v[1].tu = vbuf[1].tu;
				v[1].tv = vbuf[1].tv;

				v[2].sx = vbuf[9].sx;
				v[2].sy = (3 * vbuf[9].sy + vbuf[0].sy) * 0.25F;
				v[2].rhw = vbuf[9].rhw;
				v[2].tu = vbuf[9].tu;
				v[2].tv = vbuf[9].tv;

				v[3].sx = vbuf[10].sx;
				v[3].sy = (3 * vbuf[10].sy + vbuf[1].sy) * 0.25F;
				v[3].rhw = vbuf[10].rhw;
				v[3].tu = vbuf[10].tu;
				v[3].tv = vbuf[10].tv;

				c1 = 0;
				c2 = 0;
			}

			v[0].sz = vbuf[0].sz;
			v[1].sz = vbuf[1].sz;
			v[2].sz = vbuf[9].sz;
			v[3].sz = vbuf[10].sz;

			v[0].color = RGBA(c1, 0, 0, 0xFF);
			v[1].color = RGBA(c2, 0, 0, 0xFF);
			v[2].color = RGBA(c3, 0, 0, 0xFF);
			v[3].color = RGBA(c4, 0, 0, 0xFF);

			v[0].specular = 0xFF000000;
			v[1].specular = 0xFF000000;
			v[2].specular = 0xFF000000;
			v[3].specular = 0xFF000000;

			clipflags[0] = c[0];
			clipflags[1] = c[1];
			clipflags[2] = c[9];
			clipflags[3] = c[10];

			if (App.dx.Flags & 0x80)
				AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);

			v[0].sx = vbuf[0].sx;
			v[0].sy = vbuf[0].sy;
			v[0].rhw = vbuf[0].rhw;
			v[0].tu = vbuf[0].tu;
			v[0].tv = vbuf[0].tv;

			v[1].sx = vbuf[1].sx;
			v[1].sy = vbuf[1].sy;
			v[1].rhw = vbuf[1].rhw;
			v[1].tu = vbuf[1].tu;
			v[1].tv = vbuf[1].tv;

			v[2].sx = vbuf[9].sx;
			v[2].sy = vbuf[9].sy;
			v[2].rhw = vbuf[9].rhw;
			v[2].tu = vbuf[9].tu;
			v[2].tv = vbuf[9].tv;

			v[3].sx = vbuf[10].sx;
			v[3].sy = vbuf[10].sy;
			v[3].rhw = vbuf[10].rhw;
			v[3].tu = vbuf[10].tu;
			v[3].tv = vbuf[10].tv;

			v[0].color = RGBA(c1, 0, 0, 0xFF);
			v[1].color = RGBA(c2, 0, 0, 0xFF);
			v[2].color = RGBA(c3, 0, 0, 0xFF);
			v[3].color = RGBA(c4, 0, 0, 0xFF);

			AddQuadSorted(v, 0, 1, 3, 2, &tex2, 1);

			if (lp == 7)
			{
				vbuf += 2;
				vtx += 2;
				c += 2;
			}
			else
			{
				vbuf++;
				vtx++;
				c++;
			}
		}
	}

	phd_PopMatrix();
}

void DrawLightning()
{
	LIGHTNING_STRUCT* l;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX* pV;
	TEXTURESTRUCT tex;
	float* pPos;
	float* pVtx;
	short* pC;
	float pos[128];
	float vtx[256];
	float px, py, pz, px1, py1, pz1, px2, py2, pz2, px3, py3, pz3, n, nx, ny, nz, xAdd, yAdd, zAdd;
	float mx, my, mz, zv, size, size2, step, s, c, uAdd;
	long r1, r2, r3, col, r, g, b, lp;
	static long rand = 0xD371F947;
	short clip[32];
	short clipFlag;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

	for (int i = 0; i < 16; i++)
	{
		aSetViewMatrix();

		l = &Lightning[i];

		if (!l->Life)
			continue;

		px = (float)l->Point[0].x;
		py = (float)l->Point[0].y;
		pz = (float)l->Point[0].z;
		
		px1 = (float)l->Point[1].x - px;
		py1 = (float)l->Point[1].y - py;
		pz1 = (float)l->Point[1].z - pz;

		px2 = (float)l->Point[2].x - px;
		py2 = (float)l->Point[2].y - py;
		pz2 = (float)l->Point[2].z - pz;

		px3 = (float)l->Point[3].x - px;
		py3 = (float)l->Point[3].y - py;
		pz3 = (float)l->Point[3].z - pz;

		px = float(l->Point[0].x - lara_item->pos.x_pos);
		py = float(l->Point[0].y - lara_item->pos.y_pos);
		pz = float(l->Point[0].z - lara_item->pos.z_pos);

		r1 = rand;
		n = 0;
		pPos = pos;

		for (lp = 0; lp < 32; lp++)
		{
			if (!lp || lp == 31)
			{
				xAdd = 0;
				yAdd = 0;
				zAdd = 0;
			}
			else
			{
				r2 = 0x41C64E6D * r1 + 0x3039;
				r3 = 0x41C64E6D * r2 + 0x3039;
				r1 = 0x41C64E6D * r3 + 0x3039;
				xAdd = float(((r2 >> 10) & 0xF) - 8);
				yAdd = float(((r3 >> 10) & 0xF) - 8);
				zAdd = float(((r1 >> 10) & 0xF) - 8);
			}

			nx = (1.0F - n) * (1.0F - n) * n * 4.0F;
			ny = (1.0F - n) * (n * n) * 4.0F;
			nz = ((n + n) - 1.0F) * (n * n);
			pPos[0] = nx * px1 + ny * px2 + nz * px3 + px + xAdd;
			pPos[1] = nx * py1 + ny * py2 + nz * py3 + py + yAdd;
			pPos[2] = nx * pz1 + ny * pz2 + nz * pz3 + pz + zAdd;
			pPos[3] = 1.0F;

			n += (1.0F / 32.0F);
			pPos += 4;
		}

		rand = r1;
		pPos = pos;
		pVtx = vtx;
		pC = clip;

		for (lp = 0; lp < 32; lp++)
		{
			px = pPos[0];
			py = pPos[1];
			pz = pPos[2];
			mx = px * D3DMView._11 + py * D3DMView._21 + pz * D3DMView._31 + D3DMView._41;
			my = px * D3DMView._12 + py * D3DMView._22 + pz * D3DMView._32 + D3DMView._42;
			mz = px * D3DMView._13 + py * D3DMView._23 + pz * D3DMView._33 + D3DMView._43;

			clipFlag = 0;

			if (mz < f_mznear + 2.0F)
				clipFlag = -128;

			zv = f_mpersp / mz;

			pVtx[0] = mx * zv + f_centerx;
			pVtx[1] = my * zv + f_centery;
			pVtx[2] = zv * f_moneopersp;

			if (pVtx[0] < phd_winxmin)
				clipFlag++;
			else if (pVtx[0] > phd_winxmax)
				clipFlag += 2;

			if (pVtx[1] < phd_winymin)
				clipFlag += 4;
			else if (pVtx[1] > phd_winymax)
				clipFlag += 8;

			*pC++ = clipFlag;
			pVtx[3] = mz;
			pVtx[4] = mx;
			pVtx[5] = my;

			pPos += 4;
			pVtx += 8;
		}

		size = float(l->Size >> 1);
		step = 0;

		if (l->Flags & 8)
		{
			step = size * 0.125F;
			size = 0;
		}
		else if (l->Flags & 4)
			step = -(size * (1.0F / 32.0F));

		pPos = pos;
		pVtx = vtx;

		for (lp = 0; lp < 32; lp++)
		{
			r = phd_atan(long(pVtx[8] - pVtx[0]), long(pVtx[9] - pVtx[1]));
			s = fSin(-r);
			c = fCos(-r);

			if (size <= 0)
				size2 = 2.0F;
			else
				size2 = size;

			zv = f_mpersp / pVtx[3] * size2;

			pPos[0] = zv * s;
			pPos[1] = zv * c;

			size += step;

			if (l->Flags & 8 && lp == 8)
			{
				if (l->Flags & 4)
					step = float(l->Size >> 1) * (-1.0F / 28.0F);
				else
					step = 0;

				l->Flags &= ~8;
			}

			pPos += 4;
			pVtx += 8;
		}

		sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 28];
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;

		uAdd = float(31 - 4 * (aWibble & 7)) * (1.0F / 256.0F);

		if (l->Life < 16)
		{
			r = (l->Life * l->r) >> 4;
			g = (l->Life * l->g) >> 4;
			b = (l->Life * l->b) >> 4;
		}
		else
		{
			r = l->r;
			g = l->g;
			b = l->b;
		}

		col = RGBONLY(b, g, r);

		pPos = pos;
		pVtx = vtx;
		pC = clip;
		pV = aVertexBuffer;
		zv = f_mznear + 128.0F;

		for (lp = 0; lp < 31; lp++)
		{
			tex.u1 = (float(lp & 3) * 8.0F * (1.0F / 256.0F) + sprite->x1) + (1.0F / 512.0F) + uAdd;
			tex.v1 = sprite->y1;
			tex.u2 = tex.u1;
			tex.v2 = sprite->y2;
			tex.u3 = (float(lp & 3) * 8.0F * (1.0F / 256.0F) + sprite->x1) + (8.0F / 256.0F) + uAdd;
			tex.v3 = sprite->y2;
			tex.u4 = tex.u3;
			tex.v4 = sprite->y1;

			if (pVtx[3] >= zv && pVtx[11] >= zv)
			{
				pV[0].sx = pVtx[0] - pPos[0];
				pV[0].sy = pVtx[1] - pPos[1];
				pV[0].rhw = pVtx[2];
				pV[0].sz = pVtx[3];
				pV[0].tu = pVtx[4];
				pV[0].tv = pVtx[5];
				pV[0].color = col;
				pV[0].specular = 0xFF000000;

				pV[1].sx = pVtx[0] + pPos[0];
				pV[1].sy = pVtx[1] + pPos[1];
				pV[1].rhw = pVtx[2];
				pV[1].sz = pVtx[3];
				pV[1].tu = pVtx[4];
				pV[1].tv = pVtx[5];
				pV[1].color = col;
				pV[1].specular = 0xFF000000;

				pV[2].sx = pVtx[8] - pPos[4];
				pV[2].sy = pVtx[9] - pPos[5];
				pV[2].rhw = pVtx[10];
				pV[2].sz = pVtx[11];
				pV[2].tu = pVtx[12];
				pV[2].tv = pVtx[13];
				pV[2].color = col;
				pV[2].specular = 0xFF000000;

				pV[3].sx = pVtx[8] + pPos[4];
				pV[3].sy = pVtx[9] + pPos[5];
				pV[3].rhw = pVtx[10];
				pV[3].sz = pVtx[11];
				pV[3].tu = pVtx[12];
				pV[3].tv = pVtx[13];
				pV[3].color = col;
				pV[3].specular = 0xFF000000;

				clipflags[0] = pC[0];
				clipflags[1] = pC[0];
				clipflags[2] = pC[1];
				clipflags[3] = pC[1];

				AddQuadSorted(pV, 1, 0, 2, 3, &tex, 1);
				AddQuadSorted(pV, 1, 0, 2, 3, &tex, 1);
			}

			pPos += 4;
			pVtx += 8;
			pC++;
		}
	}

	phd_PopMatrix();
}

void OldDrawLightning()
{
	LIGHTNING_STRUCT* pL;
	SPRITESTRUCT* sprite;
	PHD_VECTOR* vec;
	SVECTOR* offsets;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR p1, p2, p3;
	long* Z;
	short* XY;
	float perspz;
	long c, xsize, ysize, r, g, b;
	long x1, y1, z1, x2, y2, z2, z;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 28];

	for (int i = 0; i < 16; i++)
	{
		pL = &Lightning[i];

		if (!pL->Life)
			continue;

		vec = (PHD_VECTOR*)&scratchpad[128];
		memcpy(&vec[0], &pL->Point[0], sizeof(PHD_VECTOR));
		memcpy(&vec[1], &pL->Point[0], 4 * sizeof(PHD_VECTOR));
		memcpy(&vec[5], &pL->Point[3], sizeof(PHD_VECTOR));

		for (int j = 0; j < 6; j++)
		{
			vec[j].x -= lara_item->pos.x_pos;
			vec[j].y -= lara_item->pos.y_pos;
			vec[j].z -= lara_item->pos.z_pos;
		}

		offsets = (SVECTOR*)&scratchpad[0];
		XY = (short*)&scratchpad[256];
		Z = (long*)&scratchpad[512];
		CalcLightningSpline(vec, offsets, pL);

		if (vec[0].x > 0x6000 || vec[0].y > 0x6000 || vec[0].z > 0x6000)
			continue;

		for (int j = 0; j < pL->Segments; j++)
		{
			p1.x = (offsets[0].x * phd_mxptr[M00] + offsets[0].y * phd_mxptr[M01] + offsets[0].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p1.y = (offsets[0].x * phd_mxptr[M10] + offsets[0].y * phd_mxptr[M11] + offsets[0].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p1.z = (offsets[0].x * phd_mxptr[M20] + offsets[0].y * phd_mxptr[M21] + offsets[0].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			p2.x = (offsets[1].x * phd_mxptr[M00] + offsets[1].y * phd_mxptr[M01] + offsets[1].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p2.y = (offsets[1].x * phd_mxptr[M10] + offsets[1].y * phd_mxptr[M11] + offsets[1].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p2.z = (offsets[1].x * phd_mxptr[M20] + offsets[1].y * phd_mxptr[M21] + offsets[1].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			p3.x = (offsets[2].x * phd_mxptr[M00] + offsets[2].y * phd_mxptr[M01] + offsets[2].z * phd_mxptr[M02] + phd_mxptr[M03]) >> 14;
			p3.y = (offsets[2].x * phd_mxptr[M10] + offsets[2].y * phd_mxptr[M11] + offsets[2].z * phd_mxptr[M12] + phd_mxptr[M13]) >> 14;
			p3.z = (offsets[2].x * phd_mxptr[M20] + offsets[2].y * phd_mxptr[M21] + offsets[2].z * phd_mxptr[M22] + phd_mxptr[M23]) >> 14;

			XY[0] = (short)p1.x;
			XY[1] = (short)p1.y;
			Z[0] = p1.z;

			XY[2] = (short)p2.x;
			XY[3] = (short)p2.y;
			Z[1] = p2.z;

			XY[4] = (short)p3.x;
			XY[5] = (short)p3.y;
			Z[2] = p3.z;

			offsets += 3;
			XY += 6;
			Z += 3;
		}

		XY = (short*)&scratchpad[256];
		Z = (long*)&scratchpad[512];

		for (int j = 0; j < 3 * pL->Segments - 1; j++)
		{
			if (pL->Life < 16)
				c = pL->Life << 2;
			else
				c = 64;

			if (Z[0] > 0x3000)
				c = (c * (0x5000 - *Z)) >> 13;

			c = RGBA(c, c, c, 66);

			x1 = XY[0];
			y1 = XY[1];
			z1 = Z[0];
			x2 = XY[2];
			y2 = XY[3];
			z2 = Z[1];
			setXYZ4(v, x1, y1, z1, x2, y2, z2, x1, y1, z1, x2, y2, z2, clipflags);
			x1 = (long)v[0].sx;
			y1 = (long)v[0].sy;
			z1 = (long)v[0].sz;
			x2 = (long)v[1].sx;
			y2 = (long)v[1].sy;
			z2 = (long)v[1].sz;

			if (ClipLine(x1, y1, z1, x2, y2, z2, phd_winxmin, phd_winymin, phd_winxmax, phd_winymax))
			{
				perspz = f_mpersp / Z[0] * f_moneopersp;

				v[0].sx = (float)x1;
				v[0].sy = (float)y1;
				v[0].sz = f_a - perspz * f_boo;
				v[0].rhw = perspz;
				v[0].color = c;
				v[0].specular = 0xFF000000;

				v[1].sx = (float)x2;
				v[1].sy = (float)y2;
				v[1].sz = f_a - perspz * f_boo;
				v[1].rhw = perspz;
				v[1].color = c;
				v[1].specular = 0xFF000000;

				AddLineSorted(&v[0], &v[1], 6);

				if (Z[0] > 0x4000)
					xsize = 1;
				else
					xsize = ((0x4000 - Z[0]) * pL->Size) >> 16;

				if (xsize < 4)
					xsize = 4;

				if (pL->Life < 16)
				{
					r = (pL->Life * pL->r) >> 4;
					g = (pL->Life * pL->g) >> 4;
					b = (pL->Life * pL->b) >> 4;
				}
				else
				{
					r = pL->r;
					g = pL->g;
					b = pL->b;
				}

				if (Z[0] > 0x3000)
				{
					r = (r * (0x5000 - Z[0])) >> 13;
					g = (g * (0x5000 - Z[0])) >> 13;
					b = (b * (0x5000 - Z[0])) >> 13;
				}

				x1 = XY[0];
				y1 = XY[1];
				z1 = Z[0];
				x2 = XY[2];
				y2 = XY[3];
				z2 = Z[1];
				setXYZ4(v, x1, y1, z1, x2, y2, z2, x1, y1, z1, x2, y2, z2, clipflags);
				x1 = (long)v[0].sx;
				y1 = (long)v[0].sy;
				z1 = (long)v[0].sz;
				x2 = (long)v[1].sx;
				y2 = (long)v[1].sy;
				z2 = (long)v[1].sz;

				vec[0].x = (x1 - x2) << 8;
				vec[0].y = (y1 - y2) << 8;
				vec[0].z = 0;
				Normalise(vec);
				vec[0].z = vec[0].x;
				vec[0].x = -vec[0].y;
				vec[0].y = vec[0].z;

				ysize = (vec[0].y * xsize) >> 12;
				xsize = (vec[0].x * xsize) >> 12;
				z = (z1 + z2) >> 1;

				if (z > 64)
				{
					setXY4(v, x1 + xsize, y1 + ysize, x2 + xsize, y2 + ysize, x1 - xsize, y1 - ysize, x2 - xsize, y2 - ysize, z, clipflags);

					c = RGBA(b, g, r, 0xFF);

					v[0].color = c;
					v[1].color = c;
					v[2].color = c;
					v[3].color = c;
					v[0].specular = 0xFF000000;
					v[1].specular = 0xFF000000;
					v[2].specular = 0xFF000000;
					v[3].specular = 0xFF000000;
					tex.drawtype = 2;
					tex.flag = 0;
					tex.tpage = sprite->tpage;
					tex.u1 = sprite->x1;
					tex.v1 = sprite->y2;
					tex.u2 = sprite->x2;
					tex.v2 = sprite->y2;
					tex.u3 = sprite->x2;
					tex.v3 = sprite->y1;
					tex.u4 = sprite->x1;
					tex.v4 = sprite->y1;
					AddQuadClippedSorted(v, 0, 1, 3, 2, &tex, 0);
				}
			}

			XY += 2;
			Z++;
		}
	}

	phd_PopMatrix();
}

void DrawTwogunLaser(TWOGUN_INFO* info)
{
	SVECTOR* pos;
	D3DTLVERTEX* v;
	SPRITESTRUCT* sprite;
	TEXTURESTRUCT tex;
	float* pVtx;
	short* c;
	float vtx[1024];
	float x, y, z, mx, my, mz, zv, uAdd;
	long r, g, b, size, size2, step, col, lp;
	short angle, pz, clipFlag;
	short clip[128];

	if (info->fadein < 8)
	{
		r = (info->fadein * (uchar)info->r) >> 3;
		g = (info->fadein * (uchar)info->g) >> 3;
		b = (info->fadein * (uchar)info->b) >> 3;
	}
	else if (info->life < 16)
	{
		r = (info->life * (uchar)info->r) >> 4;
		g = (info->life * (uchar)info->g) >> 4;
		b = (info->life * (uchar)info->b) >> 4;
	}
	else
	{
		r = (uchar)info->r;
		g = (uchar)info->g;
		b = (uchar)info->b;
	}

	phd_PushMatrix();
	phd_TranslateAbs(info->pos.x_pos, info->pos.y_pos, info->pos.z_pos);
	phd_RotYXZ(info->pos.y_rot, info->pos.x_rot, info->pos.z_rot);
	aSetViewMatrix();

	pos = (SVECTOR*)&tsv_buffer[0];
	size = 0;
	step = info->size << 2;
	pz = 0;
	angle = info->spin;

	for (lp = 0; lp < 8; lp++)
	{
		size2 = size >> 1;

		if (size2 > 48)
			size2 = 48;

		pos->x = short((size * phd_sin(angle)) >> 15);
		pos->y = short((size * phd_cos(angle)) >> 15);
		pos->z = pz;
		pos[1].x = short(pos->x - size2);
		pos[1].y = short(pos->y - size2);
		pos[1].z = pz;

		size += step;
		pz += info->length >> 6;
		angle += info->coil;
		pos += 2;
	}

	for (lp = 0; lp < 56; lp++)
	{
		size2 = size >> 1;

		if (size2 > 48)
			size2 = 48;

		pos->x = short((size * phd_sin(angle)) >> 15);
		pos->y = short((size * phd_cos(angle)) >> 15);
		pos->z = pz;
		pos[1].x = short(pos->x - size2);
		pos[1].y = short(pos->y - size2);
		pos[1].z = pz;

		if (lp & 1)
			size -= info->size;
		
		if (size < 4)
			size = 4;

		pz += info->length >> 6;
		angle += info->coil;
		pos += 2;
	}

	pos = (SVECTOR*)&tsv_buffer[0];
	pVtx = vtx;
	c = clip;

	for (lp = 0; lp < 128; lp++)
	{
		x = pos->x;
		y = pos->y;
		z = pos->z;
		mx = D3DMView._11 * x + D3DMView._21 * y + D3DMView._31 * z + D3DMView._41;
		my = D3DMView._12 * x + D3DMView._22 * y + D3DMView._32 * z + D3DMView._42;
		mz = D3DMView._13 * x + D3DMView._23 * y + D3DMView._33 * z + D3DMView._43;

		clipFlag = 0;

		if (mz < f_mznear)
			clipFlag = -128;
		else
		{
			zv = f_mpersp / mz;
			pVtx[0] = mx * zv + f_centerx;
			pVtx[1] = my * zv + f_centery;
			pVtx[2] = f_moneopersp * zv;

			if (pVtx[0] < phd_winxmin)
				clipFlag++;
			else if (pVtx[0] > phd_winxmax)
				clipFlag += 2;

			if (pVtx[1] < phd_winymin)
				clipFlag += 4;
			else if (pVtx[1] > phd_winymax)
				clipFlag += 8;
		}

		*c++ = clipFlag;
		pVtx[3] = mz;
		pVtx[4] = mx;
		pVtx[5] = my;

		pVtx += 8;
		pos++;
	}

	sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 28];
	tex.drawtype = 2;
	tex.flag = 0;
	tex.tpage = sprite->tpage;
	uAdd = float(31 - 4 * (aWibble & 7)) * (1.0F / 256.0F);
	col = RGBONLY(r, g, b);

	v = aVertexBuffer;
	pVtx = vtx;
	c = clip;

	for (lp = 0; lp < 63; lp++)
	{
		tex.u1 = ((lp & 3) * 8.0F * (1.0F / 256.0F) + sprite->x1) + uAdd + (1.0F / 512.0F);
		tex.u2 = tex.u1;
		tex.u3 = ((lp & 3) * 8.0F * (1.0F / 256.0F) + sprite->x1) + uAdd + (8.0F / 256.0F);
		tex.u4 = tex.u3;
		tex.v1 = sprite->y1;
		tex.v2 = sprite->y2;
		tex.v3 = sprite->y2;
		tex.v4 = sprite->y1;

		v[0].sx = pVtx[0];
		v[0].sy = pVtx[1];
		v[0].rhw = pVtx[2];
		v[0].sz = pVtx[3];
		v[0].tu = pVtx[4];
		v[0].tv = pVtx[5];
		v[0].color = col;
		v[0].specular = 0xFF000000;

		v[1].sx = pVtx[8];
		v[1].sy = pVtx[9];
		v[1].rhw = pVtx[10];
		v[1].sz = pVtx[11];
		v[1].tu = pVtx[12];
		v[1].tv = pVtx[13];
		v[1].color = col;
		v[1].specular = 0xFF000000;

		v[2].sx = pVtx[16];
		v[2].sy = pVtx[17];
		v[2].rhw = pVtx[18];
		v[2].sz = pVtx[19];
		v[2].tu = pVtx[20];
		v[2].tv = pVtx[21];
		v[2].color = col;
		v[2].specular = 0xFF000000;

		v[3].sx = pVtx[24];
		v[3].sy = pVtx[25];
		v[3].rhw = pVtx[26];
		v[3].sz = pVtx[27];
		v[3].tu = pVtx[28];
		v[3].tv = pVtx[29];
		v[3].color = col;
		v[3].specular = 0xFF000000;

		clipflags[0] = c[0];
		clipflags[1] = c[1];
		clipflags[2] = c[2];
		clipflags[3] = c[3];

		AddQuadSorted(v, 1, 0, 2, 3, &tex, 1);

		c += 2;
		pVtx += 16;
	}

	phd_PopMatrix();
}

void DrawRope(ROPE_STRUCT* rope)
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long dx, dy, d, b, w, spec;
	long x1, y1, z1, x2, y2, z2, x3, y3, x4, y4;

	ProjectRopePoints(rope);
	dx = rope->Coords[1][0] - rope->Coords[0][0];
	dy = rope->Coords[1][1] - rope->Coords[0][1];
	d = SQUARE(dx) + SQUARE(dy);
	d = phd_sqrt(abs(d));

	dx <<= 16;
	dy <<= 16;
	d <<= 16;

	if (d)
	{
		d = ((0x1000000 / (d >> 8)) << 8) >> 8;
		b = dx;
		dx = ((__int64)-dy * (__int64)d) >> 16;
		dy = ((__int64)b * (__int64)d) >> 16;
	}

	w = 0x60000;

	if (rope->Coords[0][2])
	{
		w = 0x60000 * phd_persp / rope->Coords[0][2];

		if (w < 1)
			w = 1;
	}

	w <<= 16;
	dx = (((__int64)dx * (__int64)w) >> 16) >> 16;
	dy = (((__int64)dy * (__int64)w) >> 16) >> 16;
	x1 = rope->Coords[0][0] - dx;
	y1 = rope->Coords[0][1] - dy;
	z1 = rope->Coords[0][2] >> 14;
	x4 = rope->Coords[0][0] + dx;
	y4 = rope->Coords[0][1] + dy;

	for (int i = 0; i < 23; i++)
	{
		dx = rope->Coords[i + 1][0] - rope->Coords[i][0];
		dy = rope->Coords[i + 1][1] - rope->Coords[i][1];
		d = SQUARE(dx) + SQUARE(dy);
		d = phd_sqrt(abs(d));

		dx <<= 16;
		dy <<= 16;
		d <<= 16;

		if (d)
		{
			d = ((0x1000000 / (d >> 8)) << 8) >> 8;
			b = dx;
			dx = ((__int64)-dy * (__int64)d) >> 16;
			dy = ((__int64)b * (__int64)d) >> 16;
		}

		w = 0x60000;

		if (rope->Coords[i][2])
		{
			w = 0x60000 * phd_persp / rope->Coords[i][2];

			if (w < 3)
				w = 3;
		}

		w <<= 16;
		dx = (((__int64)dx * (__int64)w) >> 16) >> 16;
		dy = (((__int64)dy * (__int64)w) >> 16) >> 16;
		x2 = rope->Coords[i + 1][0] - dx;
		y2 = rope->Coords[i + 1][1] - dy;
		z2 = rope->Coords[i + 1][2] >> 14;
		x3 = rope->Coords[i + 1][0] + dx;
		y3 = rope->Coords[i + 1][1] + dy;

		if ((double)z1 > f_mznear && (double)z2 > f_mznear)
		{
			setXY4(v, x1, y1, x2, y2, x3, y3, x4, y4, z1, clipflags);
			v[0].color = 0xFF7F7F7F;
			v[1].color = 0xFF7F7F7F;
			v[2].color = 0xFF7F7F7F;
			v[3].color = 0xFF7F7F7F;

			spec = 255;

			if (z1 > 0x3000)
				spec = (255 * (0x5000 - z1)) >> 13;

			v[0].specular = spec << 24;
			v[1].specular = spec << 24;
			v[2].specular = spec << 24;
			v[3].specular = spec << 24;
			sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 16];
			tex.drawtype = 1;
			tex.flag = 0;
			tex.tpage = sprite->tpage;
			tex.u1 = sprite->x1;
			tex.v1 = sprite->y1;
			tex.u2 = sprite->x1;
			tex.v2 = sprite->y2;
			tex.u3 = sprite->x2;
			tex.v3 = sprite->y2;
			tex.u4 = sprite->x2;
			tex.v4 = sprite->y1;
			AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
		}

		x1 = x2;
		y1 = y2;
		z1 = z2;
		x4 = x3;
		y4 = y3;
	}
}

void S_DrawDrawSparks(SPARKS* sptr, long smallest_size, short* xyptr, long* zptr)
{
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long x1, y1, z1, x2, y2, z2, x3, y3, x4, y4;
	long cR, cG, cB, c1, c2, s1, s2, s1h, s2h, scale;
	long sin, cos, sx1, sx2, sy1, sy2, cx1, cx2, cy1, cy2;

	if (sptr->Flags & 8)
	{
		z1 = zptr[0];

		if (z1 <= 0)
			return;

		if (z1 >= 0x5000)
		{
			sptr->On = 0;
			return;
		}

		if (sptr->Flags & 2)
		{
			scale = sptr->Size << sptr->Scalar;
			s1 = ((phd_persp * sptr->Size) << sptr->Scalar) / z1;
			s2 = ((phd_persp * sptr->Size) << sptr->Scalar) / z1;

			if (s1 > scale)
				s1 = scale;
			else if (s1 < smallest_size)
				s1 = smallest_size;

			if (s2 > scale)
				s2 = scale;
			else if (s2 < smallest_size)
				s2 = smallest_size;
		}
		else
		{
			s1 = sptr->Size;
			s2 = sptr->Size;
		}

		x1 = xyptr[0];
		y1 = xyptr[1];
		s1h = s1 >> 1;
		s2h = s2 >> 1;

		if (x1 + s1h >= phd_winxmin && x1 - s1h < phd_winxmax && y1 + s2h >= phd_winymin && y1 - s2h < phd_winymax)
		{
			if (sptr->Flags & 0x10)
			{
				sin = rcossin_tbl[sptr->RotAng << 1];
				cos = rcossin_tbl[(sptr->RotAng << 1) + 1];
				sx1 = (-s1h * sin) >> 12;
				sx2 = (s1h * sin) >> 12;
				sy1 = (-s2h * sin) >> 12;
				sy2 = (s2h * sin) >> 12;
				cx1 = (-s1h * cos) >> 12;
				cx2 = (s1h * cos) >> 12;
				cy1 = (-s2h * cos) >> 12;
				cy2 = (s2h * cos) >> 12;
				x1 = sx1 - cy1 + xyptr[0];
				x2 = sx2 - cy1 + xyptr[0];
				x3 = sx2 - cy2 + xyptr[0];
				x4 = sx1 - cy2 + xyptr[0];
				y1 = cx1 + sy1 + xyptr[1];
				y2 = cx2 + sy1 + xyptr[1];
				y3 = cx2 + sy2 + xyptr[1];
				y4 = cx1 + sy2 + xyptr[1];
				setXY4(v, x1, y1, x2, y2, x3, y3, x4, y4, z1, clipflags);
			}
			else
			{
				x1 = xyptr[0] - s1h;
				x2 = xyptr[0] + s1h;
				y1 = xyptr[1] - s2h;
				y2 = xyptr[1] + s2h;
				setXY4(v, x1, y1, x2, y1, x2, y2, x1, y2, z1, clipflags);
			}

			sprite = &spriteinfo[sptr->Def];

			if (z1 <= 0x3000)
			{
				cR = sptr->R;
				cG = sptr->G;
				cB = sptr->B;
			}
			else
			{
				cR = ((0x5000 - z1) * sptr->R) >> 13;
				cG = ((0x5000 - z1) * sptr->G) >> 13;
				cB = ((0x5000 - z1) * sptr->B) >> 13;
			}

			c1 = RGBA(cR, cG, cB, 0xFF);
			v[0].color = c1;
			v[1].color = c1;
			v[2].color = c1;
			v[3].color = c1;
			v[0].specular = 0xFF000000;
			v[1].specular = 0xFF000000;
			v[2].specular = 0xFF000000;
			v[3].specular = 0xFF000000;

			if (sptr->TransType)
				tex.drawtype = 2;
			else
				tex.drawtype = 1;

			tex.tpage = sprite->tpage;
			tex.u1 = sprite->x1;
			tex.v1 = sprite->y1;
			tex.u2 = sprite->x2;
			tex.v2 = sprite->y1;
			tex.u3 = sprite->x2;
			tex.v3 = sprite->y2;
			tex.u4 = sprite->x1;
			tex.v4 = sprite->y2;
			AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
		}
	}
	else
	{
		x1 = xyptr[0];
		y1 = xyptr[1];
		x2 = xyptr[2];
		y2 = xyptr[3];
		z1 = zptr[0];
		z2 = zptr[1];

		if (z1 <= 0x3000)
		{
			cR = sptr->R;
			cG = sptr->G;
			cB = sptr->B;
		}
		else
		{
			cR = ((0x5000 - z1) * sptr->R) >> 13;
			cG = ((0x5000 - z1) * sptr->G) >> 13;
			cB = ((0x5000 - z1) * sptr->B) >> 13;
		}

		c1 = RGBA(cR, cG, cB, 0xFF);
		c2 = RGBA(cR >> 1, cG >> 1, cB >> 1, 0xFF);

		if (ClipLine(x1, y1, z1, x2, y2, z2, phd_winxmin, phd_winymin, phd_winxmax, phd_winymax))
		{
			v[0].sx = (float)x1;
			v[0].sy = (float)y1;
			v[0].rhw = f_mpersp / z1 * f_moneopersp;
			v[0].sz = f_a - v[0].rhw * f_boo;
			v[0].color = c1;
			v[0].specular = 0xFF000000;
			v[1].sx = (float)x2;
			v[1].sy = (float)y2;
			v[1].rhw = f_mpersp / z1 * f_moneopersp;
			v[1].sz = f_a - v[1].rhw * f_boo;
			v[1].color = c2;
			v[1].specular = 0xFF000000;
			AddLineSorted(v, &v[1], 6);
		}
	}
}

void S_DrawSplashes()
{
	SPLASH_STRUCT* splash;
	RIPPLE_STRUCT* ripple;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	long* Z;
	short* XY;
	short* offsets;
	uchar* links;
	ulong c0, c1;
	long x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, linkNum, r, g, b;
	short rads[6];
	short yVals[6];

	offsets = (short*)&scratchpad[768];

	for (int i = 0; i < 4; i++)
	{
		splash = &splashes[i];

		if (!(splash->flags & 1))
			continue;

		phd_PushMatrix();
		phd_TranslateAbs(splash->x, splash->y, splash->z);
		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		rads[0] = splash->InnerRad;
		rads[1] = splash->InnerRad + splash->InnerSize;
		rads[2] = splash->MiddleRad;
		rads[3] = splash->MiddleRad + splash->MiddleSize;
		rads[4] = splash->OuterRad;
		rads[5] = splash->OuterRad + splash->OuterSize;

		yVals[0] = 0;
		yVals[1] = splash->InnerY;
		yVals[2] = 0;
		yVals[3] = splash->MiddleY;
		yVals[4] = 0;
		yVals[5] = 0;

		for (int j = 0; j < 6; j++)
		{
			for (int k = 0; k < 0x10000; k += 0x2000)
			{
				offsets[0] = (rads[j] * phd_sin(k)) >> 13;
				offsets[1] = yVals[j] >> 3;
				offsets[2] = (rads[j] * phd_cos(k)) >> 13;
				*XY++ = short((phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2] + phd_mxptr[M03]) >> 14);
				*XY++ = short((phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2] + phd_mxptr[M13]) >> 14);
				*Z++ = (phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2] + phd_mxptr[M23]) >> 14;
				Z++;
			}
		}

		phd_PopMatrix();
		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		for (int j = 0; j < 3; j++)
		{
			if (j == 2 || (!j && splash->flags & 4) || (j == 1 && splash->flags & 8))
				sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 4 + ((wibble >> 4) & 3)];
			else
				sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 8];

			links = SplashLinks;
			linkNum = j << 5;

			for (int k = 0; k < 8; k++)
			{
				x1 = XY[links[0] + linkNum];
				y1 = XY[links[0] + linkNum + 1];
				z1 = Z[links[0] + linkNum];
				links++;

				x2 = XY[links[0] + linkNum];
				y2 = XY[links[0] + linkNum + 1];
				z2 = Z[links[0] + linkNum];
				links++;

				x3 = XY[links[0] + linkNum];
				y3 = XY[links[0] + linkNum + 1];
				z3 = Z[links[0] + linkNum];
				links++;

				x4 = XY[links[0] + linkNum];
				y4 = XY[links[0] + linkNum + 1];
				z4 = Z[links[0] + linkNum];
				links++;

				setXYZ4(v, x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3, clipflags);

				r = splash->life << 1;
				g = splash->life << 1;
				b = splash->life << 1;

				if (r > 255)
					r = 255;

				if (g > 255)
					g = 255;

				if (b > 255)
					b = 255;

				c0 = RGBA(r, g, b, 0xFF);

				r = (splash->life - (splash->life >> 2)) << 1;
				g = (splash->life - (splash->life >> 2)) << 1;
				b = (splash->life - (splash->life >> 2)) << 1;

				if (r > 255)
					r = 255;

				if (g > 255)
					g = 255;

				if (b > 255)
					b = 255;

				c1 = RGBA(r, g, b, 0xFF);

				v[0].color = c0;
				v[1].color = c0;
				v[2].color = c1;
				v[3].color = c1;
				v[0].specular = 0xFF000000;
				v[1].specular = 0xFF000000;
				v[2].specular = 0xFF000000;
				v[3].specular = 0xFF000000;
				tex.drawtype = 2;
				tex.flag = 0;
				tex.tpage = sprite->tpage;
				tex.u1 = sprite->x1;
				tex.v1 = sprite->y1;
				tex.u2 = sprite->x2;
				tex.v2 = sprite->y1;
				tex.v3 = sprite->y2;
				tex.u3 = sprite->x2;
				tex.u4 = sprite->x1;
				tex.v4 = sprite->y2;
				AddQuadSorted(v, 0, 1, 2, 3, &tex, 1);
			}
		}
	}

	for (int i = 0; i < 32; i++)
	{
		ripple = &ripples[i];

		if (!(ripple->flags & 1))
			continue;

		phd_PushMatrix();
		phd_TranslateAbs(ripple->x, ripple->y, ripple->z);

		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		offsets[0] = -ripple->size;
		offsets[1] = 0;
		offsets[2] = -ripple->size;
		*XY++ = short((phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2] + phd_mxptr[M03]) >> 14);
		*XY++ = short((phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2] + phd_mxptr[M13]) >> 14);
		*Z++ = (phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2] + phd_mxptr[M23]) >> 14;
		Z++;

		offsets[0] = -ripple->size;
		offsets[1] = 0;
		offsets[2] = ripple->size;
		*XY++ = short((phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2] + phd_mxptr[M03]) >> 14);
		*XY++ = short((phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2] + phd_mxptr[M13]) >> 14);
		*Z++ = (phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2] + phd_mxptr[M23]) >> 14;
		Z++;

		offsets[0] = ripple->size;
		offsets[1] = 0;
		offsets[2] = ripple->size;
		*XY++ = short((phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2] + phd_mxptr[M03]) >> 14);
		*XY++ = short((phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2] + phd_mxptr[M13]) >> 14);
		*Z++ = (phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2] + phd_mxptr[M23]) >> 14;
		Z++;

		offsets[0] = ripple->size;
		offsets[1] = 0;
		offsets[2] = -ripple->size;
		*XY++ = short((phd_mxptr[M00] * offsets[0] + phd_mxptr[M01] * offsets[1] + phd_mxptr[M02] * offsets[2] + phd_mxptr[M03]) >> 14);
		*XY++ = short((phd_mxptr[M10] * offsets[0] + phd_mxptr[M11] * offsets[1] + phd_mxptr[M12] * offsets[2] + phd_mxptr[M13]) >> 14);
		*Z++ = (phd_mxptr[M20] * offsets[0] + phd_mxptr[M21] * offsets[1] + phd_mxptr[M22] * offsets[2] + phd_mxptr[M23]) >> 14;
		Z++;

		phd_PopMatrix();

		XY = (short*)&scratchpad[0];
		Z = (long*)&scratchpad[256];

		if (ripple->flags & 0x20)
			sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index];
		else
			sprite = &spriteinfo[objects[DEFAULT_SPRITES].mesh_index + 9];

		x1 = *XY++;
		y1 = *XY++;
		z1 = *Z++;
		Z++;

		x2 = *XY++;
		y2 = *XY++;
		z2 = *Z++;
		Z++;

		x3 = *XY++;
		y3 = *XY++;
		z3 = *Z++;
		Z++;

		x4 = *XY++;
		y4 = *XY++;
		z4 = *Z++;
		Z++;

		setXYZ4(v, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, clipflags);

		if (ripple->flags & 0x10)
		{
			if (ripple->flags & 0x20)
			{
				if (ripple->init)
				{
					r = (ripple->init >> 1) << 1;
					g = 0;
					b = (ripple->init >> 4) << 1;
				}
				else
				{
					r = (ripple->life >> 1) << 1;
					g = 0;
					b = (ripple->life >> 4) << 1;
				}
			}
			else
			{
				if (ripple->init)
				{
					r = ripple->init << 1;
					g = ripple->init << 1;
					b = ripple->init << 1;
				}
				else
				{
					r = ripple->life << 1;
					g = ripple->life << 1;
					b = ripple->life << 1;
				}
			}
		}
		else
		{
			if (ripple->init)
			{
				r = ripple->init << 2;
				g = ripple->init << 2;
				b = ripple->init << 2;
			}
			else
			{
				r = ripple->life << 2;
				g = ripple->life << 2;
				b = ripple->life << 2;
			}
		}

		if (r > 255)
			r = 255;

		if (g > 255)
			g = 255;

		if (b > 255)
			b = 255;

		c0 = RGBA(r, g, b, 0xFF);

		v[0].color = c0;
		v[1].color = c0;
		v[2].color = c0;
		v[3].color = c0;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v2 = sprite->y1;
		tex.v3 = sprite->y2;
		tex.u3 = sprite->x2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 1);
	}
}

void S_DrawFireSparks(long size, long life)
{
	FIRE_SPARKS* sptr;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	ulong r, g, b, col;
	long newSize, s, c, sx1, cx1, sx2, cx2;
	long dx, dy, dz, x1, y1, x2, y2, x3, y3, x4, y4;
	short ang;

	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];

	for (int i = 0; i < 20; i++)
	{
		sptr = &fire_spark[i];

		if (!sptr->On)
			continue;

		dx = sptr->x >> (2 - size);
		dy = sptr->y >> (2 - size);
		dz = sptr->z >> (2 - size);

		if (dx < -0x5000 || dx > 0x5000 || dy < -0x5000 || dy > 0x5000 || dz < -0x5000 || dz > 0x5000)
			continue;

		offsets[0] = (short)dx;
		offsets[1] = (short)dy;
		offsets[2] = (short)dz;
		pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
		perspz = f_persp / (float)pos.z;
		XY[0] = short(float(pos.x * perspz + f_centerx));
		XY[1] = short(float(pos.y * perspz + f_centery));
		Z[0] = pos.z >> 14;


		if (Z[0] <= 0 || Z[0] >= 0x5000)
			continue;

		newSize = (((phd_persp * sptr->Size) << 2) / Z[0]) >> (2 - size);

		if (newSize > (sptr->Size << 2))
			newSize = (sptr->Size << 2);
		else if (newSize < 4)
			newSize = 4;

		newSize >>= 1;

		if (XY[0] + newSize < phd_winxmin || XY[0] - newSize >= phd_winxmax || XY[1] + newSize < phd_winymin || XY[1] - newSize >= phd_winymax)
			continue;

		if (sptr->Flags & 0x10)
		{
			ang = sptr->RotAng << 1;
			s = rcossin_tbl[ang];
			c = rcossin_tbl[ang + 1];
			sx1 = (-newSize * s) >> 12;
			cx1 = (-newSize * c) >> 12;
			sx2 = (newSize * s) >> 12;
			cx2 = (newSize * c) >> 12;
			x1 = XY[0] + (sx1 - cx1);
			y1 = XY[1] + sx1 + cx1;
			x2 = XY[0] + (sx2 - cx1);
			y2 = XY[1] + sx1 + cx2;
			x3 = XY[0] + (sx2 - cx2);
			y3 = XY[1] + sx2 + cx2;
			x4 = XY[0] + (sx1 - cx2);
			y4 = XY[1] + sx2 + cx1;
			setXY4(v, x1, y1, x2, y2, x3, y3, x4, y4, Z[0], clipflags);
		}
		else
		{
			x1 = XY[0] - newSize;
			x2 = XY[0] + newSize;
			y1 = XY[1] - newSize;
			y2 = XY[1] + newSize;
			setXY4(v, x1, y1, x2, y1, x2, y2, x1, y2, Z[0], clipflags);
		}

		sprite = &spriteinfo[sptr->Def];

		if (Z[0] <= 0x3000)
		{
			r = sptr->R;
			g = sptr->G;
			b = sptr->B;
		}
		else
		{
			r = ((0x5000 - Z[0]) * sptr->R) >> 13;
			g = ((0x5000 - Z[0]) * sptr->G) >> 13;
			b = ((0x5000 - Z[0]) * sptr->B) >> 13;
		}

		r = (r * life) >> 8;
		g = (g * life) >> 8;
		b = (b * life) >> 8;
		col = RGBA(r, g, b, 0xFF);

		v[0].color = col;
		v[1].color = col;
		v[2].color = col;
		v[3].color = col;
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v2 = sprite->y1;
		tex.u3 = sprite->x2;
		tex.v3 = sprite->y2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
	}
}

void S_DrawSmokeSparks()
{
	SMOKE_SPARKS* sptr;
	SPRITESTRUCT* sprite;
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	PHD_VECTOR pos;
	long* Z;
	short* XY;
	short* offsets;
	float perspz;
	long is_mirror, size, col, s, c, ss, cs, sm, cm;
	long dx, dy, dz, x1, y1, x2, y2, x3, y3, x4, y4;
	short ang;

	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);
	XY = (short*)&scratchpad[0];
	Z = (long*)&scratchpad[256];
	offsets = (short*)&scratchpad[512];
	is_mirror = 0;
	sptr = &smoke_spark[0];

	for (int i = 0; i < 32; i++)
	{
		if (!sptr->On)
		{
			sptr++;
			continue;
		}

		if (sptr->mirror && !is_mirror)
			is_mirror = 1;
		else
			is_mirror = 0;

		dx = sptr->x - lara_item->pos.x_pos;
		dy = sptr->y - lara_item->pos.y_pos;
		dz = sptr->z - lara_item->pos.z_pos;

		if (is_mirror)
			dz = 2 * gfMirrorZPlane - lara_item->pos.z_pos - sptr->z;

		if (dx < -0x5000 || dx > 0x5000 || dy < -0x5000 || dy > 0x5000 || dz < -0x5000 || dz > 0x5000)
		{
			if (!is_mirror)
				sptr++;

			continue;
		}

		offsets[0] = (short)dx;
		offsets[1] = (short)dy;
		offsets[2] = (short)dz;
		pos.x = offsets[0] * phd_mxptr[M00] + offsets[1] * phd_mxptr[M01] + offsets[2] * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = offsets[0] * phd_mxptr[M10] + offsets[1] * phd_mxptr[M11] + offsets[2] * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = offsets[0] * phd_mxptr[M20] + offsets[1] * phd_mxptr[M21] + offsets[2] * phd_mxptr[M22] + phd_mxptr[M23];
		perspz = f_persp / (float)pos.z;
		XY[0] = short(float(pos.x * perspz + f_centerx));
		XY[1] = short(float(pos.y * perspz + f_centery));
		Z[0] = pos.z >> 14;

		if (Z[0] <= 0 || Z[0] >= 0x5000)
		{
			if (!is_mirror)
				sptr++;

			continue;
		}

		size = ((phd_persp * sptr->Size) << 2) / Z[0];

		if (size > sptr->Size << 2)
			size = sptr->Size << 2;
		else if (size < 4)
			size = 4;

		size >>= 1;

		if (XY[0] + size < phd_winxmin || XY[0] - size >= phd_winxmax || XY[1] + size < phd_winymin || XY[1] - size >= phd_winymax)
		{
			if (!is_mirror)
				sptr++;

			continue;
		}

		if (sptr->Flags & 0x10)
		{
			ang = sptr->RotAng << 1;
			s = rcossin_tbl[ang];
			c = rcossin_tbl[ang + 1];
			ss = (s * size) >> 12;
			cs = (c * size) >> 12;
			sm = (s * -size) >> 12;
			cm = (c * -size) >> 12;

			x1 = sm + XY[0] - cm;
			y1 = sm + XY[1] + cm;
			x2 = XY[0] - cm + ss;
			y2 = sm + XY[1] + cs;
			x3 = ss + XY[0] - cs;
			y3 = cs + XY[1] + ss;
			x4 = sm + XY[0] - cs;
			y4 = ss + XY[1] + cm;

			setXY4(v, x1, y1, x2, y2, x3, y3, x4, y4, Z[0], clipflags);
		}
		else
		{
			x1 = XY[0] - size;
			y1 = XY[1] - size;
			x2 = XY[0] + size;
			y2 = XY[1] + size;
			setXY4(v, x1, y1, x2, y1, x2, y2, x1, y2, Z[0], clipflags);
		}

		sprite = &spriteinfo[sptr->Def];

		if (Z[0] <= 0x3000)
			col = sptr->Shade;
		else
			col = ((0x5000 - Z[0]) * sptr->Shade) >> 13;

		v[0].color = RGBA(col, col, col, 0xFF);
		v[1].color = RGBA(col, col, col, 0xFF);
		v[2].color = RGBA(col, col, col, 0xFF);
		v[3].color = RGBA(col, col, col, 0xFF);
		v[0].specular = 0xFF000000;
		v[1].specular = 0xFF000000;
		v[2].specular = 0xFF000000;
		v[3].specular = 0xFF000000;
		tex.drawtype = 2;
		tex.flag = 0;
		tex.tpage = sprite->tpage;
		tex.u1 = sprite->x1;
		tex.v1 = sprite->y1;
		tex.u2 = sprite->x2;
		tex.v3 = sprite->y2;
		tex.v2 = sprite->y1;
		tex.u3 = sprite->x2;
		tex.u4 = sprite->x1;
		tex.v4 = sprite->y2;
		AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

		if (!is_mirror)
			sptr++;
	}

	phd_PopMatrix();
}

long MapCompare(MAP_STRUCT* a, MAP_STRUCT* b)
{
	static long max = -99999999;
	static long min = 99999999;
	long af, bf;

	af = room[a->room_number].minfloor;
	bf = room[b->room_number].minfloor;

	if (af > max)
		max = af;

	if (af < min)
		min = af;

	if (bf > max)
		max = bf;

	if (bf < min)
		min = bf;

	if (af > bf)
		return -1;

	if (af < bf)
		return 1;

	return 0;
}

void MapInit()
{
	static MAP_VECTOR* xVPs[400];
	static MAP_VECTOR* yVPs[400];
	MAP_VECTOR* p;
	MAP_VECTOR vs[1024];
	D3DVECTOR v[1024];
	short* data;
	long n, nVtx, found, lp, lp2, lp3;
	long x1, y1, x2, y2;
	short vList[512];

	data = (short*)MapData;

	for (int i = 0; i < number_rooms; i++)
	{
		lp2 = 0;

		while(1)
		{
			x1 = *data++;
			y1 = *data++;

			if (x1 == -1 && y1 == -1)
				return;

			x2 = *data++;
			y2 = *data++;

			vs[lp2].x1 = x1;
			vs[lp2].y1 = y1;
			vs[lp2].x2 = x2;
			vs[lp2].y2 = y2;
			lp2++;
		}

		memset(xVPs, 0, sizeof(xVPs));
		memset(yVPs, 0, sizeof(yVPs));

		for (lp = 0; lp < lp2; lp++)
		{
			if (vs[lp].y1 == vs[lp].y2)
				xVPs[(20 * (vs[lp].y1 >> 10)) + (vs[lp].x1 >> 10)] = &vs[lp];
			else
				yVPs[(20 * (vs[lp].x1 >> 10)) + (vs[lp].y1 >> 10)] = &vs[lp];
		}

		n = 0;
		nVtx = 0;

		for (lp = 0; lp < 20; lp++)
		{
			for (lp2 = 0; lp2 < 20; lp2++)
			{
				p = xVPs[(20 * lp) + lp2];

				if (!p)
					continue;

				x1 = p->x1;
				y1 = p->y1;
				x2 = p->x2;
				y2 = p->y2;

				while (lp2 <= 20)
				{
					p = yVPs[(20 * lp) + 1 + lp2];

					if (!p)
						break;

					x2 = p->x2;
					y2 = p->y2;
					lp2++;
				}

				v[n].x = (float)y1;
				v[n].y = (float)x1;
				v[n].z = 0;
				v[n + 1].x = (float)y2;
				v[n + 1].y = (float)x2;
				v[n + 1].z = 0;
				n += 2;
			}
		}

		for (lp = 0; lp < 20; lp++)
		{
			for (lp2 = 0; lp2 < 20; lp2++)
			{
				p = yVPs[(20 * lp) + lp2];

				if (!p)
					continue;

				x1 = p->x1;
				y1 = p->y1;
				x2 = p->x2;
				y2 = p->y2;

				while (lp2 <= 20)
				{
					p = yVPs[(20 * lp) + 1 + lp2];

					if (!p)
						break;

					x2 = p->x2;
					y2 = p->y2;
					lp2++;
				}

				v[n].x = (float)y1;
				v[n].y = (float)x1;
				v[n].z = 0;
				v[n + 1].x = (float)y2;
				v[n + 1].y = (float)x2;
				v[n + 1].z = 0;
				n += 2;
			}
		}

		nVtx = n;
		Map[i].nLines = n >> 1;

		for (lp = 0; lp < Map[i].nLines; lp++)
		{
			Map[i].lines[lp << 1] = short(lp << 1);
			Map[i].lines[(lp << 1) + 1] = short((lp << 1) + 1);
		}

		n = 0;

		for (lp = 0; lp < nVtx; lp++)
		{
			found = 0;

			for (lp2 = 0; lp2 < n; lp2++)
			{
				if (v[lp].x == v[vList[lp2]].x)
				{
					found = 1;
					break;
				}
			}

			if (found)
			{
				for (lp3 = 0; lp3 < Map[i].nLines << 1; lp3++)
				{
					if (Map[i].lines[lp3] == lp)
						Map[i].lines[lp3] = (short)lp2;
				}
			}
			else
			{
				vList[n] = (short)lp;

				for (lp3 = 0; lp3 < 2 * Map[i].nLines; lp3++)
				{
					if (Map[i].lines[lp3] == lp)
						Map[i].lines[lp3] = (short)n;
				}

				n++;
			}
		}

		Map[i].nVtx = n;

		for (lp = 0; lp < n; lp++)
		{
			Map[i].vtx[lp].x = (long)v[vList[lp]].x;
			Map[i].vtx[lp].y = (long)v[vList[lp]].y;
			Map[i].vtx[lp].z = (long)v[vList[lp]].z;
		}

		Map[i].visited = 1;
		Map[i].room_number = i;
	}
}
