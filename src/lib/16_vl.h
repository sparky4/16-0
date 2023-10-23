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
// ID_VL.H
#ifndef	__16_VL__
#define	__16_VL__
#include "src/lib/16_head.h"

// wolf compatability

#define MS_Quit	Quit

void Quit (char *error);

//===========================================================================


#define SC_INDEX			0x3C4
#define SC_RESET			0
#define SC_CLOCK			1
#define SC_MAPMASK			2
#define SC_CHARMAP			3
#define SC_MEMMODE			4

#define CRTC_INDEX			0x3D4
#define CRTC_DATA			0x03d5
#define CRTC_H_TOTAL		0
#define CRTC_H_DISPEND		1
#define CRTC_H_BLANK		2
#define CRTC_H_ENDBLANK		3
#define CRTC_H_RETRACE		4
#define CRTC_H_ENDRETRACE 	5
#define CRTC_V_TOTAL		6
#define CRTC_OVERFLOW		7
#define CRTC_ROWSCAN		8
#define CRTC_MAXSCANLINE 	9
#define CRTC_CURSORSTART 	10
#define CRTC_CURSOREND		11
#define CRTC_STARTHIGH		12
#define CRTC_STARTLOW		13
#define CRTC_CURSORHIGH		14
#define CRTC_CURSORLOW		15
#define CRTC_V_RETRACE		16
#define CRTC_V_ENDRETRACE 	17
#define CRTC_V_DISPEND		18
#define CRTC_OFFSET			19
#define CRTC_UNDERLINE		20
#define CRTC_V_BLANK		21
#define CRTC_V_ENDBLANK		22
#define CRTC_MODE			23
#define CRTC_LINECOMPARE 	24


#define GC_INDEX			0x3CE
#define GC_SETRESET			0
#define GC_ENABLESETRESET 	1
#define GC_COLORCOMPARE		2
#define GC_DATAROTATE		3
#define GC_READMAP			4
#define GC_MODE				5
#define GC_MISCELLANEOUS 	6
#define GC_COLORDONTCARE 	7
#define GC_BITMASK			8

#define ATR_INDEX			0x3c0
#define ATR_MODE			16
#define ATR_OVERSCAN		17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN			19
#define ATR_COLORSELECT		20

#define	STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR		0x3c8
#define PEL_READ_ADR		0x3c7
#define PEL_DATA			0x3c9

#define AC_INDEX		0x03c0
#define SC_DATA			0x03c5
#define CRTC_DATA		0x03d5
#define MISC_OUTPUT		0x03c2
#define HIGH_ADDRESS 		0x0C
#define LOW_ADDRESS		0x0D
#define VRETRACE		0x08
#define DISPLAY_ENABLE		0x01

//===========================================================================

#define SCREENSEG		0xa000

#define SCREENWIDTH		88			// default screen width in bytes
#define MAXSCANLINES	272			// size of ylookup table	240+32

#define CHARWIDTH		2
#define TILEWIDTH		4
#define TILEWH			16
#define TILEWHD			TILEWIDTH*8

typedef enum {CGAgr,EGAgr,VGAgr} grtype;

#define PAGE_OFFSET(x,y) (((y)<<6)+((y)<<4)+((x)>>2))
#define PLANE(x) (1 << ((x) & 3))
#define SELECT_ALL_PLANES() outpw(0x03c4, 0xff02)

// clips for rectangles not on 4s
#define LRCLIPDEF \
	static byte lclip[4] = {0x0f, 0x0e, 0x0c, 0x08}; \
	static byte rclip[4] = {0x00, 0x01, 0x03, 0x07};

#define VCLIPDEF \
	static byte pclip[4] = {1,2,4,8};

//===========================================================================

extern	grtype		grmode;			// CGAgr, EGAgr, VGAgr

extern	unsigned	bufferofs;			// all drawing is reletive to this
extern	unsigned	displayofs,pelpan;	// last setscreen coordinates
extern	unsigned	panx,pany;		// panning adjustments inside port in pixels
extern	unsigned	pansx,pansy;
extern	unsigned	panadjust;		// panx/pany adjusted by screen resolution

extern	unsigned	screenseg;			// set to 0xa000 for asm convenience

extern	unsigned	linewidth;
extern	unsigned	ylookup[MAXSCANLINES];

extern	boolean		screenfaded;
extern	unsigned	bordercolor;
extern	longword	TimeCount;

typedef struct{
	word tw;				/* screen width in tiles */
	word th;				/* screen height in tiles */
	word tilesw;				/* virtual screen width in tiles */
	word tilesh;				/* virtual screen height in tiles */
	sword tilemidposscreenx;	/* middle tile x position */	/* needed for scroll system to work accordingly */
	sword tilemidposscreeny;	/* middle tile y position */	/* needed for scroll system to work accordingly */
	sword tileplayerposscreenx;	/* player position on screen */	/* needed for scroll and map system to work accordingly */
	sword tileplayerposscreeny;	/* player position on screen */	/* needed for scroll and map system to work accordingly */
} pagetileinfo_t;

typedef struct {
	nibble/*word*/ id;	/* the Identification number of the page~ For layering~ */
	byte far* data;	/* the data for the page */
	pagetileinfo_t ti;	// the tile information of the page
	word dx;		/* col we are viewing on virtual screen (on page[0]) */	/* off screen buffer on the left size */
	word dy;		/* row we are viewing on virtual screen (on page[0]) */	/* off screen buffer on the top size */
	word sw;		/* screen width */	/* resolution */
	word sh;		/* screen heigth */	/* resolution */
	word width;		/* virtual width of the page */
	word height;	/* virtual height of the page */
	word stridew;	/* width/4 */	/* VGA */
	word pagesize;	/* page size */
	word pi;		/* increment page by this much to preserve location */
	int tlx,tly;
//newer vars
//TODO: find where they are used
	sword delta;			// How much should we shift the page for smooth scrolling
} page_t;


extern	page_t		page[4];

//===========================================================================

//
// VGA hardware routines
//

/*#define VGAWRITEMODE(x) asm{\
cli;\
mov dx,GC_INDEX;\
mov al,GC_MODE;\
out dx,al;\
inc dx;\
in al,dx;\
and al,252;\
or al,x;\
out dx,al;\
sti;}

#define VGAMAPMASK(x) asm{cli;mov dx,SC_INDEX;mov al,SC_MAPMASK;mov ah,x;out dx,ax;sti;}
#define VGAREADMAP(x) asm{cli;mov dx,GC_INDEX;mov al,GC_READMAP;mov ah,x;out dx,ax;sti;}*/
void VGAWRITEMODE(byte x);
void VGAMAPMASK(byte x);
void VGAREADMAP(byte x);
void VGABITMASK(byte x);


void VL_Startup (void);
void VL_Shutdown (void);

void VW_SetScreenMode (int grmode);
void VL_SetVGAPlane (void);
void VL_SetTextMode (void);
void VL_DePlaneVGA (void);
void VL_SetVGAPlaneMode (void);
void VL_ClearVideo (byte color);

void VL_SetLineWidth (unsigned width);
void VL_SetSplitScreen (int linenum);

void VL_WaitVBL (word num);
void VL_CrtcStart (int crtc);
void VL_SetScreen (word crtc, word pelpan);

void VL_FillPalette (int red, int green, int blue);
void VL_SetColor	(int color, int red, int green, int blue);
void VL_GetColor	(int color, int *red, int *green, int *blue);
void VL_SetPalette (byte far *palette);
void VL_GetPalette (byte far *palette);
void VL_FadeOut (int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn (int start, int end, byte far *palette, int steps);
void VL_ColorBorder (int color);

void VL_Plot (int x, int y, int color);
void VL_Hlin (unsigned x, unsigned y, unsigned width, unsigned color);
void VL_Vlin (int x, int y, int height, int color);
void VL_Bar (int x, int y, int width, int height, int color);

void VL_MungePic (byte far *source, unsigned width, unsigned height);
void VL_DrawPicBare (int x, int y, byte far *pic, int width, int height);
void VL_MemToLatch (byte far *source, int width, int height, unsigned dest);
void VL_ScreenToScreen (word source, word dest,word width, word height);
void VL_MemToScreen (byte far *source, int width, int height, int x, int y);
void VL_MaskedToScreen (byte far *source, int width, int height, int x, int y);

void VL_DrawTile8String (char *str, char far *tile8ptr, int printx, int printy);
void VL_DrawLatch8String (char *str, unsigned tile8ptr, int printx, int printy);
void VL_SizeTile8String (char *str, int *width, int *height);
void VL_DrawPropString (char *str, unsigned tile8ptr, int printx, int printy);
void VL_SizePropString (char *str, int *width, int *height, char far *font);

void VL_TestPaletteSet (void);
void VL_LatchToScreen (unsigned source, int width, int height, int x, int y);

void	VW_SetScreen (word crtc, word pel);

//page prototyes
void VL_ShowPage(page_t *page, boolean vsync, boolean sr);
page_t VL_InitPage(void);
page_t	VL_NextPage(page_t *p);
page_t	VL_NextPageFlexibleSize(page_t *p, word x, word y);
#endif
