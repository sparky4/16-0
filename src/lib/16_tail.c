/* Project 16 Source Code~
 * Copyright (C) 2012-2023 sparky4 & pngwen & andrius4669 & joncampbell123 & yakui-lover
 *
 * This file is part of Project 16.
 *
 * Project 16 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project 16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/*
 * 16 tail library
 * ment for lower level stuff
 */

#include "src/lib/16_tail.h"
//#include "src/lib/16text.h"

/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
	//unsigned		finscreen;
	memptr	screen=0;

	ClearMemory ();
	if (!*error)
	{
// #ifndef JAPAN
// 		CA_CacheGrChunk (ORDERSCREEN);
// 		screen = grsegs[ORDERSCREEN];
// #endif
// 		WriteConfig ();
	}
	else
	{
// 		CA_CacheGrChunk (ERRORSCREEN);
// 		screen = grsegs[ERRORSCREEN];
	}
//	Shutdown16();
//shut down included managers

/*
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ()
*/
#ifdef __16_US__
	US_Shutdown ();
#endif
#ifdef __16_SD__
	SD_Shutdown ();
#endif
#ifdef __16_PM__
	PM_Shutdown();
#endif
#ifdef __16_IN__
	IN_Shutdown();
#endif
#ifdef __16_VW__
	VW_Shutdown ();
#endif
#ifdef __16_CA__
	CA_Shutdown();
#endif
#ifdef __16_MM__
	MM_Shutdown();
#endif

	if (error && *error)
	{
		//movedata((unsigned)screen,7,0xb800,0,7*160);
		gotoxy (10,4);
		fprintf(stderr, "%s\n", error);
		gotoxy (1,8);
		exit(1);
	}
	else
	if (!error || !(*error))
	{
		clrscr();
#ifndef JAPAN
		movedata ((unsigned)screen,7,0xb800,0,4000);
		gotoxy(1,24);
#endif
//asm	mov	bh,0
//asm	mov	dh,23	// row
//asm	mov	dl,0	// collumn
//asm	mov ah,2
//asm	int	0x10
	}

	exit(0);
}

//===========================================================================

#ifdef __WATCOMC__
//
// for mary4 (XT)
// this is from my XT's BIOS
// http://www.phatcode.net/downloads.php?id=101
//
void turboXT(byte bakapee)
{
	__asm {
		push	ax
		push	bx
		push	cx
		in	al, 61h 			//; Read equipment flags
		xor	al, bakapee			//;   toggle speed
		out	61h, al 			//; Write new flags back

		mov	bx, 0F89h			//; low pitch blip
		and	al, 4				//; Is turbo mode set?
		jz	@@do_beep
		mov	bx, 52Eh			//; high pitch blip

	@@do_beep:
		mov	al, 10110110b		//; Timer IC 8253 square waves
		out	43h, al 			//;   channel 2, speaker
		mov	ax, bx
		out	42h, al 			//;   send low order
		mov	al, ah				//;   load high order
		out	42h, al 			//;   send high order
		in	al, 61h 			//; Read IC 8255 machine status
		push	ax
		or	al, 00000011b
		out	61h, al 			//; Turn speaker on
		mov	cx, 2000h
	@@delay:
		loop	@@delay
		pop	ax
		out	61h, al 			//; Turn speaker off
		pop	cx
		pop	bx
		pop	ax
	}
}
#endif
