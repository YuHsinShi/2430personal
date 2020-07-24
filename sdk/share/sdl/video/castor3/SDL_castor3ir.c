/*
   Simple DirectMedia Layer
   Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
   */
#include "SDL_config.h"

#include "../../events/SDL_events_c.h"

#include "SDL_castor3ir.h"
#include <unistd.h>
#include "ite/itp.h"

#if defined(CFG_IR0_ENABLE)
#define IR_PORT0	ITP_DEVICE_IR0
#else
#define IR_PORT0	0
#endif
#if defined(CFG_IR1_ENABLE)
#define IR_PORT1	ITP_DEVICE_IR0
#else
#define IR_PORT1	0
#endif
#if defined(CFG_IR2_ENABLE)
#define IR_PORT2	ITP_DEVICE_IR0
#else
#define IR_PORT2	0
#endif
#if defined(CFG_IR3_ENABLE)
#define IR_PORT3	ITP_DEVICE_IR0
#else
#define IR_PORT3	0
#endif

static ITPDeviceType IR_PORT[4] = {
	IR_PORT0, IR_PORT1,
	IR_PORT2, IR_PORT3,
};

void Castor3_InitIr(void)
{
    SDL_Keycode keymap[SDL_NUM_SCANCODES];

    /* Add default scancode to key mapping */
    SDL_GetDefaultKeymap(keymap);
    SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
}

typedef struct {
    int code;
    SDL_Scancode mapping_code;
} SDL_IR_TRANSCODE;

static SDL_IR_TRANSCODE Castor3_Ircodes[4][48] = {
//#include "ir_mapping_table.inc"
#ifdef CFG_IR0_RX_MAPPING_TABLE
	{
#include "ir0_mapping_table.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR1_RX_MAPPING_TABLE
	{
#include "ir1_mapping_table.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR2_RX_MAPPING_TABLE
	{
#include "ir2_mapping_table.inc" 
	},
#else
	{},
#endif
#ifdef CFG_IR3_RX_MAPPING_TABLE
	{
#include "ir3_mapping_table.inc" 
	},
#else
	{},
#endif
};

static SDL_Scancode TranslateKeycode(int keycode, int port)
{
    int idx;
    int size = SDL_arraysize(Castor3_Ircodes[port]);
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;

    for (idx = 0; idx < size; idx++)
    {
        if (Castor3_Ircodes[port][idx].code == keycode)
        {
			scancode = Castor3_Ircodes[port][idx].mapping_code;
            break;
        }
    }
    return scancode;
}

void Castor3_PumpIrEvent(void)
{
    ITPKeypadEvent ev;
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		if (read(IR_PORT[i], &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
		{
			if (ev.flags & ITP_KEYPAD_DOWN)
			{
				SDL_SendKeyboardKey(SDL_PRESSED, TranslateKeycode(ev.code, i));
			}
			else if (ev.flags & ITP_KEYPAD_UP)
			{
				SDL_SendKeyboardKey(SDL_RELEASED, TranslateKeycode(ev.code, i));
			}
		}
	}
}
