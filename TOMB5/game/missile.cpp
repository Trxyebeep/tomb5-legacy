#include "../tomb5/pch.h"
#include "missile.h"
#include "../specific/function_stubs.h"
#include "control.h"
#include "items.h"
#include "effects.h"
#include "sound.h"
#include "../specific/3dmath.h"
#include "draw.h"
#include "debris.h"
#include "camera.h"
#include "lara.h"

void ControlBodyPart(short fx_number)
{
	FX_INFO* fx;
	FLOOR_INFO* floor;
	long height, ceiling, ox, oy, oz;
	short room_number, t;

	fx = &effects[fx_number];
	ox = fx->pos.x_pos;
	oz = fx->pos.z_pos;
	oy = fx->pos.y_pos;

	if (fx->counter > 0)
	{
		t = (62 - fx->counter);

		if (fx_number & 1)
		{
			fx->pos.z_rot -= (GetRandomControl() % t) << 1;
			fx->pos.x_rot += (GetRandomControl() % t) << 1;
		}
		else
		{
			fx->pos.z_rot += (GetRandomControl() % t) << 1;
			fx->pos.x_rot -= (GetRandomControl() % t) << 1;
		}

		fx->counter--;

		if (fx->counter < 8)
			fx->fallspeed += 2;
	}
	else
	{
		if (fx->speed)
			fx->pos.x_rot += fx->fallspeed << 2;

		fx->fallspeed += 6;
	}

	fx->pos.x_pos += fx->speed * phd_sin(fx->pos.y_rot) >> 14;
	fx->pos.y_pos += fx->fallspeed;
	fx->pos.z_pos += fx->speed * phd_cos(fx->pos.y_rot) >> 14;
	room_number = fx->room_number;
	floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);

	if (!fx->counter)
	{
		ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

		if (fx->pos.y_pos < ceiling)
		{
			fx->pos.y_pos = ceiling;
			fx->fallspeed = -fx->fallspeed;
			fx->speed -= fx->speed >> 3;
		}

		height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

		if (fx->pos.y_pos >= height)
		{
			if (fx->flag2 & 1)
			{
				fx->pos.x_pos = ox;
				fx->pos.y_pos = oy;
				fx->pos.z_pos = oz;

				if (fx->flag2 & 0x200)
					ExplodeFX(fx, -2, 32);
				else
					ExplodeFX(fx, -1, 32);

				KillEffect(fx_number);

				if (fx->flag2 & 0x800)
					SoundEffect(SFX_ROCK_FALL_LAND, &fx->pos, SFX_DEFAULT);

				return;
			}

			if (oy <= height)
			{
				if (fx->fallspeed <= 32)
					fx->fallspeed = 0;
				else
					fx->fallspeed = -fx->fallspeed >> 2;
			}
			else
			{
				fx->pos.y_rot += 32768;
				fx->pos.x_pos = ox;
				fx->pos.z_pos = oz;
			}

			fx->speed -= fx->speed >> 2;

			if (abs(fx->speed) < 4)
				fx->speed = 0;

			fx->pos.y_pos = oy;
		}

		if (!fx->speed)
		{
			fx->flag1++;

			if (fx->flag1 > 32)
			{
				KillEffect(fx_number);
				return;
			}
		}

		if (fx->flag2 & 2 && GetRandomControl() & 1)
			DoBloodSplat((GetRandomControl() & 0x3F) + fx->pos.x_pos - 32, (GetRandomControl() & 0x1F) + fx->pos.y_pos - 16,
				(GetRandomControl() & 0x3F) + fx->pos.z_pos - 32, 1, short(GetRandomControl() << 1), fx->room_number);
	}

	if (room_number != fx->room_number)
		EffectNewRoom(fx_number, room_number);
}

void ShootAtLara(FX_INFO* fx)
{
	short* bounds;
	long dx, dy, dz, z, x;
	
	dx = lara_item->pos.x_pos - fx->pos.x_pos;
	dy = lara_item->pos.y_pos - fx->pos.y_pos;
	dz = lara_item->pos.z_pos - fx->pos.z_pos;
	bounds = GetBoundsAccurate(lara_item);

	z = phd_sqrt(SQUARE(dx) + SQUARE(dz));
	x = bounds[3] + 3 * (bounds[2] - bounds[3]) / 4 + dy;
	fx->pos.x_rot = -(short)phd_atan(x, z);
	fx->pos.y_rot = (short)phd_atan(z, x);
	fx->pos.x_rot += short((GetRandomControl() - 0x4000) / 64);
	fx->pos.y_rot += short((GetRandomControl() - 0x4000) / 64);
}

void ControlMissile(short fx_number)
{
	FX_INFO* fx;
	FLOOR_INFO* floor;
	long speed, h, c;
	short room_number;

	fx = &effects[fx_number];
	speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> 14;
	fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> 14;
	fx->pos.y_pos += (fx->speed * phd_sin(-fx->pos.x_rot)) >> 14;
	fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> 14;

	room_number = fx->room_number;
	floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
	h = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
	c = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);

	if (fx->pos.y_pos < h && fx->pos.y_pos > c)
	{
		if (!fx->room_number != room_number)
			EffectNewRoom(fx_number, room_number);

		if (ItemNearLara(&fx->pos, 200))
		{
			lara_item->hit_status = 1;
			fx->pos.y_rot = lara_item->pos.y_rot;
			fx->speed = lara_item->speed;
			fx->counter = 0;
			fx->frame_number = 0;
		}
	}
}

long ExplodeFX(FX_INFO* fx, long NoXZVel, short Num)
{
	short** meshpp;

	meshpp = &meshes[fx->frame_number];
	ShatterItem.YRot = fx->pos.y_rot;
	ShatterItem.meshp = *meshpp;
	ShatterItem.Sphere.x = fx->pos.x_pos;
	ShatterItem.Sphere.y = fx->pos.y_pos;
	ShatterItem.Sphere.z = fx->pos.z_pos;
	ShatterItem.Bit = 0;
	ShatterItem.Flags = fx->flag2 & 0x1400;

	if (fx->flag2 & 0x2000)
		DebrisFlags = 1;

	ShatterObject(&ShatterItem, 0, Num, fx->room_number, NoXZVel);
	DebrisFlags = 0;
	return 1;
}
