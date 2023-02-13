#pragma once
#include "../global/types.h"

void AlterFOV(short fov);
void gte_sttr(PHD_VECTOR* vec);
void aInitMatrix();
void aSetViewMatrix();
void aSetTrans(long x, long y, long z);
void aTranslateAbs(long x, long y, long z);
void aUnitMatrixByMat(float* matrix);
void aPushUnitMatrix();
long aTranslateRel(long x, long y, long z);
void aRotX(short angle);
void aRotY(short angle);
void aRotZ(short angle);
void aRotYXZPack(long angles);
void aRotYXZ(short y, short x, short z);
void phd_PushMatrix();
void phd_SetTrans(long x, long y, long z);
void phd_PushUnitMatrix();
long phd_TranslateRel(long x, long y, long z);
void phd_RotX(short angle);
void phd_RotY(short angle);
void phd_RotZ(short angle);
void phd_RotYXZpack(long angles);
void phd_RotYXZ(short y, short x, short z);
void phd_TranslateAbs(long x, long y, long z);
void phd_GetVectorAngles(long x, long y, long z, short* angles);
void phd_TransposeMatrix();
void phd_LookAt(long xsrc, long ysrc, long zsec, long xtar, long ytar, long ztar, short roll);
void aLookAt(float xsrc, float ysrc, float zsrc, float xtar, float ytar, float ztar, long roll);
void aOuterProduct(FVECTOR* v1, FVECTOR* v2, FVECTOR* dest);
void aVectorNormal(FVECTOR* s, FVECTOR* v);
void aPerpVectors(FVECTOR* a, FVECTOR* b, FVECTOR* c);
void aPointCameraByVector(float* mx, FCAMERA* cam);
void aPointCamera(FCAMERA* cam);
void aScaleCurrentMatrix(PHD_VECTOR* vec);
void ScaleCurrentMatrix(PHD_VECTOR* vec);
void SetupZRange(long znear, long zfar);
void InitWindow(long x, long y, long w, long h, long znear, long zfar, long fov, long a, long b);
long phd_atan(long x, long y);
ulong phd_sqrt(ulong num);
ulong mGetAngle(long x, long z, long x1, long z1);
void phd_GenerateW2V(PHD_3DPOS* viewPos);

extern FCAMERA aCamera;
extern FVECTOR aCamPos;
extern FVECTOR aCamTar;
extern FVECTOR aCamDir;
extern PHD_VECTOR CamPos;
extern SVECTOR CamRot;

extern float one;
extern float mone;
extern float FogStart;
extern float FogEnd;

extern float f_centerx;
extern float f_centery;
extern float f_top;
extern float f_left;
extern float f_bottom;
extern float f_right;
extern float f_znear;
extern float f_zfar;
extern float f_mznear;
extern float f_mzfar;
extern float f_persp;
extern float f_mpersp;
extern float f_oneopersp;
extern float f_moneopersp;
extern float f_perspoznear;
extern float f_mperspoznear;
extern float f_moneoznear;
extern float f_a;
extern float f_b;
extern float f_boo;

extern float fcossin_tbl[65536];

extern float* aMXPtr;
extern float aFMatrixStack[20 * indices_count];

extern long* phd_mxptr;
extern long w2v_matrix[indices_count];
extern long matrix_stack[20 * indices_count];

extern long phd_winheight;
extern long phd_winwidth;
extern long phd_centerx;
extern long phd_centery;
extern long phd_top;
extern long phd_left;
extern long phd_bottom;
extern long phd_right;
extern long phd_znear;
extern long phd_zfar;
extern long phd_persp;
extern short phd_winxmax;
extern short phd_winxmin;
extern short phd_winymax;
extern short phd_winymin;

__inline short phd_sin(long angle)
{
	angle >>= 3;
	return 4 * rcossin_tbl[angle & 0x1FFE];
}

__inline short phd_cos(long angle)
{
	angle >>= 3;
	return 4 * rcossin_tbl[(angle & 0x1FFE) + 1];
}

__inline float fSin(long angle)
{
	return fcossin_tbl[(ushort)angle];
}

__inline float fCos(long angle)
{
	return fcossin_tbl[ushort(angle + 0x4000)];
}

__inline void phd_PopMatrix()
{
	phd_mxptr -= indices_count;
	aMXPtr -= indices_count;
}
