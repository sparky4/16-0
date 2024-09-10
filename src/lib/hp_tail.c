/* Project 16 Source Code~
 * Copyright (C) 2012-2024 sparky4 & pngwen & andrius4669 & joncampbell123 & yakui-lover
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
 *
 * ment for higher level stuff
 */
//===========================================================================

#include "src/lib/hp_tail.h"


byte	*updateptr;
unsigned	uwidthtable[UPDATEHIGH];
unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];

//===========================================================================

/*
==========================
=
= Startup16
=
= Load a few things right away
=
==========================
*/

void Startup16 (void)
{
//	VL_Started=0;
//++++	TL_VidInit();
//	mmstarted=0;
//	PMStarted=0;
	StartupCAMMPM();
#ifdef __WATCOMC__
#ifdef __DEBUG_InputMgr__
	if(!dbg_nointest)
#endif
	IN_Startup();
#endif
}

//===========================================================================

/*
==========================
=
= Shutdown16
=
= Shuts down all ID_?? managers
=
==========================
*/

void Shutdown16 (void)
{
#ifdef __WATCOMC__
#ifdef __DEBUG_InputMgr__
	if(!dbg_nointest)
#endif
	IN_Shutdown();
#endif
	ShutdownCAMMPM();
#ifdef __WATCOMC__
	if(VL_Started)
		VL_Shutdown ();//VGAmodeX(0, 1, gvar);
#endif
}

//===========================================================================

/*
==========================
=
= StartupCAMMPM
=
==========================
*/

void StartupCAMMPM (void)
{
/*
 	MM_Startup ();                  // so the signon screen can be freed

	SignonScreen ();

	VW_Startup ();
	IN_Startup ();
	PM_Startup ();
	PM_UnlockMainMem ();
	SD_Startup ();
	CA_Startup ();
	US_Startup ();
*/
	MM_Startup();
#ifdef __16_PM__
	PM_Startup();
//????	PM_CheckMainMem();
	PM_UnlockMainMem();
#endif
	CA_Startup();
}

//===========================================================================

/*
==========================
=
= ShutdownCAMMPM
=
==========================
*/

void ShutdownCAMMPM (void)
{
/*
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ()
*/
#ifdef __16_PM__
	PM_Shutdown();
#endif
	CA_Shutdown();
	MM_Shutdown();
}

//===========================================================================

/*
==================
=
= DebugMemory
=
==================
*/

void DebugMemory_(boolean q)
{
	/*VW_FixRefreshBuffer ();
	US_CenterWindow (16,7);

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total	 :");
	US_PrintUnsigned (mminfo.mainmem/1024);
	US_Print ("k\nFree	  :");
	US_PrintUnsigned (MM_UnusedMemory()/1024);
	US_Print ("k\nWith purge:");
	US_PrintUnsigned (MM_TotalFree()/1024);
	US_Print ("k\n");
	VW_UpdateScreen();*/
	if(q){
	printf("========================================\n");
	printf("		DebugMemory_\n");
	printf("========================================\n");}
	if(q) { printf("Memory Usage\n");
	printf("------------\n"); }else printf("	%c%c", 0xD3, 0xC4);
	printf("Total:	"); if(q) printf("	"); printf("%uk", mminfo.mainmem/1024);
	if(q) printf("\n"); else printf("	");
	printf("Free:	"); if(q) printf("	"); printf("%uk", MM_UnusedMemory()/1024);
	if(q) printf("\n"); else printf("	");
	printf("With purge:"); if(q) printf("	"); printf("%uk\n", MM_TotalFree()/1024);
	if(q) printf("------------\n");
#ifdef __WATCOMC__
	//IN_Ack ();
#endif
//	if(q) MM_ShowMemory ();
}

/*
===================
=
= TestSprites
=
===================
*/

#if 0
#define DISPWIDTH	110
#define	TEXTWIDTH   40
void TestSprites(void)
{
	int hx,hy,sprite,oldsprite,bottomy,topx,shift;
	spritetabletype far *spr;
	spritetype _seg	*block;
	unsigned	mem,scan;


	VW_FixRefreshBuffer ();
	US_CenterWindow (30,17);

	US_CPrint ("Sprite Test");
	US_CPrint ("-----------");

	hy=PrintY;
	hx=(PrintX+56)&(~7);
	topx = hx+TEXTWIDTH;

	US_Print ("Chunk:\nWidth:\nHeight:\nOrgx:\nOrgy:\nXl:\nYl:\nXh:\nYh:\n"
			  "Shifts:\nMem:\n");

	bottomy = PrintY;

	sprite = STARTSPRITES;
	shift = 0;

	do
	{
		if (sprite>=STARTTILE8)
			sprite = STARTTILE8-1;
		else if (sprite<STARTSPRITES)
			sprite = STARTSPRITES;

		spr = &spritetable[sprite-STARTSPRITES];
		block = (spritetype _seg *)grsegs[sprite];

		VL_Bar (hx,hy,TEXTWIDTH,bottomy-hy,WHITE);

		PrintX=hx;
		PrintY=hy;
		US_PrintUnsigned (sprite);US_Print ("\n");PrintX=hx;
		US_PrintUnsigned (spr->width);US_Print ("\n");PrintX=hx;
		US_PrintUnsigned (spr->height);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->orgx);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->orgy);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->xl);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->yl);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->xh);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->yh);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->shifts);US_Print ("\n");PrintX=hx;
		if (!block)
		{
			US_Print ("-----");
		}
		else
		{
			mem = block->sourceoffset[3]+5*block->planesize[3];
			mem = (mem+15)&(~15);		// round to paragraphs
			US_PrintUnsigned (mem);
		}

		oldsprite = sprite;
		do
		{
		//
		// draw the current shift, then wait for key
		//
			VL_Bar(topx,hy,DISPWIDTH,bottomy-hy,WHITE);
			if (block)
			{
				PrintX = topx;
				PrintY = hy;
				US_Print ("Shift:");
				US_PrintUnsigned (shift);
				US_Print ("\n");
				VWB_DrawSprite (topx+16+shift*2,PrintY,sprite);
			}

			VW_UpdateScreen();

			scan = IN_WaitForKey ();

			switch (scan)
			{
			case sc_UpArrow:
				sprite++;
				break;
			case sc_DownArrow:
				sprite--;
				break;
			case sc_LeftArrow:
				if (--shift == -1)
					shift = 3;
				break;
			case sc_RightArrow:
				if (++shift == 4)
					shift = 0;
				break;
			case sc_Escape:
				return;
			}

		} while (sprite == oldsprite);

  } while (1);


}

#endif

/*
==========================
=
= ClearMemory
=
==========================
*/

void ClearMemory (void)
{
#ifdef __16_PM__
	PM_UnlockMainMem();
#endif
	//sd
	MM_SortMem ();
}

//===========================================================================

////////////////////////////////////////////////////////////////////
//
// HANDLE INTRO SCREEN (SYSTEM CONFIG)
//
////////////////////////////////////////////////////////////////////
void MU_IntroScreen(global_game_variables_t *gvar)
{
#define MAINCOLOR	0x6c
#define EMSCOLOR	0x6c
#define XMSCOLOR	0x6c

#define FILLCOLOR	14

	long memory,emshere,xmshere;
	int i,num,ems[10]={100,200,300,400,500,600,700,800,900,1000},
		xms[10]={100,200,300,400,500,600,700,800,900,1000},
		main[10]={32,64,96,128,160,192,224,256,288,320};

	gvar->video.print.t=1;
	gvar->video.print.tlsw=1;
	gvar->video.print.bgcolor=8;
	gvar->video.print.color=5;

	//
	// DRAW MAIN MEMORY
	//
	num=32;
	gvar->video.print.x=49-32;

	memory=(1023l+mminfo.nearheap+mminfo.farheap)/1024l;
	for (i=0;i<10;i++)
		if (memory>=main[i])
		{
			gvar->video.print.y=163-8*i;
			sprintf(global_temp_status_text, "% 4u", num); VL_print(global_temp_status_text, 0, gvar);
			VL_Bar(49,163-8*i,6,5,MAINCOLOR-i,gvar);
			num+=32;
		}
	gvar->video.print.y=171;
	VL_print("MAIN", 0, gvar);


	//
	// DRAW EMS MEMORY
	//
	if (EMSPresent)
	{
		num=100;
		gvar->video.print.x=89-32;

		emshere=4l*EMSPagesAvail;
		for (i=0;i<10;i++)
			if (emshere>=ems[i])
			{
				gvar->video.print.y=163-8*i;
				sprintf(global_temp_status_text, "% 4u", num); VL_print(global_temp_status_text, 0, gvar);
				VL_Bar(89,163-8*i,6,5,EMSCOLOR-i,gvar);
				num+=100;
			}
		gvar->video.print.y=171;
		VL_print(" EMS", 0, gvar);
	}

	//
	// DRAW XMS MEMORY
	//
	if (XMSPresent)
	{
		num=100;
		gvar->video.print.x=129-32;

		xmshere=4l*XMSPagesAvail;
		for (i=0;i<10;i++)
			if (xmshere>=xms[i])
			{
				gvar->video.print.y=163-8*i;
				sprintf(global_temp_status_text, "% 4u", num); VL_print(global_temp_status_text, 0, gvar);
				VL_Bar(129,163-8*i,6,5,XMSCOLOR-i,gvar);
				num+=100;
			}
		gvar->video.print.y=171;
		VL_print(" XMS", 0, gvar);
	}

	//
	// FILL BOXES
	//
	if (gvar->in.MousePresent)
		VL_Bar(164,82,12,2,FILLCOLOR,gvar);

	if (gvar->in.JoysPresent[0] || gvar->in.JoysPresent[1])
		VL_Bar(164,105,12,2,FILLCOLOR,gvar);

//++++	if (gvar->sd.AdLibPresent)//SB && !SoundBlasterPresent)
//++++		VL_Bar(164,128,12,2,FILLCOLOR,gvar);

//SB	if (SoundBlasterPresent)
//SB		VL_Bar(164,151,12,2,FILLCOLOR,gvar);

//SS	if (SoundSourcePresent)
//SS		VL_Bar(164,174,12,2,FILLCOLOR,gvar);
	IN_Ack ();
}

//===========================================================================

/*
====================
=
= ReadConfig
=
====================
*/
#if 0
void ReadConfig(void)
{
	int					 file;
//	SDMode		  sd;
//	SMMode		  sm;
//	SDSMode		 sds;


	if ( (file = open(CONFIGNAME,O_BINARY | O_RDONLY)) != -1)
	{
	//
	// valid config file
	//
//		read(file,Scores,sizeof(HighScore) * MaxScores);

//		read(file,&sd,sizeof(sd));
//		read(file,&sm,sizeof(sm));
//		read(file,&sds,sizeof(sds));

		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));

		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));

		read(file,&viewsize,sizeof(viewsize));
		read(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);

		/*if (sd == sdm_AdLib && !AdLibPresent && !SoundBlasterPresent)
		{
			sd = sdm_PC;
			sd = smm_Off;
		}

		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) ||
			(sds == sds_SoundSource && !SoundSourcePresent))
			sds = sds_Off;*/

		if (!MousePresent)
			mouseenabled = false;
		if (!JoysPresent[joystickport])
			joystickenabled = false;

		MainMenu[6].active=1;
		MainItems.curpos=0;
	}
	else
	{
	//
	// no config file, so select by hardware
	//
/*		if (SoundBlasterPresent || AdLibPresent)
		{
			sd = sdm_AdLib;
			sm = smm_AdLib;
		}
		else
		{
			sd = sdm_PC;
			sm = smm_Off;
		}

		if (SoundBlasterPresent)
			sds = sds_SoundBlaster;
		else if (SoundSourcePresent)
			sds = sds_SoundSource;
		else
			sds = sds_Off;*/

		if (MousePresent)
			mouseenabled = true;

		joystickenabled = false;
		joystickport = 0;
		joystickprogressive = false;

		viewsize = 15;
		mouseadjustment=5;
	}

	SD_SetMusicMode (sm);
	SD_SetSoundMode (sd);
	SD_SetDigiDevice (sds);
}


/*
====================
=
= WriteConfig
=
====================
*/

void WriteConfig(void)
{
	int					 file;

	file = open(CONFIGNAME,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (file != -1)
	{
//		write(file,Scores,sizeof(HighScore) * MaxScores);

//		write(file,&SoundMode,sizeof(SoundMode));
//		write(file,&MusicMode,sizeof(MusicMode));
//		write(file,&DigiMode,sizeof(DigiMode));

		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joystickprogressive,sizeof(joystickprogressive));
		write(file,&joystickport,sizeof(joystickport));

		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));

//		write(file,&viewsize,sizeof(viewsize));
		write(file,&mouseadjustment,sizeof(mouseadjustment));

		close(file);
	}
}
#endif

//===========================================================================
