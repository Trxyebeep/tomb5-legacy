#pragma once
#include "../global/types.h"

void S_CDPlay(long track, long mode);
void S_CDStop();
void S_CDFade(long n);
void S_StartSyncedAudio(long track);
void ACMSetVolume();
void OpenStreamFile(char* name);
void GetADPCMData();
void ACMEmulateCDPlay(long track, long mode);
BOOL __stdcall ACMEnumCallBack(HACMDRIVERID hadid, DWORD_PTR dwInstance, DWORD fdwSupport);
long ACMSetupNotifications();
void FillADPCMBuffer(char* p, long track);
long ACMHandleNotifications();
bool ACMInit();
void ACMClose();

extern HACMDRIVER hACMDriver;
extern uchar* wav_file_buffer;
extern uchar* ADPCMBuffer;
extern bool acm_ready;
extern long XATrack;
extern long XAFlag;
