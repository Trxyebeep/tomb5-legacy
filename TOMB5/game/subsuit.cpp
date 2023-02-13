#include "../tomb5/pch.h"
#include "subsuit.h"
#include "control.h"
#include "minisub.h"
#include "sound.h"
#include "delstuff.h"
#include "camera.h"
#include "items.h"
#include "objects.h"
#include "effect2.h"
#include "../specific/function_stubs.h"
#include "../specific/3dmath.h"
#include "deltapak.h"
#include "tomb4fx.h"
#include "../specific/input.h"
#include "lara.h"

SUBSUIT_INFO subsuit;
char SubHitCount = 0;

static SVECTOR Eng1 = { 0, 64, 0, 0 };
static SVECTOR Eng2 = { 0, 0, 0, 0 };
static char BVols[8] = { 4, 5, 6, 8, 11, 14, 18, 23 };

static short BreathCount = 0;
static short BreathDelay = 16;
static short ZeroStressCount = 0;

void DoSubsuitStuff()
{
	PHD_VECTOR s;
	PHD_VECTOR d;
	long pitch, anx, vol;

	if (CheckCutPlayed(40))
		TriggerAirBubbles();

	if (SubHitCount)
		SubHitCount--;

	anx = lara.Anxiety;

	if (lara.Anxiety > 127)
		anx = 127;

	BreathCount++;

	if (BreathCount >= BreathDelay)
	{
		pitch = ((anx & 0x70) << 8) + 0x8000;
		anx &= 0x70;
		vol = BVols[anx >> 4];
		ZeroStressCount = (short)anx;

		if (lara_item->hit_points > 0)
			SoundEffect(SFX_LARA_SUB_BREATHE, 0, (pitch << 8) | (vol << 8) | SFX_ALWAYS | SFX_SETPITCH | SFX_SETVOL);

		BreathCount = short(-40 - (30 * (128 - anx) >> 7));

		if (lara.Anxiety)
		{
			anx = lara.Anxiety;

			if (anx > 128)
				anx -= 16;
			else
				anx -= 4;

			if (anx < 0)
				anx = 0;

			lara.Anxiety = (uchar)anx;
			BreathDelay = 0;
		}
		else if (BreathDelay < 16)
			BreathDelay += 2;
	}

	s.x = 0;
	s.y = -1024;
	s.z = -128;
	GetLaraJointPos(&s, 7);

	d.x = 0;
	d.y = -20480;
	d.z = -128;
	GetLaraJointPos(&d, 7);

	LaraTorch(&s, &d, lara_item->pos.y_rot, 255);
	TriggerEngineEffects();

	if (lara.ChaffTimer)
		lara.ChaffTimer--;

	if (dbinput & IN_SPRINT && !lara.ChaffTimer)
		FireChaff();
}

void FireChaff()
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	long h;
	short item_number;

	if (!lara.puzzleitems[0])
		return;

	item_number = CreateItem();

	if (item_number == NO_ITEM)
		return;

	SoundEffect(SFX_UNDERWATER_CHAFF, &lara_item->pos, SFX_ALWAYS);

	item = &items[item_number];
	item->object_number = CHAFF;
	item->shade = -15856;
	lara.puzzleitems[0]--;

	pos.x = 0;
	pos.y = -112;
	pos.z = -112;
	GetLaraJointPos(&pos, 7);

	item->room_number = lara_item->room_number;
	floor = GetFloor(pos.x, pos.y, pos.z, &item->room_number);
	h = GetHeight(floor, pos.x, pos.y, pos.z);

	if (h >= pos.y)
	{
		item->pos.x_pos = pos.x;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = pos.z;
	}
	else
	{
		item->pos.x_pos = lara_item->pos.x_pos;
		item->pos.y_pos = pos.y;
		item->pos.z_pos = lara_item->pos.z_pos;
		item->room_number = lara_item->room_number;
	}

	InitialiseItem(item_number);
	item->pos.x_rot = 0;
	item->pos.y_rot = lara_item->pos.y_rot - 0x8000;
	item->pos.z_rot = 0;
	item->speed = 32;
	item->fallspeed = -128;
	AddActiveItem(item_number);
	lara.ChaffTimer = 150;

	for (int i = 0; i < 8; i++)
	{
		pos.x = 0;
		pos.y = (GetRandomControl() & 0x1F) - 128;
		pos.z = -112;
		GetLaraJointPos(&pos, 7);

		pos2.x = (GetRandomControl() & 0xFF) - 128;
		pos2.y = GetRandomControl() & (((i + 1) << 7) - 1);
		pos2.z = -112 - (GetRandomControl() & (((i + 1) << 6) - 1));
		GetLaraJointPos(&pos2, 7);

		TriggerTorpedoSteam(&pos, &pos2, 1);
	}
}

void TriggerAirBubbles()
{
	SPARKS* sptr;
	PHD_VECTOR pos1;
	PHD_VECTOR pos2;
	long size;

	pos1.x = 0;
	pos1.y = -192;
	pos1.z = -160;
	GetLaraJointPos(&pos1, 7);

	pos2.x = 0;
	pos2.y = -192;
	pos2.z = -512 - (GetRandomControl() & 0x7F);
	GetLaraJointPos(&pos2, 7);

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 32;
	sptr->sG = 32;
	sptr->sB = 32;
	sptr->dR = 160;
	sptr->dG = 160;
	sptr->dB = 160;
	sptr->ColFadeSpeed = 2;
	sptr->FadeToBlack = 6;
	sptr->TransType = 2;
	sptr->Life = (GetRandomControl() & 7) + 16;
	sptr->sLife = sptr->Life;

	sptr->x = (GetRandomControl() & 0x1F) + pos1.x - 16;
	sptr->y = (GetRandomControl() & 0x1F) + pos1.y - 16;
	sptr->z = (GetRandomControl() & 0x1F) + pos1.z - 16;
	sptr->Xvel = short(pos2.x - pos1.x + ((GetRandomControl() & 0x7F) - 64));
	sptr->Yvel = short(pos2.y - pos1.y + ((GetRandomControl() & 0x7F) - 64));
	sptr->Zvel = short(pos2.z - pos1.z + ((GetRandomControl() & 0x7F) - 64));

	sptr->Friction = 0; 
	sptr->Def = objects[DEFAULT_SPRITES].mesh_index + 17;
	sptr->MaxYvel = 0;
	sptr->Gravity = -4 - (GetRandomControl() & 3);
	sptr->Scalar = 1;
	sptr->Flags = 26;
	sptr->RotAng = GetRandomControl() & 0xFFF;
	sptr->RotAdd = (GetRandomControl() & 0xF) - 8;
	size = 16 + (GetRandomControl() & 15);
	sptr->Size = (uchar)size;
	sptr->sSize = sptr->Size;
	sptr->dSize = sptr->Size << 1;
}

void GetLaraJointPosRot(PHD_VECTOR* pos, long node, long rot, SVECTOR* sv)
{
	phd_PushMatrix();
	phd_mxptr[M00] = lara_joint_matrices[node * indices_count + M00];
	phd_mxptr[M01] = lara_joint_matrices[node * indices_count + M01];
	phd_mxptr[M02] = lara_joint_matrices[node * indices_count + M02];
	phd_mxptr[M03] = lara_joint_matrices[node * indices_count + M03];
	phd_mxptr[M10] = lara_joint_matrices[node * indices_count + M10];
	phd_mxptr[M11] = lara_joint_matrices[node * indices_count + M11];
	phd_mxptr[M12] = lara_joint_matrices[node * indices_count + M12];
	phd_mxptr[M13] = lara_joint_matrices[node * indices_count + M13];
	phd_mxptr[M20] = lara_joint_matrices[node * indices_count + M20];
	phd_mxptr[M21] = lara_joint_matrices[node * indices_count + M21];
	phd_mxptr[M22] = lara_joint_matrices[node * indices_count + M22];
	phd_mxptr[M23] = lara_joint_matrices[node * indices_count + M23];
	phd_TranslateRel(pos->x, pos->y, pos->z);
	phd_RotX((short)rot);
	phd_TranslateRel(sv->x, sv->y, sv->z);
	gte_sttr(pos);
	pos->x += lara_item->pos.x_pos;
	pos->y += lara_item->pos.y_pos;
	pos->z += lara_item->pos.z_pos;
	phd_PopMatrix();
}

void TriggerSubMist(PHD_VECTOR* pos, PHD_VECTOR* pos1, long size)
{
	SPARKS* sptr;
	long n;

	if (size < 0)
	{
		size = -size;
		n = 2;
	}
	else
		n = 0;

	sptr = &spark[GetFreeSpark()];
	sptr->On = 1;
	sptr->sR = 32;
	sptr->sG = 32;
	sptr->sB = 32;
	sptr->dR = 160;
	sptr->dG = 160;
	sptr->dB = 160;
	sptr->ColFadeSpeed = 2;
	sptr->FadeToBlack = 6;
	sptr->TransType = 2;
	sptr->Life = uchar((GetRandomControl() & 7) - size + 16);
	sptr->sLife = sptr->Life;
	sptr->x = (GetRandomControl() & 0x1F) + pos->x - 16;
	sptr->y = (GetRandomControl() & 0x1F) + pos->y - 16;
	sptr->z = (GetRandomControl() & 0x1F) + pos->z - 16;
	sptr->Xvel = short(pos1->x + (GetRandomControl() & 0x7F) - pos->x - 64);
	sptr->Yvel = short(pos1->y + (GetRandomControl() & 0x7F) - pos->y - 64);
	sptr->Zvel = short(pos1->z + (GetRandomControl() & 0x7F) - pos->z - 64);
	sptr->Friction = 0;
	sptr->Def = objects[458].mesh_index + (n != 0 ? 17 : 13);
	sptr->Gravity = -4 - (GetRandomControl() & 3);
	sptr->MaxYvel = 0;
	sptr->Scalar = 1;

	if (n)
	{
		sptr->Flags = 26;
		sptr->RotAng = GetRandomControl() & 0xFFF;
		sptr->RotAdd = (GetRandomControl() & 0xF) - 8;
		sptr->Size = uchar((GetRandomControl() & 0xF) + 2 * size + 8);
		sptr->sSize = sptr->Size;
		sptr->dSize = sptr->Size << 1;
	}
	else
	{
		sptr->Flags = 10;
		sptr->dSize = uchar((GetRandomControl() & 3) + size + 4);
		sptr->Size = sptr->dSize >> 1;
		sptr->sSize = sptr->Size >> 1;
	}
}

void TriggerEngineEffects()
{
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	short x, lp, n;

	if (lara.water_status == LW_FLYCHEAT)
		return;

	x = -80;

	for (lp = 0; lp < 2; lp++)
	{
		if (subsuit.Vel[lp])
		{
			pos.x = x;
			pos.y = -192;
			pos.z = -160;
			GetLaraJointPosRot(&pos, 7, subsuit.XRot, &Eng1);

			Eng2.y = subsuit.Vel[lp];
			n = Eng2.y >> 2;

			pos2.x = x;
			pos2.y = -192;
			pos2.z = -160;

			if (n)
			{
				pos2.x += GetRandomControl() % n - (n >> 1);
				pos2.z += GetRandomControl() % n - (n >> 1);
			}

			GetLaraJointPosRot(&pos2, 7, subsuit.XRot, &Eng2);
			TriggerSubMist(&pos, &pos2, Eng2.y >> 8);

			pos2.x = x;
			pos2.y = -192;
			pos2.z = -160;

			if (n)
			{
				pos2.x += GetRandomControl() % n - (n >> 1);
				pos2.z += GetRandomControl() % n - (n >> 1);
			}

			GetLaraJointPosRot(&pos2, 7, subsuit.XRot, &Eng2);
			TriggerSubMist(&pos, &pos2, -(Eng2.y >> 8));
		}

		x = 80;
	}
}

void TriggerEngineEffects_CUT()
{
	PHD_VECTOR pos;
	PHD_VECTOR pos2;
	short x, lp;

	x = -80;

	for (lp = 0; lp < 2; lp++)
	{
		if (subsuit.Vel[lp])
		{
			pos.x = x;
			pos.y = -128;
			pos.z = -144;
			GetActorJointAbsPosition(1, 7, &pos);

			Eng2.y = 512;
			pos2.x = x + (GetRandomControl() % 128 - 64);
			pos2.y = 320;
			pos2.z = -144 + (GetRandomControl() % 128 - 64);

			GetActorJointAbsPosition(1, 7, &pos2);
			TriggerSubMist(&pos, &pos2, Eng2.y >> 8);

			pos2.x = x + (GetRandomControl() % 128 - 64);
			pos2.y = 320;
			pos2.z = -144 + (GetRandomControl() % 128 - 64);

			GetActorJointAbsPosition(1, 7, &pos2);
			TriggerSubMist(&pos, &pos2, -(Eng2.y >> 8));
		}

		x = 80;
	}
}
