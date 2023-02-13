#pragma once
#include "../global/types.h"

void WriteSG(void* pointer, long size);
void ReadSG(void* pointer, long size);
long CheckSumValid(char* buffer);
void SaveLaraData();
void RestoreLaraData(long FullSave);
void SaveLevelData(long FullSave);
void RestoreLevelData(long FullSave);
void sgSaveGame();
void sgRestoreGame();

extern SAVEGAME_INFO savegame;
