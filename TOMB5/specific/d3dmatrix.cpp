#include "../tomb5/pch.h"
#include "d3dmatrix.h"
#include "dxshell.h"
#include "winmain.h"
#include "3dmath.h"

D3DMATRIX D3DMView;
D3DMATRIX D3DCameraMatrix;
D3DMATRIX D3DInvCameraMatrix;

static D3DMATRIX D3DMWorld;
static D3DMATRIX D3DMProjection;

void SetD3DMatrixF(LPD3DMATRIX dest, float* src)
{
	D3DIdentityMatrix(dest);
	dest->_11 = src[M00];
	dest->_12 = src[M10];
	dest->_13 = src[M20];
	dest->_21 = src[M01];
	dest->_22 = src[M11];
	dest->_23 = src[M21];
	dest->_31 = src[M02];
	dest->_32 = src[M12];
	dest->_33 = src[M22];
	dest->_41 = src[M03];
	dest->_42 = src[M13];
	dest->_43 = src[M23];
}

LPD3DMATRIX D3DIdentityMatrix(LPD3DMATRIX matrix)
{
	matrix->_11 = 1;
	matrix->_12 = 0;
	matrix->_13 = 0;
	matrix->_14 = 0;
	matrix->_21 = 0;
	matrix->_22 = 1;
	matrix->_23 = 0;
	matrix->_24 = 0;
	matrix->_31 = 0;
	matrix->_32 = 0;
	matrix->_33 = 1;
	matrix->_34 = 0;
	matrix->_41 = 0;
	matrix->_42 = 0;
	matrix->_43 = 0;
	matrix->_44 = 1;
	return matrix;
}

void SaveD3DCameraMatrix()
{
	D3DIdentityMatrix(&D3DCameraMatrix);
	D3DCameraMatrix._11 = aMXPtr[M00];
	D3DCameraMatrix._12 = aMXPtr[M10];
	D3DCameraMatrix._13 = aMXPtr[M20];
	D3DCameraMatrix._21 = aMXPtr[M01];
	D3DCameraMatrix._22 = aMXPtr[M11];
	D3DCameraMatrix._23 = aMXPtr[M21];
	D3DCameraMatrix._31 = aMXPtr[M02];
	D3DCameraMatrix._32 = aMXPtr[M12];
	D3DCameraMatrix._33 = aMXPtr[M22];
	D3DCameraMatrix._41 = aMXPtr[M03];
	D3DCameraMatrix._42 = aMXPtr[M13];
	D3DCameraMatrix._43 = aMXPtr[M23];
}

void SetD3DViewMatrix()
{
	D3DIdentityMatrix(&D3DMView);
	D3DMView._11 = (float)phd_mxptr[M00] / 16384;
	D3DMView._12 = (float)phd_mxptr[M10] / 16384;
	D3DMView._13 = (float)phd_mxptr[M20] / 16384;
	D3DMView._21 = (float)phd_mxptr[M01] / 16384;
	D3DMView._22 = (float)phd_mxptr[M11] / 16384;
	D3DMView._23 = (float)phd_mxptr[M21] / 16384;
	D3DMView._31 = (float)phd_mxptr[M02] / 16384;
	D3DMView._32 = (float)phd_mxptr[M12] / 16384;
	D3DMView._33 = (float)phd_mxptr[M22] / 16384;
	D3DMView._41 = float(phd_mxptr[M03] >> 14);
	D3DMView._42 = float(phd_mxptr[M13] >> 14);
	D3DMView._43 = float(phd_mxptr[M23] >> 14);
	DXAttempt(App.dx.lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &D3DMView));
}

void SetD3DMatrix(D3DMATRIX* mx, long* imx)
{
	D3DIdentityMatrix(mx);
	mx->_11 = (float)imx[M00] / 16384;
	mx->_12 = (float)imx[M10] / 16384;
	mx->_13 = (float)imx[M20] / 16384;
	mx->_21 = (float)imx[M01] / 16384;
	mx->_22 = (float)imx[M11] / 16384;
	mx->_23 = (float)imx[M21] / 16384;
	mx->_31 = (float)imx[M02] / 16384;
	mx->_32 = (float)imx[M12] / 16384;
	mx->_33 = (float)imx[M22] / 16384;
	mx->_41 = float(imx[M03] >> 14);
	mx->_42 = float(imx[M13] >> 14);
	mx->_43 = float(imx[M23] >> 14);
}

void S_InitD3DMatrix()
{
	D3DIdentityMatrix(&D3DMWorld);
	D3DIdentityMatrix(&D3DMProjection);
	D3DMProjection._22 = -1;
	DXAttempt(App.dx.lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &D3DMWorld));
	DXAttempt(App.dx.lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &D3DMProjection));
}

D3DVECTOR* D3DNormalise(D3DVECTOR* vec)
{
	float val;

	if (vec->x != 0 || vec->y != 0 || vec->z != 0)
	{
		val = 1.0F / sqrt(SQUARE(vec->x) + SQUARE(vec->y) + SQUARE(vec->z));
		vec->x = val * vec->x;
		vec->y = val * vec->y;
		vec->z = val * vec->z;
	}

	return vec;
}

D3DVECTOR* D3DVSubtract(D3DVECTOR* out, D3DVECTOR* a, D3DVECTOR* b)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
	return out;
}

D3DVECTOR* D3DVAdd(D3DVECTOR* out, D3DVECTOR* a, D3DVECTOR* b)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
	return out;
}

D3DVECTOR* D3DCrossProduct(D3DVECTOR* out, D3DVECTOR* a, D3DVECTOR* b)
{
	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
	return out;
}

float D3DDotProduct(D3DVECTOR* a, D3DVECTOR* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

D3DMATRIX* D3DSetTranslate(D3DMATRIX* mx, float x, float y, float z)
{
	mx->_41 = x;
	mx->_42 = y;
	mx->_43 = z;
	return mx;
}

D3DMATRIX* D3DSetRotateX(D3DMATRIX* mx, float ang)
{
	float s, c;

	s = sin(ang);
	c = cos(ang);
	mx->_22 = c;
	mx->_23 = -s;
	mx->_32 = s;
	mx->_33 = c;
	return mx;
}

D3DMATRIX* D3DSetRotateY(D3DMATRIX* mx, float ang)
{
	float s, c;

	s = sin(ang);
	c = cos(ang);
	mx->_11 = c;
	mx->_13 = s;
	mx->_31 = -s;
	mx->_33 = c;
	return mx;
}

D3DMATRIX* D3DSetRotateZ(D3DMATRIX* mx, float ang)
{
	float s, c;

	s = sin(ang);
	c = cos(ang);
	mx->_11 = c;
	mx->_12 = -s;
	mx->_21 = s;
	mx->_22 = c;
	return mx;
}

D3DMATRIX* D3DSetScale(D3DMATRIX* mx, float scale)
{
	mx->_11 = scale;
	mx->_22 = scale;
	mx->_33 = scale;
	return mx;
}

D3DMATRIX* D3DZeroMatrix(D3DMATRIX* mx)
{
	mx->_11 = 0;
	mx->_12 = 0;
	mx->_13 = 0;
	mx->_14 = 0;
	mx->_21 = 0;
	mx->_22 = 0;
	mx->_23 = 0;
	mx->_24 = 0;
	mx->_31 = 0;
	mx->_32 = 0;
	mx->_33 = 0;
	mx->_34 = 0;
	mx->_41 = 0;
	mx->_42 = 0;
	mx->_43 = 0;
	mx->_44 = 0;
	return mx;
}

D3DMATRIX* D3DViewMatrix(D3DMATRIX* mx, D3DVECTOR* eye, D3DVECTOR* target, D3DVECTOR* GlobalUp)
{
	D3DVECTOR f;	//forward vector
	D3DVECTOR r;	//right vector
	D3DVECTOR u;	//up vector

	D3DIdentityMatrix(mx);

	D3DVSubtract(&f, target, eye);
	D3DNormalise(&f);
	D3DCrossProduct(&r, GlobalUp, &f);
	D3DCrossProduct(&u, &f, &r);
	D3DNormalise(&r);
	D3DNormalise(&u);

	mx->_11 = r.x;
	mx->_21 = -r.y;
	mx->_31 = r.z;

	mx->_12 = u.x;
	mx->_22 = -u.y;
	mx->_32 = u.z;

	mx->_13 = f.x;
	mx->_23 = -f.y;
	mx->_33 = f.z;

	mx->_41 = D3DDotProduct(&r, eye);
	mx->_42 = D3DDotProduct(&u, eye);
	mx->_43 = D3DDotProduct(&f, eye);

	return mx;
}

D3DMATRIX* D3DProjectionMatrix(D3DMATRIX* mx, float hFov, float vFov, float nPlane, float fPlane)
{
	float s, c, n;

	s = sin(vFov * 0.5F);
	c = cos(hFov * 0.5F);
	n = s / (1.0F - nPlane / fPlane);
	mx->_11 = c;
	mx->_22 = -c;
	mx->_33 = n;
	mx->_34 = s;
	mx->_43 = -(n * nPlane);
	return mx;
}

void D3DTransform(D3DVECTOR* vec, D3DMATRIX* mx)
{
	float x, y, z;

	x = mx->_11 * vec->x + mx->_21 * vec->y + mx->_31 * vec->z;
	y = mx->_12 * vec->x + mx->_22 * vec->y + mx->_32 * vec->z;
	z = mx->_13 * vec->x + mx->_23 * vec->y + mx->_33 * vec->z;
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

void D3DTranspose(D3DVECTOR* vec, D3DMATRIX* mx)
{
	float x, y, z;

	x = mx->_11 * vec->x + mx->_12 * vec->y + mx->_13 * vec->z;
	y = mx->_21 * vec->x + mx->_22 * vec->y + mx->_23 * vec->z;
	z = mx->_31 * vec->x + mx->_32 * vec->y + mx->_33 * vec->z;
	vec->x = x;
	vec->y = y;
	vec->z = z;
}

LPD3DMATRIX D3DMultMatrix(LPD3DMATRIX d, LPD3DMATRIX a, LPD3DMATRIX b)
{
	d->_11 = a->_11 * b->_11;
	d->_11 += a->_12 * b->_21;
	d->_11 += a->_13 * b->_31;
	d->_11 += a->_14 * b->_41;

	d->_12 = a->_11 * b->_12;
	d->_12 += a->_12 * b->_22;
	d->_12 += a->_13 * b->_32;
	d->_12 += a->_14 * b->_42;

	d->_13 = a->_11 * b->_13;
	d->_13 += a->_12 * b->_23;
	d->_13 += a->_13 * b->_33;
	d->_13 += a->_14 * b->_43;

	d->_14 = a->_11 * b->_14;
	d->_14 += a->_12 * b->_24;
	d->_14 += a->_13 * b->_34;
	d->_14 += a->_14 * b->_44;

	d->_21 = a->_21 * b->_11;
	d->_21 += a->_22 * b->_21;
	d->_21 += a->_23 * b->_31;
	d->_21 += a->_24 * b->_41;

	d->_22 = a->_21 * b->_12;
	d->_22 += a->_22 * b->_22;
	d->_22 += a->_23 * b->_32;
	d->_22 += a->_24 * b->_42;

	d->_23 = a->_21 * b->_13;
	d->_23 += a->_22 * b->_23;
	d->_23 += a->_23 * b->_33;
	d->_23 += a->_24 * b->_43;

	d->_24 = a->_21 * b->_14;
	d->_24 += a->_22 * b->_24;
	d->_24 += a->_23 * b->_34;
	d->_24 += a->_24 * b->_44;

	d->_31 = a->_31 * b->_11;
	d->_31 += a->_32 * b->_21;
	d->_31 += a->_33 * b->_31;
	d->_31 += a->_34 * b->_41;

	d->_32 = a->_31 * b->_12;
	d->_32 += a->_32 * b->_22;
	d->_32 += a->_33 * b->_32;
	d->_32 += a->_34 * b->_42;

	d->_33 = a->_31 * b->_13;
	d->_33 += a->_32 * b->_23;
	d->_33 += a->_33 * b->_33;
	d->_33 += a->_34 * b->_43;

	d->_34 = a->_31 * b->_14;
	d->_34 += a->_32 * b->_24;
	d->_34 += a->_33 * b->_34;
	d->_34 += a->_34 * b->_44;

	d->_41 = a->_41 * b->_11;
	d->_41 += a->_42 * b->_21;
	d->_41 += a->_43 * b->_31;
	d->_41 += a->_44 * b->_41;

	d->_42 = a->_41 * b->_12;
	d->_42 += a->_42 * b->_22;
	d->_42 += a->_43 * b->_32;
	d->_42 += a->_44 * b->_42;

	d->_43 = a->_41 * b->_13;
	d->_43 += a->_42 * b->_23;
	d->_43 += a->_43 * b->_33;
	d->_43 += a->_44 * b->_43;

	d->_44 = a->_41 * b->_14;
	d->_44 += a->_42 * b->_24;
	d->_44 += a->_43 * b->_34;
	d->_44 += a->_44 * b->_44;

	return d;
}
