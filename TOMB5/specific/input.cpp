#include "../tomb5/pch.h"
#include "input.h"
#include "dxshell.h"
#include "../game/lara_states.h"
#include "../game/sound.h"
#include "3dmath.h"
#include "../game/text.h"
#include "winmain.h"
#include "../game/camera.h"
#include "../game/spotcam.h"
#include "../game/control.h"
#include "LoadSave.h"
#include "../game/lara.h"
#include "../game/savegame.h"
#include "../game/gameflow.h"

const char* KeyboardButtons[272] =
{
	0,
	"Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "Bksp",
	"Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "<", ">", "Return",
	"Ctrl", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
	"Shift", "#", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift",
	"Padx", "Alt", "Space", "Caps", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Nmlk", 0,
	"Pad7", "Pad8", "Pad9", "Pad-",
	"Pad4", "Pad5", "Pad6", "Pad+",
	"Pad1", "Pad2", "Pad3",
	"Pad0", "Pad.", 0, 0, "\\", 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Enter", "Ctrl", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Shift", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Pad/", 0, 0, "Alt", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Home", "Up", "PgUp", 0, "Left", 0, "Right", 0, "End", "Down", "PgDn", "Ins", "Del",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

const char* GermanKeyboard[272] =
{
	0,
	"Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "=", "`", "Entf",
	"Tab", "Q", "W", "E", "R", "T", "Z", "U", "I", "O", "P", "~u", "x", "Enter",
	"Strg", "A", "S", "D", "F", "G", "H", "J", "K", "L", "~o", "~a", "( ",
	"Shift", "#", "Y", "X", "C", "V", "B", "N", "M", ",", ".", "-", "Shift",
	"Num x", "Alt", "Leert.", "Caps", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Nmlk", 0,
	"Num7", "Num8", "Num9", "Num-",
	"Num4", "Num5", "Num6", "Num+",
	"Num1", "Num2", "Num3",
	"Num0", "Num.", 0, 0, ">", 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Enter", "Strg", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Shift", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Num/", 0, 0, "Alt", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Pos1", "Hoch", "BdAuf", 0, "Links", 0, "Rechts", 0, "Ende", "Runter", "BdAb", "Einfg", "Entf",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

const char* JoyStickButtons[16] =
{
	"Joy 1", "Joy 2", "Joy 3", "Joy 4", "Joy 5", "Joy 6", "Joy 7", "Joy 8",
	"Joy 9", "Joy 10", "Joy 11", "Joy 12", "Joy 13", "Joy 14", "Joy 15", "Joy 16"
};

long jLayout[32] = { 5, 3, 4, 0, 1, 2, 9, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
long defaultJLayout[32] = { 5, 3, 4, 0, 1, 2, 9, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

short layout[2][18] =
{
	{ DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_RALT, DIK_RCONTROL,
	DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_DELETE, DIK_PGDN, DIK_P, DIK_RETURN },

	{ DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT, DIK_RALT, DIK_RCONTROL,
	DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_DELETE, DIK_PGDN, DIK_P, DIK_RETURN }
};

long conflict[18];
long input;
long dbinput;
long inputBusy;
long joystick_read;
long joystick_read_x;
long joystick_read_y;
long joystick_read_fire;

static long joy_fire;
static double screen_sizer = 1.0;
static double game_sizer = 1.0;

long Key(long number)
{
	short key = layout[1][number];

	if (number >= 256)
		return joy_fire & (1 << number);

	if (keymap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return keymap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return keymap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return keymap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return keymap[DIK_RSHIFT];
	case DIK_RMENU:
		return keymap[DIK_LMENU];
	case DIK_LMENU:
		return keymap[DIK_RMENU];
	}

	if (conflict[number])
		return 0;

	key = layout[0][number];

	if (keymap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return keymap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return keymap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return keymap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return keymap[DIK_RSHIFT];
	case DIK_RMENU:
		return keymap[DIK_LMENU];
	case DIK_LMENU:
		return keymap[DIK_RMENU];
	}

	return 0;
}

long S_UpdateInput()
{
	static long linput;
	static long LookCnt;
	static long med_hotkey_timer;
	static long cheat_code;
	static long weird = 0;
	static long joy_x, joy_y;
	long flag;
	short state;
	static bool flare_no_db = 0;
	bool debounce;

	flag = 0;
	debounce = SetDebounce;

	DXUpdateJoystick();
	DXReadKeyboard(keymap);

	if (ControlMethod == 1)
		joy_fire = ReadJoystick(joy_x, joy_y);

	linput = 0;

	if (ControlMethod == 1)
	{
		if (joy_x < -8)
			linput = IN_LEFT;
		else if (joy_x > 8)
			linput = IN_RIGHT;

		if (joy_y > 8)
			linput |= IN_BACK;
		else if (joy_y < -8)
			linput |= IN_FORWARD;
	}

	if (Key(0))
		linput |= IN_FORWARD;

	if (Key(1))
		linput |= IN_BACK;

	if (Key(2))
		linput |= IN_LEFT;

	if (Key(3))
		linput |= IN_RIGHT;

	if (Key(4))
		linput |= IN_DUCK;

	if (Key(5))
		linput |= IN_SPRINT;

	if (Key(6))
		linput |= IN_WALK;

	if (Key(7))
		linput |= IN_JUMP;

	if (Key(8))
		linput |= IN_SELECT | IN_ACTION;

	if (Key(9))
		linput |= IN_DRAW;

	if (ControlMethod == 1)
	{
		if (Key(jLayout[3] + 256))
			linput |= IN_JUMP;

		if (Key(jLayout[4] + 256))
			linput |= IN_SELECT | IN_ACTION;

		if (Key(jLayout[5] + 256))
			linput |= IN_DRAW;

		if (Key(jLayout[1] + 256))
			linput |= IN_SPRINT;

		if (Key(jLayout[2] + 256))
			linput |= IN_WALK;

		if (Key(jLayout[0] + 256))
			linput |= IN_DUCK;

		if (Key(jLayout[6] + 256))
			flag = 1;
	}

	if (Key(10) || flag)
	{
		if (!flare_no_db)
		{
			state = lara_item->current_anim_state;

			if (state == AS_ALL4S || state == AS_CRAWL || state == AS_ALL4TURNL ||
				state == AS_ALL4TURNR || state == AS_CRAWLBACK || state == AS_CRAWL2HANG)
			{
				SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
				flare_no_db = 1;
			}
			else
			{
				flare_no_db = 0;
				linput |= IN_FLARE;
			}
		}
	}
	else
		flare_no_db = 0;

	if (Key(11))
		linput |= IN_LOOK;

	if (Key(12))
		linput |= IN_ROLL;

	if (Key(13))
		linput |= IN_OPTION;

	if (Key(14))
		linput |= IN_WALK | IN_LEFT;

	if (Key(15))
		linput |= IN_WALK | IN_RIGHT;

	if (Key(16))
		linput |= IN_PAUSE;

	if (Key(17))
		linput |= IN_SELECT;

	if (ControlMethod == 1)
	{
		if (Key(jLayout[7] + 256))
			linput |= IN_LOOK;

		if (Key(jLayout[8] + 256))
			linput |= IN_ROLL;

		if (Key(jLayout[9] + 256))
			linput |= IN_OPTION;
	}

	if (keymap[DIK_ESCAPE])
		linput |= IN_DESELECT | IN_OPTION;

	if (lara.gun_status == LG_READY)
	{
		savegame.AutoTarget = (uchar)App.AutoTarget;

		if (linput & IN_LOOK)
		{
			if (LookCnt >= 6)
				LookCnt = 100;
			else
			{
				linput &= ~IN_LOOK;
				LookCnt++;
			}
		}
		else
		{
			if (LookCnt && LookCnt != 100)
				linput |= IN_TARGET;

			LookCnt = 0;
		}
	}

	if (keymap[DIK_0])
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
				if (lara.num_small_medipack && lara.num_small_medipack != -1)
					lara.num_small_medipack--;

				if (lara.num_small_medipack)
				{
					lara.dpoisoned = 0;
					lara_item->hit_points += 500;

					if (lara_item->hit_points > 1000)
					{
						lara_item->hit_points = 1000;
						SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
						savegame.Game.HealthUsed++;
					}
				}

				med_hotkey_timer = 15;
			}
		}
	}
	else if (keymap[DIK_9])
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
				if (lara.num_large_medipack && lara.num_large_medipack != -1)
					lara.num_large_medipack--;

				if (lara.num_large_medipack)
				{
					lara.dpoisoned = 0;
					lara_item->hit_points += 1000;

					if (lara_item->hit_points > 1000)
					{
						lara_item->hit_points = 1000;
						SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
						savegame.Game.HealthUsed++;
					}
				}

				med_hotkey_timer = 15;
			}
		}
	}
	else if (med_hotkey_timer)
		med_hotkey_timer--;

	if (keymap[DIK_F10])
		linput |= IN_E;

	if (linput & IN_WALK && !(linput & (IN_FORWARD | IN_BACK)))
	{
		if (linput & IN_LEFT)
			linput = (linput & ~IN_LEFT) | IN_LSTEP;
		else if (linput & IN_RIGHT)
			linput = (linput & ~IN_RIGHT) | IN_RSTEP;
	}

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (linput & IN_ROLL && BinocularRange)
		linput &= ~IN_ROLL;

	if ((linput & (IN_RIGHT | IN_LEFT)) == (IN_RIGHT | IN_LEFT))
		linput -= IN_RIGHT | IN_LEFT;

	if (debounce)
		dbinput = inputBusy;

	if (!gfGameMode && Gameflow->LoadSaveEnabled)
	{
		if (keymap[DIK_F5])
			linput |= IN_SAVE;

		if (keymap[DIK_F6])
			linput |= IN_LOAD;
	}

	if (Gameflow->CheatEnabled)
	{
		if (linput)
			cheat_code = 0;

		switch (cheat_code)
		{
			case 0:

				if (keymap[DIK_D])
					cheat_code = 1;

				break;

			case 1:

				if (keymap[DIK_O])
					cheat_code = 2;

				break;

			case 2:

				if (keymap[DIK_Z])
					cheat_code = 3;

				break;

			case 3:

				if (keymap[DIK_Y])
					linput = IN_CHEAT;

				break;
		}
	}

	inputBusy = linput;

	if (lara.Busy)
	{
		linput &= IN_PAUSE | IN_LOOK | IN_OPTION | IN_RIGHT | IN_LEFT | IN_BACK | IN_FORWARD;

		if (linput & IN_FORWARD && linput & IN_BACK)
			linput ^= IN_BACK;
	}

	if (debounce)
		dbinput = inputBusy & (dbinput ^ inputBusy);

	input = linput;
	linput = weird;

	if (keymap[DIK_F])
		linput = 1;

	weird &= weird ^ linput;

	return 1;
}

long ReadJoystick(long& x, long& y)
{
	if (joystick_read)
	{
		x = joystick_read_x;
		y = joystick_read_y;
		return joystick_read_fire;
	}

	x = 0;
	y = 0;
	return 0;
}

static __inline void setup_screen_size()
{
	DXDISPLAYMODE* dm;
	long w, h, sw, sh;

	dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
	w = dm->w;
	h = dm->h;
	sw = long(w * screen_sizer);
	sh = long(h * screen_sizer);

	if (sw > w)
		sw = w;

	if (sh > h)
		sh = h;

	InitWindow((w - sw) / 2, (h - sh) / 2, sw, sh, 20, 20480, 80, w, h);
	InitFont();
}

void IncreaseScreenSize()
{
	if (screen_sizer != 1.0)
	{
		screen_sizer += 0.08;

		if (screen_sizer > 1.0)
			screen_sizer = 1.0;

		game_sizer = screen_sizer;
		setup_screen_size();
	}
}

void DecreaseScreenSize()
{
	if (screen_sizer != 0.44)
	{
		screen_sizer -= 0.08;

		if (screen_sizer < 0.44)
			screen_sizer = 0.44;

		game_sizer = screen_sizer;
		setup_screen_size();
	}
}
