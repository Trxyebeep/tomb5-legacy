#pragma once
#include "../global/types.h"

long FlashIt();
void DrawGameInfo(long timed);
void DrawHealthBar(long flash_state);
void DrawAirBar(long flash_state);
void MakeAmmoString(char* string);
void InitialisePickUpDisplay();
void DrawPickups(long timed);
void AddDisplayPickup(short object_number);

extern DISPLAYPU pickups[8];
extern short PickupX;
extern short CurrentPickup;
extern long health_bar_timer;
