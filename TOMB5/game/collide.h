#pragma once
#include "../global/types.h"

void TriggerLaraBlood();
void GetCollisionInfo(COLL_INFO* coll, long xpos, long ypos, long zpos, short room_number, long objheight);
long FindGridShift(long src, long dst);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
short GetTiltType(FLOOR_INFO* floor, long x, long y, long z);
void GenericSphereBoxCollision(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll);
void ObjectCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll);
void CreatureCollision(short item_number, ITEM_INFO* laraitem, COLL_INFO* coll);
void AIPickupCollision(short item_number, ITEM_INFO* laraitem, COLL_INFO* coll);
void TrapCollision(short item_number, ITEM_INFO* laraitem, COLL_INFO* coll);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
long GetCollidedObjects(ITEM_INFO* item, long rad, long noInvisible, ITEM_INFO** StoredItems, MESH_INFO** StoredStatics, long StoreLara);
long MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
long Move3DPosTo3DPos(PHD_3DPOS* pos, PHD_3DPOS* dest, long speed, short rotation);
long CollideStaticObjects(COLL_INFO* coll, long x, long y, long z, short room_number, long hite);
void UpdateLaraRoom(ITEM_INFO* item, long height);
void LaraBaddieCollision(ITEM_INFO* l, COLL_INFO* coll);
long ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, long spaz, long BigPush);
long ItemPushLaraStatic(ITEM_INFO* l, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll);
long TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, long rad);
long TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, long rad);
long TestLaraPosition(short* bounds, ITEM_INFO* item, ITEM_INFO* l);
void AlignLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);

extern short GlobalCollisionBounds[6];
