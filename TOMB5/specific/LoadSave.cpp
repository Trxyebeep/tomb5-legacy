#include "../tomb5/pch.h"
#include "LoadSave.h"
#include "../game/text.h"
#include "../game/gameflow.h"
#include "../game/sound.h"
#include "audio.h"
#include "dxsound.h"
#include "input.h"
#include "function_table.h"
#include "drawroom.h"
#include "polyinsert.h"
#include "winmain.h"
#include "output.h"
#include "dxshell.h"
#include "texture.h"
#include "function_stubs.h"
#include "../game/savegame.h"
#include "gamemain.h"
#include "specificfx.h"
#include "time.h"
#include "file.h"
#include "fmv.h"
#include "../game/newinv2.h"
#include "../game/control.h"
#include "3dmath.h"
#include "../game/lara.h"

long sfx_frequencies[3] = { 11025, 22050, 44100 };
long SoundQuality = 1;
long MusicVolume = 40;
long SFXVolume = 80;
long ControlMethod;

MONOSCREEN_STRUCT MonoScreen[5];
char MonoScreenOn;

long loadbar_on;
static float loadbar_steps;
static float loadbar_pos;
static long loadbar_maxpos;
static long SpecialFeaturesNum = -1;
static long NumSpecialFeatures;

static LPDIRECTDRAWSURFACE4 screen_surface;
static SAVEFILE_INFO SaveGames[15];
static long MonoScreenX[4] = { 0, 256, 512, 640 };
static long MonoScreenY[3] = { 0, 256, 480 };
static char SpecialFeaturesPage[5];

static const char* screen_paths[5] =
{
	"SCREENS\\STORY1.STR",
	"SCREENS\\NXG.STR",
	"SCREENS\\STORY2.STR",
	"SCREENS\\GALLERY.STR",
	"SCREENS\\SCREENS.STR"
};

void CheckKeyConflicts()
{
	short key;

	for (int i = 0; i < 18; i++)
	{
		key = layout[0][i];

		conflict[i] = 0;

		for (int j = 0; j < 18; j++)
		{
			if (key == layout[1][j])
			{
				conflict[i] = 1;
				break;
			}
		}
	}
}

#pragma warning(push)
#pragma warning(disable : 4244)
void DoStatScreen()
{
	ushort ypos;
	short Days, Hours, Min, Sec;
	char buffer[40];
	long seconds;

	ypos = phd_centery - 4 * font_height;
	PrintString(phd_centerx, ypos, 6, SCRIPT_TEXT(TXT_Statistics), FF_CENTER);
	PrintString(phd_centerx, ypos + 2 * font_height, 2, SCRIPT_TEXT(gfLevelNames[gfCurrentLevel]), FF_CENTER);
	PrintString(phd_centerx >> 2, ypos + 3 * font_height, 2, SCRIPT_TEXT(TXT_Time_Taken), 0);
	PrintString(phd_centerx >> 2, ypos + 4 * font_height, 2, SCRIPT_TEXT(TXT_Distance_Travelled), 0);
	PrintString(phd_centerx >> 2, ypos + 5 * font_height, 2, SCRIPT_TEXT(TXT_Ammo_Used), 0);
	PrintString(phd_centerx >> 2, ypos + 6 * font_height, 2, SCRIPT_TEXT(TXT_Health_Packs_Used), 0);
	PrintString(phd_centerx >> 2, ypos + 7 * font_height, 2, SCRIPT_TEXT(TXT_Secrets_Found), 0);

	seconds = GameTimer / 30;
	Days = seconds / (24 * 60 * 60);
	Hours = (seconds % (24 * 60 * 60)) / (60 * 60);
	Min = (seconds / 60) % 60;
	Sec = (seconds % 60);

	sprintf(buffer, "%02d:%02d:%02d", (Days * 24) + Hours, Min, Sec);
	PrintString(phd_centerx + (phd_centerx >> 2), ypos + 3 * font_height, 6, buffer, 0);
	sprintf(buffer, "%dm", savegame.Game.Distance / 419);
	PrintString(phd_centerx + (phd_centerx >> 2), ypos + 4 * font_height, 6, buffer, 0);
	sprintf(buffer, "%d", savegame.Game.AmmoUsed);
	PrintString(phd_centerx + (phd_centerx >> 2), ypos + 5 * font_height, 6, buffer, 0);
	sprintf(buffer, "%d", savegame.Game.HealthUsed);
	PrintString(phd_centerx + (phd_centerx >> 2), ypos + 6 * font_height, 6, buffer, 0);
	sprintf(buffer, "%d / 36", savegame.Game.Secrets);
	PrintString(phd_centerx + (phd_centerx >> 2), ypos + 7 * font_height, 6, buffer, 0);
}
#pragma warning(pop)

void DisplayStatsUCunt()
{
	DoStatScreen();
}

void S_DrawAirBar(long pos)
{
	if (gfCurrentLevel != LVL5_TITLE)
		DoBar(490 - (font_height >> 2), (font_height >> 1) + (font_height >> 2) + 32, 150, 12, pos, 0x0000A0, 0x0050A0);
}

void S_DrawHealthBar(long pos)
{
	long color;

	if (gfCurrentLevel != LVL5_TITLE)
	{
		if (lara.poisoned || lara.Gassed)
			color = 0xA0A000;//yellowish poison, rgb 160, 160, 0
		else
			color = 0x00A000;//green, rgb 0, 160, 0

		DoBar(font_height >> 2, (font_height >> 2) + 32, 150, 12, pos, 0xA00000, color);//red rgb 160, 0, 0/color
	}
}

void S_DrawHealthBar2(long pos)//same as above just different screen position
{
	long color;

	if (gfCurrentLevel != LVL5_TITLE)
	{
		if (lara.poisoned || lara.Gassed)
			color = 0xA0A000;
		else
			color = 0xA000;

		DoBar(245, (font_height >> 1) + 32, 150, 12, pos, 0xA00000, color);
	}
}

void S_DrawDashBar(long pos)
{
	if (gfCurrentLevel != LVL5_TITLE)
		DoBar(490 - (font_height >> 2), (font_height >> 2) + 32, 150, 12, pos, 0xA0A000, 0x00A000);
}

#pragma warning(push)
#pragma warning(disable : 4244)
long DoLoadSave(long LoadSave)
{
	SAVEFILE_INFO* pSave;
	static long selection;
	long txt, color, l;
	char string[80];
	char name[41];

	if (LoadSave & IN_SAVE)
		txt = TXT_Save_Game;
	else
		txt = TXT_Load_Game;

	PrintString(phd_centerx, font_height, 6, SCRIPT_TEXT(txt), FF_CENTER);

	for (int i = 0; i < 15; i++)
	{
		pSave = &SaveGames[i];
		color = 2;

		if (i == selection)
			color = 1;

		memset(name, ' ', 40);
		l = strlen(pSave->name);

		if (l > 40)
			l = 40;

		strncpy(name, pSave->name, l);
		name[40] = 0;
		small_font = 1;

		if (pSave->valid)
		{
			wsprintf(string, "%03d", pSave->num);
			PrintString(phd_centerx - long((float)phd_winwidth / 640.0F * 310.0F), font_height + font_height * (i + 2), color, string, 0);
			PrintString(phd_centerx - long((float)phd_winwidth / 640.0F * 270.0F), font_height + font_height * (i + 2), color, name, 0);
			wsprintf(string, "%d %s %02d:%02d:%02d", pSave->days, SCRIPT_TEXT(TXT_days), pSave->hours, pSave->minutes, pSave->seconds);
			PrintString(phd_centerx - long((float)phd_winwidth / 640.0F * -135.0F), font_height + font_height * (i + 2), color, string, 0);
		}
		else
		{
			wsprintf(string, "%s", pSave->name);
			PrintString(phd_centerx, font_height + font_height * (i + 2), color, string, FF_CENTER);
		}

		small_font = 0;
	}

	if (dbinput & IN_FORWARD)
	{
		selection--;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_DEFAULT);
	}

	if (dbinput & IN_BACK)
	{
		selection++;
		SoundEffect(SFX_MENU_CHOOSE, 0, SFX_DEFAULT);
	}

	if (selection < 0)
		selection = 0;

	if (selection > 14)
		selection = 14;

	if (dbinput & IN_SELECT)
	{
		if (SaveGames[selection].valid || LoadSave == IN_SAVE)
			return selection;
		else
			SoundEffect(SFX_LARA_NO, 0, SFX_DEFAULT);
	}

	return -1;
}
#pragma warning (pop)

void S_MemSet(void* p, long val, size_t sz)
{
	memset(p, val, sz);
}

long GetCampaignCheatValue()
{
	static long counter = 0;
	static long timer;
	long jump;

	if (timer)
		timer--;
	else
		counter = 0;

	jump = 0;

	switch (counter)
	{
	case 0:

		if (keymap[DIK_F])
		{
			timer = 450;
			counter = 1;
		}

		break;

	case 1:
		if (keymap[DIK_I])
			counter = 2;

		break;

	case 2:
		if (keymap[DIK_L])
			counter = 3;

		break;

	case 3:
		if (keymap[DIK_T])
			counter = 4;

		break;

	case 4:
		if (keymap[DIK_H])
			counter = 5;

		break;

	case 5:
		if (keymap[DIK_Y])
			counter = 6;

		break;

	case 6:
		if (keymap[DIK_1])
			jump = LVL5_STREETS_OF_ROME;

		if (keymap[DIK_2])
			jump = LVL5_BASE;

		if (keymap[DIK_3])
			jump = LVL5_GALLOWS_TREE;

		if (keymap[DIK_4])
			jump = LVL5_THIRTEENTH_FLOOR;

		if (jump)
		{
			counter = 0;
			timer = 0;
		}

		break;
	}

	return jump;
}

#pragma warning(push)
#pragma warning(disable : 4244)
void DoOptions()
{
	const char** keyboard_buttons;
	static long menu;	//0: options, 1: controls, 100: special features
	static ulong selection = 1;	//selection
	static ulong selection_bak;
	static ulong controls_selection;	//selection for when mapping keys
	static long music_volume_bar_shade = 0xFF3F3F3F;
	static long sfx_volume_bar_shade = 0xFF3F3F3F;
	static long sfx_bak;	//backup sfx volume
	static long sfx_quality_bak;	//backup sfx quality
	static long sfx_breath_db = -1;
	long textY, textY2, special_features_available, joystick, joystick_x, joystick_y, joy1, joy2, joy3;
	const char* text;
	uchar clr, num, num2;
	char quality_buffer[256];
	char quality_text[80];
	static char sfx_backup_flag;	//have we backed sfx stuff up?
	static bool waiting_for_key = 0;

	if (!(sfx_backup_flag & 1))
	{
		sfx_backup_flag |= 1;
		sfx_bak = SFXVolume;
	}

	if (!(sfx_backup_flag & 2))
	{
		sfx_backup_flag |= 2;
		sfx_quality_bak = SoundQuality;
	}

	textY = font_height - 4;

	if (menu == 1)	//controls menu
	{
		if (Gameflow->Language == 2)
			keyboard_buttons = GermanKeyboard;
		else
			keyboard_buttons = KeyboardButtons;

		if (ControlMethod)
			num = 11;
		else
			num = 17;

		PrintString(phd_centerx >> 2, font_height, selection & 1 ? 1 : 2, SCRIPT_TEXT(TXT_Control_Method), 0);
		textY = font_height;
		font_height = (long)((float)phd_winymax * 0.050000001F);
		big_char_height = 10;
		textY2 = font_height + (font_height + (font_height >> 1));

		if (!ControlMethod)
		{
			PrintString(phd_centerx >> 2, ushort(textY2 + 1 * font_height), selection & 2 ? 1 : 2, "\x18", 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 2 * font_height), selection & 4 ? 1 : 2, "\x1A", 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 3 * font_height), selection & 8 ? 1 : 2, "\x19", 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 4 * font_height), selection & 0x10 ? 1 : 2, "\x1B", 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 5 * font_height),  selection & 0x20 ? 1 : 2, SCRIPT_TEXT(TXT_Duck), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 6 * font_height), selection & 0x40 ? 1 : 2, SCRIPT_TEXT(TXT_Dash), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 7 * font_height), selection & 0x80 ? 1 : 2, SCRIPT_TEXT(TXT_Walk), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 8 * font_height), selection & 0x100 ? 1 : 2, SCRIPT_TEXT(TXT_Jump), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 9 * font_height), selection & 0x200 ? 1 : 2, SCRIPT_TEXT(TXT_Action), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 10 * font_height), selection & 0x400 ? 1 : 2, SCRIPT_TEXT(TXT_Draw_Weapon), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 11 * font_height), selection & 0x800 ? 1 : 2, SCRIPT_TEXT(TXT_Use_Flare), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 12 * font_height), selection & 0x1000 ? 1 : 2, SCRIPT_TEXT(TXT_Look), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 13 * font_height), selection & 0x2000 ? 1 : 2, SCRIPT_TEXT(TXT_Roll), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 14 * font_height), selection & 0x4000 ? 1 : 2, SCRIPT_TEXT(TXT_Inventory), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 15 * font_height), selection & 0x8000 ? 1 : 2, SCRIPT_TEXT(TXT_Step_Left), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 16 * font_height), selection & 0x10000 ? 1 : 2, SCRIPT_TEXT(TXT_Step_Right), 0);
			text = (waiting_for_key && (controls_selection & 2)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][0]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + font_height), controls_selection & 2 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 4)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][1]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 2 * font_height), controls_selection & 4 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 8)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][2]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 3 * font_height), (controls_selection & 8) != 0 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x10)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][3]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 4 * font_height), controls_selection & 0x10 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x20)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][4]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 5 * font_height), controls_selection & 0x20 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x40)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][5]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 6 * font_height), controls_selection & 0x40 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x80)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][6]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 7 * font_height), controls_selection & 0x80 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x100)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][7]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 8 * font_height), controls_selection & 0x100 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x200)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][8]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 9 * font_height), controls_selection & 0x200 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x400)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][9]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 10 * font_height), controls_selection & 0x400 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x800)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][10]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 11 * font_height), controls_selection & 0x800 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x1000)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][11]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 12 * font_height), controls_selection & 0x1000 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x2000)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][12]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 13 * font_height), controls_selection & 0x2000 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x4000)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][13]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 14 * font_height), controls_selection & 0x4000 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x8000)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][14]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 15 * font_height), controls_selection & 0x8000 ? 1 : 6, text, 0);
			text = (waiting_for_key && (controls_selection & 0x10000)) ? SCRIPT_TEXT(TXT_Waiting) : keyboard_buttons[layout[1][15]];
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 16 * font_height), controls_selection & 0x10000 ? 1 : 6, text, 0);
		}

		if (ControlMethod == 1)
		{
			PrintString(phd_centerx >> 2, ushort(textY2 + 5 * font_height), selection & 2 ? 1 : 2, SCRIPT_TEXT(TXT_Duck), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 6 * font_height), selection & 4 ? 1 : 2, SCRIPT_TEXT(TXT_Dash), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 7 * font_height), selection & 8 ? 1 : 2, SCRIPT_TEXT(TXT_Walk), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 8 * font_height), selection & 0x10 ? 1 : 2, SCRIPT_TEXT(TXT_Jump), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 9 * font_height), selection & 0x20 ? 1 : 2, SCRIPT_TEXT(TXT_Action), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 10 * font_height), selection & 0x40 ? 1 : 2, SCRIPT_TEXT(TXT_Draw_Weapon), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 11 * font_height), selection & 0x80 ? 1 : 2, SCRIPT_TEXT(TXT_Use_Flare), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 12 * font_height), selection & 0x100 ? 1 : 2, SCRIPT_TEXT(TXT_Look), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 13 * font_height), selection & 0x200 ? 1 : 2, SCRIPT_TEXT(TXT_Roll), 0);
			PrintString(phd_centerx >> 2, ushort(textY2 + 14 * font_height), selection & 0x400 ? 1 : 2, SCRIPT_TEXT(TXT_Inventory), 0);

			for (int i = 0; i < 10; i++)
			{
				sprintf(quality_buffer, "(%s)", keyboard_buttons[layout[1][i + 4]]);
				PrintString((phd_centerx >> 3) + phd_centerx + (phd_centerx >> 1), ushort(textY2 + font_height * (i + 5)), 5, quality_buffer, 0);
			}

			text = (waiting_for_key && (controls_selection & 2)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[0]];
			clr = (waiting_for_key && (selection & 2)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 5 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 4)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[1]];
			clr = (waiting_for_key && (selection & 4)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 6 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 8)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[2]];
			clr = (waiting_for_key && (selection & 8)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 7 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x10)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[3]];
			clr = (waiting_for_key && (selection & 0x10)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 8 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x20)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[4]];
			clr = (waiting_for_key && (selection & 0x20)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 9 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x40)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[5]];
			clr = (waiting_for_key && (selection & 0x40)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 10 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x80)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[6]];
			clr = (waiting_for_key && (selection & 0x80)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 11 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x100)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[7]];
			clr = (waiting_for_key && (selection & 0x100)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 12 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x200)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[8]];
			clr = (waiting_for_key && (selection & 0x200)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 13 * font_height), clr, text, 0);
			text = (waiting_for_key && (controls_selection & 0x400)) ? SCRIPT_TEXT(TXT_Waiting) : JoyStickButtons[jLayout[9]];
			clr = (waiting_for_key && (selection & 0x400)) ? 1 : 6;
			PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY2 + 14 * font_height), clr, text, 0);
		}

		font_height = default_font_height;
		big_char_height = 6;

		if (!ControlMethod)
			PrintString(phd_centerx + (phd_centerx >> 2), (ushort)textY, controls_selection & 1 ? 1 : 6, SCRIPT_TEXT(TXT_Keyboard), 0);
		else if (ControlMethod == 1)
			PrintString(phd_centerx + (phd_centerx >> 2), (ushort)textY, controls_selection & 1 ? 1 : 6, SCRIPT_TEXT(TXT_Joystick), 0);
		else if (ControlMethod == 2)
			PrintString(phd_centerx + (phd_centerx >> 2), (ushort)textY, controls_selection & 1 ? 1 : 6, SCRIPT_TEXT(TXT_Reset), 0);

		if (ControlMethod < 2 && !waiting_for_key)
		{
			if (dbinput & IN_FORWARD)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				selection >>= 1;
			}

			if (dbinput & IN_BACK)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				selection <<= 1;
			}
		}

		if (waiting_for_key)
		{
			num2 = 0;

			if (keymap[DIK_ESCAPE])
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				controls_selection = 0;
				dbinput = 0;
				waiting_for_key = 0;
				return;
			}

			if (!ControlMethod)
			{
				for (int i = 0; i < 255; i++)
				{
					if (keymap[i] && keyboard_buttons[i])
					{
						if (i != DIK_RETURN && i != DIK_LEFT && i != DIK_RIGHT && i != DIK_UP && i != DIK_DOWN)
						{
							waiting_for_key = 0;

							for (int j = controls_selection >> 2; j; num2++)
								j >>= 1;

							controls_selection = 0;
							layout[1][num2] = i;
						}
					}
				}
			}

			if (ControlMethod == 1)
			{
				joystick = ReadJoystick(joystick_x, joystick_y);

				if (joystick)
				{
					joy1 = selection >> 2;
					joy2 = 0;
					joy3 = 0;

					while (joy1)
					{
						joy1 >>= 1;
						joy2++;
					}

					joy1 = joystick >> 1;

					while (joy1)
					{
						joy1 >>= 1;
						joy3++;
					}

					jLayout[joy2] = joy3;
					waiting_for_key = 0;
				}
			}

			CheckKeyConflicts();
			dbinput = 0;
		}

		if (dbinput & IN_SELECT && selection > 1 && ControlMethod < 2)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			controls_selection = selection;
			waiting_for_key = 1;
			memset(keymap, 0, sizeof(keymap));
		}

		if (dbinput & IN_SELECT && ControlMethod == 2)
		{
			SoundEffect(SFX_MENU_SELECT, 0, 2);
			memcpy(layout[1], layout, 72);
			ControlMethod = 0;
			memcpy(jLayout, defaultJLayout, 32);
		}

		if (selection & 1)
		{
			if (dbinput & IN_LEFT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod--;
			}

			if (dbinput & IN_RIGHT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				ControlMethod++;
			}

			if (ControlMethod > 2)
				ControlMethod = 2;

			if (ControlMethod < 0)
				ControlMethod = 0;

			if (ControlMethod == 1 && !joystick_read)
			{
				if (dbinput & IN_LEFT)
					ControlMethod = 0;

				if (dbinput & IN_RIGHT)
					ControlMethod = 2;
			}
		}

		if (!selection)
			selection = 1;

		if (selection > (ulong)(1 << (num - 1)))
			selection = 1 << (num - 1);

		if (dbinput & IN_DESELECT)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);

			if (ControlMethod < 2)
				menu = 0;

			dbinput = 0;
			selection = 1;
		}
	}
	else if (menu == 100)	//special features
	{
		PrintString(phd_centerx, ushort(textY + 3 * font_height), 6, SCRIPT_TEXT(TXT_Special_Features), FF_CENTER);

		if (SpecialFeaturesPage[0])
			clr = selection & 1 ? 1 : 2;
		else
			clr = 3;

		PrintString(phd_centerx, ushort(textY + 5 * font_height), clr, SCRIPT_TEXT(TXT_Storyboards_Part_1), FF_CENTER);

		if (SpecialFeaturesPage[1])
			clr = selection & 2 ? 1 : 2;
		else
			clr = 3;

		PrintString(phd_centerx, ushort(textY + 6 * font_height), clr, SCRIPT_TEXT(TXT_Next_Generation_Concept), FF_CENTER);

		if (SpecialFeaturesPage[2])
			clr = selection & 4 ? 1 : 2;
		else
			clr = 3;

		PrintString(phd_centerx, ushort(textY + 7 * font_height), clr, SCRIPT_TEXT(TXT_Storyboards_Part_2), FF_CENTER);

		if (SpecialFeaturesPage[3])
			clr = selection & 8 ? 1 : 2;
		else
			clr = 3;

		PrintString(phd_centerx, ushort(textY + 8 * font_height), clr, "Gallery", FF_CENTER);

		if (NumSpecialFeatures)
		{
			if (dbinput & IN_FORWARD)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
				selection = FindSFCursor(1, selection);
			}

			if (dbinput & IN_BACK)
			{
				SoundEffect(SFX_MENU_CHOOSE, 0, 2);
				selection = FindSFCursor(2, selection);
			}

			if (!selection)
				selection = 1;
			else if (selection > 8)
				selection = 8;

			if (dbinput & IN_SELECT)
			{
				if (selection & 1)
					SpecialFeaturesNum = 0;

				if (selection & 2)
					SpecialFeaturesNum = 1;

				if (selection & 4)
					SpecialFeaturesNum = 2;

				if (selection & 8)
					SpecialFeaturesNum = 3;

				if (selection & 16)
					SpecialFeaturesNum = 4;
			}
		}

		if (dbinput & IN_DESELECT)
		{
			menu = 0;	//go back to main options menu
			selection = selection_bak;	//go back to selection
			dbinput &= ~IN_DESELECT;	//don't deselect twice
		}
	}
	else if (menu == 0)	//main options menu
	{
		textY= 3 * font_height;
		num = 5;
		PrintString(phd_centerx, 3 * font_height, 6, SCRIPT_TEXT(TXT_Options), FF_CENTER);
		PrintString(phd_centerx, ushort(textY + font_height + (font_height >> 1)), selection & 1 ? 1 : 2, SCRIPT_TEXT(TXT_Control_Configuration), FF_CENTER);
		PrintString(phd_centerx >> 2, ushort(textY + 3 * font_height), selection & 2 ? 1 : 2, SCRIPT_TEXT(TXT_Music_Volume), 0);
		PrintString(phd_centerx >> 2, ushort(textY + 4 * font_height), selection & 4 ? 1 : 2, SCRIPT_TEXT(TXT_SFX_Volume), 0);
		PrintString(phd_centerx >> 2, ushort(textY + 5 * font_height), selection & 8 ? 1 : 2, SCRIPT_TEXT(TXT_Sound_Quality), 0);
		PrintString(phd_centerx >> 2, ushort(textY + 6 * font_height), selection & 0x10 ? 1 : 2, SCRIPT_TEXT(TXT_Targeting), 0);
		DoSlider(400, 3 * font_height - (font_height >> 1) + textY + 4, 200, 16, MusicVolume, 0xFF1F1F1F, 0xFF3F3FFF, music_volume_bar_shade);
		DoSlider(400, textY + 4 * font_height + 4 - (font_height >> 1), 200, 16, SFXVolume, 0xFF1F1F1F, 0xFF3F3FFF, sfx_volume_bar_shade);

		switch (SoundQuality)
		{
		case 0:
			strcpy(quality_text, SCRIPT_TEXT(TXT_Low));
			break;

		case 1:
			strcpy(quality_text, SCRIPT_TEXT(TXT_Medium));
			break;

		case 2:
			strcpy(quality_text, SCRIPT_TEXT(TXT_High));
			break;
		}

		PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY + 5 * font_height), selection & 8 ? 1 : 6, quality_text, 0);

		if (App.AutoTarget)
			strcpy(quality_text, SCRIPT_TEXT(TXT_Automatic));
		else
			strcpy(quality_text, SCRIPT_TEXT(TXT_Manual));

		PrintString(phd_centerx + (phd_centerx >> 2), ushort(textY + 6 * font_height), selection & 0x10 ? 1 : 6, quality_text, 0);
		special_features_available = 0x20;	//not the most accurate name

		if (gfGameMode == 1)
		{
			num = 6;
			PrintString(phd_centerx, ushort((font_height >> 1) + textY + 7 * font_height), selection & 0x20 ? 1 : 2, SCRIPT_TEXT(TXT_Special_Features), FF_CENTER);
		}
		else
			special_features_available = 0;

		if (dbinput & IN_FORWARD)
		{
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			selection >>= 1;
		}

		if (dbinput & IN_BACK)
		{
			SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			selection <<= 1;
		}

		if (dbinput & IN_SELECT && selection & 1)
		{
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			menu = 1;
		}

		if (!selection)
			selection = 1;

		if (selection > (ulong)(1 << (num - 1)))
			selection = 1 << (num - 1);

		music_volume_bar_shade = 0xFF3F3F3F;
		sfx_volume_bar_shade = 0xFF3F3F3F;

		if (selection & 2)
		{
			sfx_bak = SFXVolume;

			if (input & IN_LEFT || keymap[DIK_LEFT])
				MusicVolume--;

			if (input & IN_RIGHT || keymap[DIK_RIGHT])
				MusicVolume++;

			if (MusicVolume > 100)
				MusicVolume = 100;

			if (MusicVolume < 0)
				MusicVolume = 0;

			sfx_volume_bar_shade = 0xFF3F3F3F;
			music_volume_bar_shade = 0xFF7F7F7F;
			ACMSetVolume();
		}
		else if (selection & 4)
		{
			if (input & IN_LEFT || keymap[DIK_LEFT])
				SFXVolume--;

			if (input & IN_RIGHT || keymap[DIK_RIGHT])
				SFXVolume++;

			if (SFXVolume > 100)
				SFXVolume = 100;

			if (SFXVolume < 0)
				SFXVolume = 0;

			if (SFXVolume != sfx_bak)
			{
				if (sfx_breath_db == -1 || !DSIsChannelPlaying(0))
				{
					S_SoundStopAllSamples();
					sfx_bak = SFXVolume;
					sfx_breath_db = SoundEffect(SFX_LARA_BREATH, 0, SFX_DEFAULT);
					DSChangeVolume(0, -100 * (long(100 - SFXVolume) >> 1));
				}
				else if (sfx_breath_db != -1 && DSIsChannelPlaying(0))
					DSChangeVolume(0, -100 * ((100 - SFXVolume) >> 1));
			}

			music_volume_bar_shade = 0xFF3F3F3F;
			sfx_volume_bar_shade = 0xFF7F7F7F;
		}
		else if (selection & 8)
		{
			sfx_bak = SFXVolume;
			
			if (dbinput & IN_LEFT)
				SoundQuality--;

			if (dbinput & IN_RIGHT)
				SoundQuality++;

			if (SoundQuality > 2)
				SoundQuality = 2;

			if (SoundQuality < 0)
				SoundQuality = 0;

			if (SoundQuality != sfx_quality_bak)
			{
				S_SoundStopAllSamples();
				DXChangeOutputFormat(sfx_frequencies[SoundQuality], 0);
				sfx_quality_bak = SoundQuality;
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}
		}
		else if (selection & 16)
		{
			if (dbinput & IN_LEFT)
			{
				if (App.AutoTarget)
					App.AutoTarget = 0;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_RIGHT)
			{
				if (!App.AutoTarget)
					App.AutoTarget = 1;

				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
			}

			savegame.AutoTarget = (uchar)App.AutoTarget;
		}
		else if (selection & special_features_available && dbinput & IN_SELECT)
		{
			CalculateNumSpecialFeatures();
			selection_bak = selection;
			selection = 1;
			menu = 100;
		}
	}
}
#pragma warning (pop)

void DoBar(long x, long y, long width, long height, long pos, long clr1, long clr2)
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float fx, fx2, fy, fw, fh, r1, g1, b1, r2, g2, b2, r, g, b, mul;
	long lr, lg, lb, clr_11, clr_12, clr_21, clr_22;

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	nPolyType = 4;
	tex.drawtype = 0;
	tex.tpage = 0;
	fx = (float)phd_winxmax * 0.0015625F;
	fy = (float)phd_winymax * 0.0020833334F;
	fw = (float)width;
	fh = (float)(height >> 1);
	fx2 = (fw * fx) * 0.0099999998F * (float)pos;
	v[0].specular = 0xFF000000;
	v[1].specular = 0xFF000000;
	v[2].specular = 0xFF000000;
	v[3].specular = 0xFF000000;
	v[0].sx = (float)x * fx;
	v[1].sx = ((float)x * fx) + fx2;
	v[2].sx = (float)x * fx;
	v[3].sx = ((float)x * fx) + fx2;
	v[0].sy = (float)y * fy;
	v[1].sy = (float)y * fy;
	v[2].sy = ((float)y * fy) + (fh * fy);
	v[3].sy = ((float)y * fy) + (fh * fy);
	v[0].sz = f_mznear;
	v[1].sz = f_mznear;
	v[2].sz = f_mznear;
	v[3].sz = f_mznear;
	v[0].rhw = f_mpersp / f_mznear * f_moneopersp;
	v[1].rhw = f_mpersp / f_mznear * f_moneopersp;
	v[2].rhw = f_mpersp / f_mznear * f_moneopersp;
	v[3].rhw = f_mpersp / f_mznear * f_moneopersp;

	r1 = (float)CLRR(clr1);		//get rgbs
	g1 = (float)CLRG(clr1);
	b1 = (float)CLRB(clr1);
	r2 = (float)CLRR(clr2);
	g2 = (float)CLRG(clr2);
	b2 = (float)CLRB(clr2);

	mul = fx2 / (fw * fx);		//mix
	r = r1 + ((r2 - r1) * mul);
	g = g1 + ((g2 - g1) * mul);
	b = b1 + ((b2 - b1) * mul);

	lr = (long)r1;
	lg = (long)g1;
	lb = (long)b1;
	clr_11 = RGBONLY(lr >> 1, lg >> 1, lb >> 1);
	clr_12 = RGBONLY(lr, lg, lb);

	lr = (long)r;
	lg = (long)g;
	lb = (long)b;
	clr_21 = RGBONLY(lr >> 1, lg >> 1, lb >> 1);
	clr_22 = RGBONLY(lr, lg, lb);

	v[0].color = clr_11;
	v[1].color = clr_21;
	v[2].color = clr_12;
	v[3].color = clr_22;
	AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);	//top half

	v[0].color = clr_12;
	v[1].color = clr_22;
	v[2].color = clr_11;
	v[3].color = clr_21;
	v[0].sy = ((float)y * fy) + (fh * fy);
	v[1].sy = ((float)y * fy) + (fh * fy);
	v[2].sy = (fh * fy) + (fh * fy) + ((float)y * fy);
	v[3].sy = (fh * fy) + (fh * fy) + ((float)y * fy);
	AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);		//bottom half

	v[0].sx = (float)x * fx;
	v[1].sx = (fw * fx) + ((float)x * fx);
	v[2].sx = (float)x * fx;
	v[3].sx = (fw * fx) + ((float)x * fx);
	v[0].sy = (float)y * fy;
	v[1].sy = (float)y * fy;
	v[2].sy = (fh * fy) + (fh * fy) + ((float)y * fy);
	v[3].sy = (fh * fy) + (fh * fy) + ((float)y * fy);
	v[0].sz = f_mznear + 1;
	v[1].sz = f_mznear + 1;
	v[2].sz = f_mznear + 1;
	v[3].sz = f_mznear + 1;
	v[0].rhw = f_mpersp / (f_mznear + 1) * f_moneopersp;
	v[1].rhw = f_mpersp / (f_mznear + 1) * f_moneopersp;
	v[2].rhw = f_mpersp / (f_mznear + 1) * f_moneopersp;
	v[3].rhw = f_mpersp / (f_mznear + 1) * f_moneopersp;
	v[0].color = 0;
	v[1].color = 0;
	v[2].color = 0;
	v[3].color = 0;
	AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);	//black background

	v[0].sx = ((float)x * fx) - 1;
	v[1].sx = (fw * fx) + ((float)x * fx) + 1;
	v[2].sx = ((float)x * fx) - 1;
	v[3].sx = (fw * fx) + ((float)x * fx) + 1;
	v[0].sy = ((float)y * fy) - 1;
	v[1].sy = ((float)y * fy) - 1;
	v[2].sy = (fh * fy) + (fh * fy) + ((float)y * fy) + 1;
	v[3].sy = (fh * fy) + (fh * fy) + ((float)y * fy) + 1;
	v[0].sz = f_mznear + 2;
	v[1].sz = f_mznear + 2;
	v[2].sz = f_mznear + 2;
	v[3].sz = f_mznear + 2;
	v[0].rhw = f_mpersp / (f_mznear + 2) * f_moneopersp;
	v[1].rhw = f_mpersp / (f_mznear + 2) * f_moneopersp;
	v[2].rhw = f_mpersp / (f_mznear + 2) * f_moneopersp;
	v[3].rhw = f_mpersp / (f_mznear + 2) * f_moneopersp;
	v[0].color = 0xFFFFFFFF;
	v[1].color = 0xFFFFFFFF;
	v[2].color = 0xFFFFFFFF;
	v[3].color = 0xFFFFFFFF;
	AddQuadSorted(v, 0, 1, 3, 2, &tex, 1);	//white border
}

void CreateMonoScreen()
{
	MonoScreenOn = 1;
	ConvertSurfaceToTextures(App.dx.lpPrimaryBuffer);
}

void S_InitLoadBar(long max)
{
	loadbar_steps = 0;
	loadbar_maxpos = max;
	loadbar_pos = 0;
	loadbar_on = 1;
}

void S_UpdateLoadBar()
{
	loadbar_steps = 100.0F / loadbar_maxpos + loadbar_steps;
}

long S_DrawLoadBar()
{
	_BeginScene();
	InitBuckets();
	InitialiseSortList();
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0);
	DoBar(170, 480 - font_height, 300, 10, (long)loadbar_pos, 0xA0, 0xF0);
	SortPolyList(SortCount, SortList);
	RestoreFPCW(FPCW);
	DrawSortList();
	MungeFPCW(&FPCW);
	S_DumpScreenFrame();

	if (loadbar_pos >= loadbar_steps)
		return loadbar_maxpos <= loadbar_steps;

	loadbar_pos += 2;
	return 0;
}

void S_LoadBar()
{
	S_UpdateLoadBar();
}

void MemBltSurf(void* dest, long x, long y, long w, long h, long dadd, void* source, long x2, long y2, DDSURFACEDESC2 surface, float xsize, float ysize)
{
	ulong* pDest;
	short* psSrc;
	char* pSrc;
	long xadd, yadd, rx2, ry2, xoff, yoff, curY;
	short andVal;
	uchar r, g, b, rshift, gshift, bshift, rcount, gcount, bcount;

	xadd = long(((float)App.dx.dwRenderWidth / 640.0F) * xsize * 65536.0);
	yadd = long(((float)App.dx.dwRenderHeight / 480.0F) * ysize * 65536.0);
	rx2 = long(x2 * ((float)App.dx.dwRenderWidth / 639.0F));
	ry2 = long(y2 * ((float)App.dx.dwRenderHeight / 479.0F));

	if (App.dx.Flags & 2)
	{
		rx2 += App.dx.rScreen.left;
		ry2 += App.dx.rScreen.top;
	}

	DXBitMask2ShiftCnt(surface.ddpfPixelFormat.dwRBitMask, &rshift, &rcount);
	DXBitMask2ShiftCnt(surface.ddpfPixelFormat.dwGBitMask, &gshift, &gcount);
	DXBitMask2ShiftCnt(surface.ddpfPixelFormat.dwBBitMask, &bshift, &bcount);
	pDest = (ulong*)dest + 4 * h * y + x;
	pSrc = (char*)source + rx2 * (surface.ddpfPixelFormat.dwRGBBitCount >> 3) + (ry2 * surface.lPitch);
	psSrc = (short*)pSrc;
	curY = 0;
	yoff = 0;

	if (surface.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		for (int i = 0; i < h; i++)
		{
			xoff = 0;

			for (int j = 0; j < w; j++)
			{
				andVal = psSrc[curY + (xoff >> 16)];
				r = uchar(((surface.ddpfPixelFormat.dwRBitMask & andVal) >> rshift) << (8 - rcount));
				g = uchar(((surface.ddpfPixelFormat.dwGBitMask & andVal) >> gshift) << (8 - gcount));
				b = uchar(((surface.ddpfPixelFormat.dwBBitMask & andVal) >> bshift) << (8 - bcount));
				*pDest = RGBA(r, g, b, 0xFF);
				pDest++;
				xoff += xadd;
			}

			yoff += yadd;
			curY = (surface.lPitch >> 1) * (yoff >> 16);
			pDest += dadd - w;
		}
	}
	else if (surface.ddpfPixelFormat.dwRGBBitCount == 24)
	{
		for (int i = 0; i < h; i++)
		{
			xoff = 0;

			for (int j = 0; j < w; j++)
			{
				r = pSrc[curY + (xoff >> 16)];
				g = pSrc[curY + 1 + (xoff >> 16)];
				b = pSrc[curY + 2 + (xoff >> 16)];
				*pDest = RGBA(r, g, b, 0xFF);
				pDest++;
				xoff += 3 * xadd;
			}

			yoff += yadd;
			curY = surface.lPitch * (yoff >> 16);
			pDest += dadd - w;
		}
	}
	else if (surface.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		for (int i = 0; i < h; i++)
		{
			xoff = 0;

			for (int j = 0; j < w; j++)
			{
				r = pSrc[curY + (xoff >> 16)];
				g = pSrc[curY + 1 + (xoff >> 16)];
				b = pSrc[curY + 2 + (xoff >> 16)];
				*pDest = RGBA(r, g, b, 0xFF);
				pDest++;
				xoff += xadd << 2;
			}

			yoff += yadd;
			curY = surface.lPitch * (yoff >> 16);
			pDest += dadd - w;
		}
	}
}

void RGBM_Mono(uchar* r, uchar* g, uchar* b)
{
	uchar c;

	c = (*r + *b) >> 1;
	*r = c;
	*g = c;
	*b = c;
}

void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACE4 surface)
{
	DDSURFACEDESC2 tSurf;
	ushort* pTexture;
	ushort* pSrc;

	memset(&tSurf, 0, sizeof(tSurf));
	tSurf.dwSize = sizeof(DDSURFACEDESC2);
	surface->Lock(0, &tSurf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
	pSrc = (ushort*)tSurf.lpSurface;
	pTexture = (ushort*)malloc(0x40000);

	MemBltSurf(pTexture, 0, 0, 256, 256, 256, pSrc, 0, 0, tSurf, 1.0F, 1.0F);
	MonoScreen[0].surface = CreateTexturePage(256, 256, 0, (long*)pTexture, RGBM_Mono, 0);
	DXAttempt(MonoScreen[0].surface->QueryInterface(IID_IDirect3DTexture2, (void**)&MonoScreen[0].tex));

	MemBltSurf(pTexture, 0, 0, 256, 256, 256, pSrc, 256, 0, tSurf, 1.0F, 1.0F);
	MonoScreen[1].surface = CreateTexturePage(256, 256, 0, (long*)pTexture, RGBM_Mono, 0);
	DXAttempt(MonoScreen[1].surface->QueryInterface(IID_IDirect3DTexture2, (void**)&MonoScreen[1].tex));

	MemBltSurf(pTexture, 0, 0, 128, 256, 256, pSrc, 512, 0, tSurf, 1.0F, 1.0F);
	MemBltSurf(pTexture, 128, 0, 128, 224, 256, pSrc, 512, 256, tSurf, 1.0F, 1.0F);
	MonoScreen[2].surface = CreateTexturePage(256, 256, 0, (long*)pTexture, RGBM_Mono, 0);
	DXAttempt(MonoScreen[2].surface->QueryInterface(IID_IDirect3DTexture2, (void**)&MonoScreen[2].tex));

	MemBltSurf(pTexture, 0, 0, 256, 224, 256, pSrc, 0, 256, tSurf, 1.0F, 1.0F);
	MonoScreen[3].surface = CreateTexturePage(256, 256, 0, (long*)pTexture, RGBM_Mono, 0);
	DXAttempt(MonoScreen[3].surface->QueryInterface(IID_IDirect3DTexture2, (void**)&MonoScreen[3].tex));

	MemBltSurf(pTexture, 0, 0, 256, 224, 256, pSrc, 256, 256, tSurf, 1.0F, 1.0F);
	MonoScreen[4].surface = CreateTexturePage(256, 256, 0, (long*)pTexture, RGBM_Mono, 0);
	DXAttempt(MonoScreen[4].surface->QueryInterface(IID_IDirect3DTexture2, (void**)&MonoScreen[4].tex));

	surface->Unlock(0);
	free(pTexture);
}

void FreeMonoScreen()
{
	if (MonoScreenOn == 1)
	{
		if (MonoScreen[0].surface)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen[0].surface, MonoScreen[0].surface->Release());
			MonoScreen[0].surface = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

		if (MonoScreen[1].surface)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen[1].surface, MonoScreen[1].surface->Release());
			MonoScreen[1].surface = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

		if (MonoScreen[2].surface)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen[2].surface, MonoScreen[2].surface->Release());
			MonoScreen[2].surface = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

		if (MonoScreen[3].surface)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen[3].surface, MonoScreen[3].surface->Release());
			MonoScreen[3].surface = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

		if (MonoScreen[4].surface)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Surface", MonoScreen[4].surface, MonoScreen[4].surface->Release());
			MonoScreen[4].surface = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Surface");

		if (MonoScreen[0].tex)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen[0].tex, MonoScreen[0].tex->Release());
			MonoScreen[0].tex = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");

		if (MonoScreen[1].tex)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen[1].tex, MonoScreen[1].tex->Release());
			MonoScreen[1].tex = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");

		if (MonoScreen[2].tex)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen[2].tex, MonoScreen[2].tex->Release());
			MonoScreen[2].tex = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");

		if (MonoScreen[3].tex)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen[3].tex, MonoScreen[3].tex->Release());
			MonoScreen[3].tex = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");

		if (MonoScreen[4].tex)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Mono Screen Texture", MonoScreen[4].tex, MonoScreen[4].tex->Release());
			MonoScreen[4].tex = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Mono Screen Texture");
	}

	MonoScreenOn = 0;
}

void S_DrawTile(long x, long y, long w, long h, IDirect3DTexture2* t, long tU, long tV, long tW, long tH, long c0, long c1, long c2, long c3)
{
	D3DTLBUMPVERTEX v[4];
	D3DTLBUMPVERTEX tri[3];
	float u1, v1, u2, v2;

	u1 = float(tU * (1.0F / 256.0F));
	v1 = float(tV * (1.0F / 256.0F));
	u2 = float((tW + tU) * (1.0F / 256.0F));
	v2 = float((tH + tV) * (1.0F / 256.0F));

	v[0].sx = (float)x;
	v[0].sy = (float)y;
	v[0].sz = 0.995F;
	v[0].tu = u1;
	v[0].tv = v1;
	v[0].rhw = 1;
	v[0].color = c0;
	v[0].specular = 0xFF000000;

	v[1].sx = float(w + x);
	v[1].sy = (float)y;
	v[1].sz = 0.995F;
	v[1].tu = u2;
	v[1].tv = v1;
	v[1].rhw = 1;
	v[1].color = c1;
	v[1].specular = 0xFF000000;

	v[2].sx = float(w + x);
	v[2].sy = float(h + y);
	v[2].sz = 0.995F;
	v[2].tu = u2;
	v[2].tv = v2;
	v[2].rhw = 1;
	v[2].color = c3;
	v[2].specular = 0xFF000000;

	v[3].sx = (float)x;
	v[3].sy = float(h + y);
	v[3].sz = 0.995F;
	v[3].tu = u1;
	v[3].tv = v2;
	v[3].rhw = 1;
	v[3].color = c2;
	v[3].specular = 0xFF000000;

	App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
	App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0);
	DXAttempt(App.dx.lpD3DDevice->SetTexture(0, t));
	tri[0] = v[0];
	tri[1] = v[2];
	tri[2] = v[3];
	App.dx.lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, FVF, v, 3, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
	App.dx.lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, FVF, tri, 3, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
	App.dx.lpD3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 1);

	if (App.Filtering)
	{
		App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		App.dx.lpD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	}
}

void S_DisplayMonoScreen()
{
	long x[4];
	long y[4];
	ulong col;

	if (MonoScreenOn == 1)
	{
		for (int i = 0; i < 3; i++)
		{
			x[i] = phd_winxmin + phd_winwidth * MonoScreenX[i] / 640;
			y[i] = phd_winymin + phd_winheight * MonoScreenY[i] / 480;
		}

		x[3] = phd_winxmin + phd_winwidth * MonoScreenX[3] / 640;
		RestoreFPCW(FPCW);
		col = 0xFFFFFF80;
		S_DrawTile(x[0], y[0], x[1] - x[0], y[1] - y[0], MonoScreen[0].tex, 0, 0, 256, 256, col, col, col, col);
		S_DrawTile(x[1], y[0], x[2] - x[1], y[1] - y[0], MonoScreen[1].tex, 0, 0, 256, 256, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80);
		S_DrawTile(x[2], y[0], x[3] - x[2], y[1] - y[0], MonoScreen[2].tex, 0, 0, 128, 256, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80);
		S_DrawTile(x[0], y[1], x[1] - x[0], y[2] - y[1], MonoScreen[3].tex, 0, 0, 256, 224, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80);
		S_DrawTile(x[1], y[1], x[2] - x[1], y[2] - y[1], MonoScreen[4].tex, 0, 0, 256, 224, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80);
		S_DrawTile(x[2], y[1], x[3] - x[2], y[2] - y[1], MonoScreen[2].tex, 128, 0, 128, 224, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80, 0xFFFFFF80);
		MungeFPCW(&FPCW);
	}
}

long S_LoadSave(long load_or_save, long mono)
{
	long fade, ret;

	fade = 0;

	if (!mono)
		CreateMonoScreen();

	GetSaveLoadFiles();
	InventoryActive = 1;

	while (1)
	{
		S_InitialisePolyList();

		if (fade)
			dbinput = 0;
		else
			S_UpdateInput();

		SetDebounce = 1;
		S_DisplayMonoScreen();
		ret = DoLoadSave(load_or_save);
		UpdatePulseColour();
		S_OutputPolyList();
		S_DumpScreen();

		if (ret >= 0)
		{
			if (load_or_save & IN_SAVE)
			{
				sgSaveGame();
				S_SaveGame(ret);
				GetSaveLoadFiles();
				break;
			}

			fade = ret + 1;
			S_LoadGame(ret);
			SetFade(0, 255);
			ret = -1;
		}

		if (fade && DoFade == 2)
		{
			ret = fade - 1;
			break;
		}

		if (input & IN_OPTION)
		{
			ret = -1;
			break;
		}

		if (MainThread.ended)
			break;
	}

	TIME_Init();

	if (!mono)
		FreeMonoScreen();

	InventoryActive = 0;
	return ret;
}

void LoadScreen(long screen, long pathNum)
{
	FILE* file;
	DDPIXELFORMAT pixel_format;
	DDSURFACEDESC2 surf;
	void* pic;
	ushort* pSrc;
	ushort* pDst;
	ushort col, r, g, b;

	memset(&surf, 0, sizeof(surf));
	memset(&pixel_format, 0, sizeof(pixel_format));
	surf.dwSize = sizeof(DDSURFACEDESC2);
	surf.dwWidth = 640;
	surf.dwHeight = 480;
	pixel_format.dwSize = sizeof(DDPIXELFORMAT);
	pixel_format.dwFlags = DDPF_RGB;
	pixel_format.dwRGBBitCount = 16;
	pixel_format.dwRBitMask = 0xF800;
	pixel_format.dwGBitMask = 0x7E0;
	pixel_format.dwBBitMask = 0x1F;
	pixel_format.dwRGBAlphaBitMask = 0;
	memcpy(&surf.ddpfPixelFormat, &pixel_format, sizeof(surf.ddpfPixelFormat));
	surf.dwFlags = DDSD_PIXELFORMAT | DDSD_HEIGHT | DDSD_WIDTH | DDSD_CAPS;
	surf.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	file = FileOpen(screen_paths[pathNum]);

	if (DXCreateSurface(G_dxptr->lpDD, &surf, &screen_surface) && file)
	{
		pic = malloc(0x96000);
		fseek(file, 0x96000 * screen, SEEK_SET);
		fread(pic, 0x96000, 1, file);
		fclose(file);
		memset(&surf, 0, sizeof(surf));
		surf.dwSize = sizeof(DDSURFACEDESC2);
		screen_surface->Lock(0, &surf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
		pDst = (ushort*)surf.lpSurface;
		pSrc = (ushort*)pic;
		
		for (int i = 0; i < 0x4B000; i++, pSrc++, pDst++)
		{
			col = *pSrc;
			r = (col >> 10) & 0x1F;
			g = (col >> 5) & 0x1F;
			b = col & 0x1F;
			*pDst = (r << 11) | (g << 6) | b;
		}

		screen_surface->Unlock(0);
		free(pic);
		MonoScreenOn = 1;
	}
	else
		Log(0, "WHORE!");
}

void ReleaseScreen()
{
	MonoScreenOn = 0;

	if (screen_surface)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Picture Surface", screen_surface, screen_surface->Release());
		screen_surface = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Picture Surface");
}

void DrawLoadingScreen()
{
#if 0
	DDSURFACEDESC2 surf;
	ushort* pSrc;
	uchar* pDest;
	float xoff, yoff, xadd, yadd;
	long w, h, val;
	ushort sVal;

	if (!(App.dx.Flags & 0x80))	//software
	{
		memset(&surf, 0, sizeof(surf));
		surf.dwSize = sizeof(DDSURFACEDESC2);
		screen_surface->Lock(0, &surf, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
		pSrc = (ushort*)surf.lpSurface;
		pDest = (uchar*)MMXGetDeviceViewPort(App.dx.lpD3DDevice);
		MMXGetBackSurfWH(w, h);
		xadd = 640.0F / (float)w;
		yadd = 480.0F / (float)h;
		xoff = 0;
		yoff = 0;

		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				val = long(640 * yoff + xoff);
				xoff += xadd;
				sVal = pSrc[val];
				pDest[0] = sVal << 3;			//b
				pDest[1] = (sVal >> 6) << 3;	//g
				pDest[2] = (sVal >> 11) << 3;	//r
				pDest[3] = 0xFF;				//a
				pDest += 4;
			}

			xoff = 0;
			yoff += yadd;
		}

		screen_surface->Unlock(0);
	}
	else
#endif
		G_dxptr->lpBackBuffer->Blt(0, screen_surface, 0, DDBLT_WAIT, 0);
}

long GetSaveLoadFiles()
{
	FILE* file;
	SAVEFILE_INFO* pSave;
	SAVEGAME_INFO save_info;
	static long nSaves;
	char name[75];

	SaveCounter = 0;

	for (int i = 0; i < 15; i++)
	{
		pSave = &SaveGames[i];
		wsprintf(name, "savegame.%d", i);
		file = fopen(name, "rb");
		Log(0, "Attempting to open %s", name);

		if (!file)
		{
			pSave->valid = 0;
			strcpy(pSave->name, SCRIPT_TEXT(TXT_Empty_Slot));
			continue;
		}

		Log(0, "Opened OK");
		fread(&pSave->name, sizeof(char), 75, file);
		fread(&pSave->num, sizeof(long), 1, file);
		fread(&pSave->days, sizeof(short), 1, file);
		fread(&pSave->hours, sizeof(short), 1, file);
		fread(&pSave->minutes, sizeof(short), 1, file);
		fread(&pSave->seconds, sizeof(short), 1, file);
		fread(&save_info, 1, sizeof(SAVEGAME_INFO), file);
		fclose(file);

		if (pSave->num > SaveCounter)
			SaveCounter = pSave->num;

		pSave->valid = 1;
		nSaves++;
		Log(0, "Validated savegame");
	}

	SaveCounter++;
	return nSaves;
}

void DoSlider(long x, long y, long width, long height, long pos, long c1, long c2, long c3)
{
	D3DTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float x2, sx, sy;
	static float V;

	nPolyType = 4;
	V += 0.0099999998F;

	if (V > 0.99000001F)
		V = 0;

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;
	x2 = (float)phd_winxmax / 640.0F;
	sx = width * x2;
	sy = ((float)phd_winymax / 480.0F) * (height >> 1);
	x2 *= x;

	v[0].sx = x2;
	v[0].sy = (float)y;
	v[0].sz = f_mznear;
	v[0].rhw = f_moneoznear - 2.0F;
	v[0].color = c1;
	v[0].specular = 0xFF000000;

	v[1].sx = sx + x2;
	v[1].sy = (float)y;
	v[1].sz = f_mznear;
	v[1].rhw = f_moneoznear - 2.0F;
	v[1].color = c1;
	v[1].specular = 0xFF000000;

	v[2].sx = sx + x2;
	v[2].sy = (float)y + sy;
	v[2].sz = f_mznear;
	v[2].rhw = f_moneoznear - 2.0F;
	v[2].color = c2;
	v[2].specular = 0xFF000000;

	v[3].sx = x2;
	v[3].sy = (float)y + sy;
	v[3].sz = f_mznear;
	v[3].rhw = f_moneoznear - 2.0F;
	v[3].color = c2;
	v[3].specular = 0xFF000000;

	tex.tpage = ushort(nTextures - 1);
	tex.drawtype = 0;
	tex.flag = 0;
	tex.u1 = 0;
	tex.v1 = V;
	tex.u2 = 1;
	tex.v2 = V;
	tex.u3 = 1;
	tex.v3 = V + 0.0099999998F;
	tex.u4 = 0;
	tex.v4 = V + 0.0099999998F;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

	v[0].sx = x2;
	v[0].sy = (float)y + sy;
	v[0].sz = f_mznear;
	v[0].rhw = f_moneoznear - 2.0F;
	v[0].color = c2;
	v[0].specular = 0xFF000000;

	v[1].sx = sx + x2;
	v[1].sy = (float)y + sy;
	v[1].sz = f_mznear;
	v[1].rhw = f_moneoznear - 2.0F;
	v[1].color = c2;
	v[1].specular = 0xFF000000;


	v[2].sx = sx + x2;
	v[2].sy = (float)y + 2 * sy;
	v[2].sz = f_mznear;
	v[2].rhw = f_moneoznear - 2.0F;
	v[2].color = c1;
	v[2].specular = 0xFF000000;

	v[3].sx = x2;
	v[3].sy = (float)y + 2 * sy;
	v[3].sz = f_moneoznear - 2.0F;
	v[3].rhw = v[0].rhw;
	v[3].color = c1;
	v[3].specular = 0xFF000000;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

	v[0].sx = x2 - 1;
	v[0].sy = float(y - 1);
	v[0].sz = f_mznear + 2.0F;
	v[0].rhw = f_moneoznear - 3.0F;
	v[0].color = 0xFFFFFFFF;
	v[0].specular = 0xFF000000;

	v[1].sx = sx + x2 + 1;
	v[1].sy = float(y - 1);
	v[1].sz = f_mznear + 2.0F;
	v[1].rhw = f_moneoznear - 3.0F;
	v[1].color = 0xFFFFFFFF;
	v[1].specular = 0xFF000000;

	v[2].sx = sx + x2 + 1;
	v[2].sy = ((float)y + 2 * sy) + 1;
	v[2].sz = f_mznear + 2.0F;
	v[2].rhw = f_moneoznear - 3.0F;
	v[2].color = 0xFFFFFFFF;
	v[2].specular = 0xFF000000;

	v[3].sx = x2 - 1;
	v[3].sy = ((float)y + 2 * sy) + 1;
	v[3].sz = f_mznear + 2.0F;
	v[3].rhw = f_moneoznear - 3.0F;
	v[3].color = 0xFFFFFFFF;
	v[3].specular = 0xFF000000;
	tex.tpage = 0;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);

	sx = pos * sx / 100 + x2;

	v[0].sx = x2;
	v[0].sy = (float)y;
	v[0].sz = f_mznear - 1.0F;
	v[0].rhw = f_moneoznear - 1.0F;
	v[0].color = c3;
	v[0].specular = 0xFF000000;

	v[1].sx = sx + 1;
	v[1].sy = (float)y;
	v[1].sz = f_mznear - 1.0F;
	v[1].rhw = f_moneoznear - 1.0F;
	v[1].color = c3;
	v[1].specular = 0xFF000000;

	v[2].sx = sx;
	v[2].sy = (float)y + 2 * sy;
	v[2].sz = f_mznear - 1.0F;
	v[2].rhw = f_moneoznear - 1.0F;
	v[2].color = c3;
	v[2].specular = 0xFF000000;

	v[3].sx = x2 - 1;
	v[3].sy = (float)y + 2 * sy;
	v[3].sz = f_mznear - 1.0F;
	v[3].rhw = f_moneoznear - 1.0F;
	v[3].color = c3;
	v[3].specular = 0xFF000000;

	tex.tpage = 0;
	tex.drawtype = 2;
	AddQuadSorted(v, 0, 1, 2, 3, &tex, 0);
}

#pragma warning(push)
#pragma warning(disable : 4244)
long S_DisplayPauseMenu(long reset)
{
	static long menu, selection = 1;
	long y;

	if (!menu)
	{
		if (reset)
		{
			selection = reset;
			menu = 0;
		}
		else
		{
			y = phd_centery - font_height;
			PrintString(phd_centerx, y - ((3 * font_height) >> 1), 6, SCRIPT_TEXT(TXT_Paused), FF_CENTER);
			PrintString(phd_centerx, y, selection & 1 ? 1 : 2, SCRIPT_TEXT(TXT_Statistics), FF_CENTER);
			PrintString(phd_centerx, y + font_height, selection & 2 ? 1 : 2, SCRIPT_TEXT(TXT_Options), FF_CENTER);
			PrintString(phd_centerx, y + 2 * font_height, selection & 4 ? 1 : 2, SCRIPT_TEXT(TXT_Exit_to_Title), FF_CENTER);

			if (dbinput & IN_FORWARD)
			{
				if (selection > 1)
					selection >>= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_BACK)
			{
				if (selection < 4)
					selection <<= 1;

				SoundEffect(SFX_MENU_CHOOSE, 0, SFX_ALWAYS);
			}

			if (dbinput & IN_DESELECT)
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
				return 1;
			}

			if (dbinput & IN_SELECT && !keymap[DIK_LALT])
			{
				SoundEffect(SFX_MENU_SELECT, 0, SFX_DEFAULT);

				if (selection & 1)
					menu = 2;
				else if (selection & 2)
					menu = 1;
				else if (selection & 4)
					return 8;
			}
		}
	}
	else if (menu == 1)
	{
		DoOptions();

		if (dbinput & IN_DESELECT)
		{
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
		}
	}
	else if (menu == 2)
	{
		DoStatScreen();

		if (dbinput & IN_DESELECT)
		{
			menu = 0;
			SoundEffect(SFX_MENU_SELECT, 0, SFX_ALWAYS);
		}
	}

	return 0;
}
#pragma warning (pop)

long S_PauseMenu()
{
	long fade, ret;

	fade = 0;
	CreateMonoScreen();
	S_DisplayPauseMenu(1);
	InventoryActive = 1;

	do
	{
		S_InitialisePolyList();

		if (fade)
			dbinput = 0;
		else
			S_UpdateInput();

		SetDebounce = 1;
		S_DisplayMonoScreen();
		ret = S_DisplayPauseMenu(0);
		UpdatePulseColour();
		S_OutputPolyList();
		S_DumpScreen();

		if (ret == 1)
			break;

		if (ret == 8)
		{
			fade = 8;
			ret = 0;
			SetFade(0, 255);
		}

		if (fade && DoFade == 2)
		{
			ret = fade;
			break;
		}

	} while (!MainThread.ended);

	TIME_Init();
	FreeMonoScreen();
	InventoryActive = 0;
	return ret;
}

long IsHardware()
{
#if 1
	return 1;
#else
	return App.dx.Flags & 0x80;
#endif
}

long IsSuperLowRes()
{
#if 1
	return 0;
#else
	long w, h;

	MMXGetBackSurfWH(w, h);

	if (w < 400)
		return 1;

	if (w <= 512)
		return 2;

	return 0;
#endif
}

void DoFrontEndOneShotStuff()
{
	static long done;

	if (!done)
	{
		PlayFmvNow(0, 0);
		PlayFmvNow(1, 0);
		done = 1;
	}
}

long FindSFCursor(long in, long selection)
{
	long num, bak;

	num = 0;

	while (selection != 1)
	{
		selection >>= 1;
		num++;
	}

	bak = num;

	if (in & IN_FORWARD && num)
		do num--; while (num && !SpecialFeaturesPage[num]);

	if (in & IN_BACK && num < 4)
		do num++; while (num < 4 && !SpecialFeaturesPage[num]);

	if (!SpecialFeaturesPage[num])
		num = bak;

	return 1 << num;
}

void CalculateNumSpecialFeatures()
{
	SpecialFeaturesPage[0] = 0;
	SpecialFeaturesPage[1] = 0;
	SpecialFeaturesPage[2] = 0;
	SpecialFeaturesPage[3] = 0;
	SpecialFeaturesPage[4] = 0;
	NumSpecialFeatures = 0;

	for (int i = 0; i < 4; i++)
	{
		if (savegame.CampaignSecrets[i] >= 9)
		{
			SpecialFeaturesPage[i] = 1;
			NumSpecialFeatures++;
		}
	}
}

#pragma warning(push)
#pragma warning(disable : 4244)
void SpecialFeaturesDisplayScreens(long num)
{
	static long start[4] = { 0, 0, 0, 0 };
	static long nPics[4] = { 12, 11, 12, 23 };
	long first, max, pos, count;

	first = start[num];
	max = nPics[num];
	pos = 0;
	count = 0;
	LoadScreen(first, num);

	while (!MainThread.ended && !(dbinput & IN_DESELECT))
	{
		_BeginScene();
		InitBuckets();
		InitialiseSortList();
		S_UpdateInput();
		SetDebounce = 1;

		if (count < 2)
		{
			count++;
			DrawLoadingScreen();
		}
		else if (count == 2)
		{
			count = 3;
			ReleaseScreen();
		}

		if (!pos)
			PrintString(font_height, phd_winymax - font_height, 6, "Next \x1B", 0);
		else if (pos < max)
			PrintString(font_height, phd_winymax - font_height, 6, "\x19 Previous / Next \x1b", 0);
		else
			PrintString(font_height, phd_winymax - font_height, 6, "\x19 Previous", 0);

		UpdatePulseColour();
		S_OutputPolyList();
		S_DumpScreen();

		if (dbinput & IN_LEFT && pos)
		{
			pos--;
			LoadScreen(pos + first, num);
			count = 0;
		}
		if (dbinput & IN_RIGHT && pos < max)
		{
			pos++;
			LoadScreen(pos + first, num);
			count = 0;
		}
	}

	dbinput &= ~IN_DESELECT;
	ReleaseScreen();
}
#pragma warning (pop)

void DoSpecialFeaturesServer()
{
	switch (SpecialFeaturesNum)
	{
	case 0:
		SpecialFeaturesDisplayScreens(0);
		break;

	case 1:
		SpecialFeaturesDisplayScreens(1);
		break;

	case 2:
		SpecialFeaturesDisplayScreens(2);
		break;

	case 3:
		SpecialFeaturesDisplayScreens(3);
		break;
	}

	SpecialFeaturesNum = -1;
}
