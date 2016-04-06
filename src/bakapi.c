/* Project 16 Source Code~
 * Copyright (C) 2012-2016 sparky4 & pngwen & andrius4669 & joncampbell123
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
 * You screen.heightould have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "src/bakapi.h"

/*
 * BAKAPEE!
 */
global_game_variables_t gvar;
static bakapee_t bakapee;
word key,d,xpos,ypos,xdir,ydir;
sword vgamodex_mode = 1; // 320x240 default
int ch=0x0;

void
main(int argc, char *argvar[])
{
	char *a;
	int i;

	// allow changing default mode from command line
	for (i=1;i < argc;) {
		a = argvar[i++];

		if (*a == '-') {
			do { a++; } while (*a == '-');

			if (!strcmp(a,"mx")) {
				// (based on src/lib/modex16.c)
				// 1 = 320x240
				// 2 = 160x120
				// 3 = 320x200
				// 4 = 192x144
				// 5 = 256x192
				vgamodex_mode = (sword)strtoul(argvar[i++],NULL,0);
			}
			else {
				fprintf(stderr,"Unknown switch %s\n",a);
				return;
			}
		}
		else {
			fprintf(stderr,"Unknown command arg %s\n",a);
			return;
		}
	}

	// DOSLIB: check our environment
	probe_dos();

	// DOSLIB: what CPU are we using?
	// NTS: I can see from the makefile Sparky4 intends this to run on 8088 by the -0 switch in CFLAGS.
	//      So this code by itself shouldn't care too much what CPU it's running on. Except that other
	//      parts of this project (DOSLIB itself) rely on CPU detection to know what is appropriate for
	//      the CPU to carry out tasks. --J.C.
	cpu_probe();

	// DOSLIB: check for VGA
	if (!probe_vga()) {
		printf("VGA probe failed\n");
		return;
	}
	// hardware must be VGA or higher!
	if (!(vga_state.vga_flags & VGA_IS_VGA)) {
		printf("This program requires VGA or higher graphics hardware\n");
		return;
	}

	// main variables values
	d=4; // switch variable
	key=2; // default screensaver number
	xpos=TILEWH*2;
	ypos=TILEWH*2;
	xdir=1;
	ydir=1;

	VGAmodeX(vgamodex_mode, 1, &gvar); // TODO: Suggestion: Instead of magic numbers for the first param, might I suggest defining an enum or some #define constants that are easier to remember? --J.C.
		// this code is written around modex16 which so far is a better fit than using DOSLIB vga directly, so leave MXLIB code in.
		// we'll integrate DOSLIB vga into that part of the code instead for less disruption. -- J.C.

	bakapee.xx = rand()&0%gvar.video.page[0].width;
	bakapee.yy = rand()&0%gvar.video.page[0].height;
	bakapee.gq = 0;
	bakapee.sx=0;
	bakapee.sy=0;
	bakapee.bakax=0;
	bakapee.bakay=0;
	bakapee.coor=0;
	bakapee.tile=0;

	/* setup camera and screen~ */
	gvar.video.page[0] = modexDefaultPage(&gvar.video.page[0]);
	gvar.video.page[0].width += (TILEWH*2);
	gvar.video.page[0].height += (TILEWH*2);
	textInit();

	//modexPalUpdate(bmp.palette); //____
	//modexDrawBmp(VGA, 0, 0, &bmp, 0); //____
	//getch(); //____

	modexShowPage(&gvar.video.page[0]);

// screen savers
#ifdef BOINK
	while(d>0)	// on!
	{
		/* run screensaver routine until keyboard input */
		while (key > 0) {
			if (kbhit()) {
				getch(); // eat keyboard input
				break;
			}

			ding(&gvar.video.page[0], &bakapee, key);
		}

		{
			int c;

		// this code is written around modex16 which so far is a better fit than using DOSLIB vga directly, so leave MXLIB code in.
		// we'll integrate DOSLIB vga into that part of the code instead for less disruption. -- J.C.
			VGAmodeX(0, 0, &gvar);
			// user imput switch
			//fprintf(stderr, "xx=%d	yy=%d	tile=%d\n", bakapee.xx, bakapee.yy, bakapee.tile);
			printf("Tiled mode is ");
			switch (bakapee.tile)
			{
				case 0:
					printf("off.\n");
				break;
				case 1:
					printf("on.\n");
				break;
			}
			printf("Enter 1, 2, 3, 4, 5, 6, 8, or 9 to run a screensaver, or enter 0 to quit.\n");

			c = getch();
			switch (c) {
				case 27: /* Escape key */
				case '0':
					d=0;
					break;
				case 'b': // test tile change
					switch (bakapee.tile)
					{
						case 0:
							bakapee.tile=1;
						break;
						case 1:
							bakapee.tile=0;
						break;
					}
					key=0;
					break;
				case '8':
					c+=8;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '9':
					key = c - '0';
					gvar.video.page[0] = modexDefaultPage(&gvar.video.page[0]);
					gvar.video.page[0].width += (TILEWH*2);
					gvar.video.page[0].height += (TILEWH*2);
					VGAmodeX(vgamodex_mode, 0, &gvar);
		// this code is written around modex16 which so far is a better fit than using DOSLIB vga directly, so leave MXLIB code in.
		// we'll integrate DOSLIB vga into that part of the code instead for less disruption. -- J.C.
					modexShowPage(&gvar.video.page[0]);
					break;
				default:
					key=0;
					break;
			}
		}
	}
#else // !defined(BOINK)
word panq=1, pand=0;
boolean panswitch=0;
// FIXME: Does not compile. Do you want to remove this?
// TODO: This is a testing sextion for textrendering and panning for project 16 --sparky4
	while(1)
	{ // conditions of screen saver
// 		while(!kbhit())
// 		{
// 			ding(&gvar.video.page[0], &bakapee, key);
// 		}
		//end of screen savers
		//pdump(&gvar.video.page[0]);

// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-48, "========================================");
// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-40, "|    |Chikyuu:$line1");
// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-32, "|    |$line2");
// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-24, "|    |$line3");
// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-16, "|    |$line4");
// 		mxOutText(xpos+1, ypos+gvar.video.page[0].height-8,  "========================================");

	ding(&gvar.video.page[0], &bakapee, key);
	modexPanPage(&gvar.video.page[0], xpos, ypos);
		//right movement
	if((IN_KeyDown(77) && !IN_KeyDown(75) && pand == 0) || pand == 2)
	{
		if(pand == 0){ pand = 2; }
			if(panq<=(TILEWH/(4)))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dx++;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dx++;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dx++;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//left movement
	if((IN_KeyDown(75) && !IN_KeyDown(77) && pand == 0) || pand == 4)
	{
		if(pand == 0){ pand = 4; }
			if(panq<=(TILEWH/(4)))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dx--;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dx--;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dx--;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//down movement
	if((IN_KeyDown(72) && !IN_KeyDown(80) && pand == 0) || pand == 3)
	{
		if(pand == 0){ pand = 3; }
			if(panq<=(TILEWH/(4)))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dy--;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dy--;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dy--;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//up movement
	if((IN_KeyDown(80) && !IN_KeyDown(72) && pand == 0) || pand == 1)
	{
		if(pand == 0){ pand = 1; }
			if(panq<=(TILEWH/(4)))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dy++;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dy++;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dy++;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
// 	xpos+=xdir;
// 	ypos+=ydir;
// 	if( (xpos>(VW-gvar.video.page[0].width-1))  || (xpos<1)){xdir=-xdir;}
// 	if( (ypos>(BH-gvar.video.page[0].height-1)) || (ypos<1)){ydir=-ydir;}
	ch=getch();
	if(ch==0x71)break; // 'q'
	if(ch==0x1b)break; // 'ESC'
	}
	VGAmodeX(0, 1, &gvar);
#endif // defined(BOINK)
	printf("bakapi ver. 1.04.16.04\nis made by sparky4�i���ց��j feel free to use it ^^\nLicence: GPL v3\n");
	printf("compiled on 2016/04/04\n");
}
//pee!
