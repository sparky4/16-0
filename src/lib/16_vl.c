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
// ID_VL.C

#include <dos.h>
#include <malloc.h>
#include <mem.h>
#include <string.h>
#include "src/lib/16_vl.h"
#pragma hdrstop

//
// SC_INDEX is expected to stay at SC_MAPMASK for proper operation
//

grtype		grmode;			// CGAgr, EGAgr, VGAgr

unsigned	bufferofs;
unsigned	displayofs,pelpan;
unsigned	panx,pany;		// panning adjustments inside port in pixels
unsigned	pansx,pansy;	// panning adjustments inside port in screen
							// block limited pixel values (ie 0/8 for ega x)
unsigned	panadjust;		// panx/pany adjusted by screen resolution

unsigned	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

unsigned	linewidth;
unsigned	ylookup[MAXSCANLINES];

boolean		screenfaded;
unsigned	bordercolor;

boolean		fastpalette;				// if true, use outsb to set

byte		far	palette1[256][3],far palette2[256][3];

unsigned	timecount;

//===========================================================================

// asm

int	 VL_VideoID (void);
//void VL_SetCRTC (word crtc);
//void VL_SetScreen (word crtc, word pelpan);
//void VL_WaitVBL (word num);

//===========================================================================


/*
=======================
=
= VL_Startup
=
=======================
*/

#if 0
void	VL_Startup (void)
{
	if ( !MS_CheckParm ("HIDDENCARD") && VL_VideoID () != 5)
		MS_Quit ("You need a VGA graphics card to run this!");

	asm	cld;				// all string instructions assume forward
}

#endif

/*
=======================
=
= VL_Startup	// WOLFENSTEIN HACK
=
=======================
*/

static	char *ParmStrings[] = {"HIDDENCARD",""};

void	VL_Startup (void)
{
	int i,videocard;

	asm	cld;

	videocard = VL_VideoID ();
	for (i = 1;i < _argc;i++)
		if (US_CheckParm(_argv[i],ParmStrings) == 0)
		{
			videocard = 5;
			break;
		}

	if (videocard != 5)
Quit ("Improper video card!  If you really have a VGA card that I am not \n"
	  "detecting, use the -HIDDENCARD command line parameter!");

}



/*
=======================
=
= VL_Shutdown
=
=======================
*/

void	VL_Shutdown (void)
{
	VL_SetTextMode ();
}


/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void	VL_SetVGAPlaneMode (void)
{
asm	mov	ax,0x13
asm	int	0x10
	VL_DePlaneVGA ();
	VGAMAPMASK(15);
	VL_SetLineWidth (44);

}


/*
=======================
=
= VL_SetTextMode
=
=======================
*/

void	VL_SetTextMode (void)
{
asm	mov	ax,3
asm	int	0x10
}

//===========================================================================

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo (byte color)
{
asm	mov	dx,GC_INDEX
asm	mov	al,GC_MODE
asm	out	dx,al
asm	inc	dx
asm	in	al,dx
asm	and	al,0xfc				// write mode 0 to store directly to video
asm	out	dx,al

asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax				// write through all four planes

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,[color]
asm	mov	ah,al
asm	mov	cx,0x8000			// 0x8000 words, clearing 8 video bytes/word
asm	xor	di,di
asm	rep	stosw
}


/*
=============================================================================

			VGA REGISTER MANAGEMENT ROUTINES

=============================================================================
*/


/*
=================
=
= VL_DePlaneVGA
=
=================
*/

void VL_DePlaneVGA (void)
{
#if 0
//
// change CPU addressing to non linear mode
//

//
// turn off chain 4 and odd/even
//
	outportb (SC_INDEX,SC_MEMMODE);
	outportb (SC_INDEX+1,(inportb(SC_INDEX+1)&~8)|4);

	outportb (SC_INDEX,SC_MAPMASK);		// leave this set throughought

//
// turn off odd/even and set write mode 0
//
	outportb (GC_INDEX,GC_MODE);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~0x13);

//
// turn off chain
//
	outportb (GC_INDEX,GC_MISCELLANEOUS);
	outportb (GC_INDEX+1,inportb(GC_INDEX+1)&~2);

//
// clear the entire buffer space, because int 10h only did 16 k / plane
//
	VL_ClearVideo (0);

//
// change CRTC scanning from doubleword to byte mode, allowing >64k scans
//
	outportb (CRTC_INDEX,CRTC_UNDERLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)&~0x40);

	outportb (CRTC_INDEX,CRTC_MODE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1)|0x40);

//
// added lines for Project 16
//
/*	vga_write_sequencer(4,0x06);
	vga_write_sequencer(0,0x01);
	vga_write_CRTC(0x17,0xE3);
	vga_write_CRTC(0x14,0x00);
	vga_write_sequencer(0,0x03);
	vga_write_sequencer(VGA_SC_MAP_MASK,0xF);*/
#endif
//
// enter mode X
//
word i;
//dword far*ptr=(dword far*)SCREENSEG;      /* used for faster screen clearing */
word CRTParms[] = {
	0x0d06,		/* vertical total */
	0x3e07,		/* overflow (bit 8 of vertical counts) */
	0x4109,		/* cell height (2 to double-scan */
	0xea10,		/* v sync start */
	0xac11,		/* v sync end and protect cr0-cr7 */
	0xdf12,		/* vertical displayed */
	0x2813,		/* offset for virtual resolution */
	0x0014,		/* turn off dword mode */
	0xe715,		/* v blank start */
	0x0616,		/* v blank end */
	0xe317		/* turn on byte mode */

};
int CRTParmCount = sizeof(CRTParms) / sizeof(CRTParms[0]);

/* disable chain4 mode */
outport(SC_INDEX, 0x0604);

/* synchronous reset while setting Misc Output */
outport(SC_INDEX, 0x0100);

/* select 25 MHz dot clock & 60 Hz scanning rate */
outportb(MISC_OUTPUT, 0xe3);

/* undo reset (restart sequencer) */
outport(SC_INDEX, 0x0300);

/* reprogram the CRT controller */
outport(CRTC_INDEX, 0x11); /* VSync End reg contains register write prot */
outport(CRTC_DATA, 0x7f);  /* get current write protect on varios regs */

/* send the CRTParms */
for(i=0; i<CRTParmCount; i++) {
	outport(CRTC_INDEX, CRTParms[i]);
}

}

//===========================================================================

/*
 * ====================
 * =
 * = VL_SetLineWidth
 * =
 * = Line witdh is in WORDS, 40 words is normal width for vgaplanegr
 * =
 * ====================
 */

void VL_SetLineWidth (unsigned width)
{
	int i,offset;

	//
	// set wide virtual screen
	//
	outport (CRTC_INDEX,CRTC_OFFSET+width*256);

	//
	// set up lookup tables
	//
	linewidth = width*2;

	offset = 0;

	for (i=0;i<MAXSCANLINES;i++)
	{
		ylookup[i]=offset;
		offset += linewidth;
	}
}

/*
 * ====================
 * =
 * = VL_SetSplitScreen
 * =
 * ====================
 */

void VL_SetSplitScreen (int linenum)
{
	VL_WaitVBL (1);
	linenum=linenum*2-1;
	outportb (CRTC_INDEX,CRTC_LINECOMPARE);
	outportb (CRTC_INDEX+1,linenum % 256);
	outportb (CRTC_INDEX,CRTC_OVERFLOW);
	outportb (CRTC_INDEX+1, 1+16*(linenum/256));
	outportb (CRTC_INDEX,CRTC_MAXSCANLINE);
	outportb (CRTC_INDEX+1,inportb(CRTC_INDEX+1) & (255-64));
}


/*
 * =============================================================================
 *
 *						PALETTE OPS
 *
 *		To avoid snow, do a WaitVBL BEFORE calling these
 *
 * =============================================================================
 */


/*
 * =================
 * =
 * = VL_FillPalette
 * =
 * =================
 */

void VL_FillPalette (int red, int green, int blue)
{
	int	i;

	outportb (PEL_WRITE_ADR,0);
	for (i=0;i<256;i++)
	{
		outportb (PEL_DATA,red);
		outportb (PEL_DATA,green);
		outportb (PEL_DATA,blue);
	}
}

//===========================================================================

/*
 * =================
 * =
 * = VL_SetColor
 * =
 * =================
 */

void VL_SetColor	(int color, int red, int green, int blue)
{
	outportb (PEL_WRITE_ADR,color);
	outportb (PEL_DATA,red);
	outportb (PEL_DATA,green);
	outportb (PEL_DATA,blue);
}

//===========================================================================

/*
 * =================
 * =
 * = VL_GetColor
 * =
 * =================
 */

void VL_GetColor	(int color, int *red, int *green, int *blue)
{
	outportb (PEL_READ_ADR,color);
	*red = inportb (PEL_DATA);
	*green = inportb (PEL_DATA);
	*blue = inportb (PEL_DATA);
}

//===========================================================================

/*
 * =================
 * =
 * = VL_SetPalette
 * =
 * = If fast palette setting has been tested for, it is used
 * = (some cards don't like outsb palette setting)
 * =
 * =================
 */

void VL_SetPalette (byte far *palette)
{
	int	i;

	//	outportb (PEL_WRITE_ADR,0);
	//	for (i=0;i<768;i++)
	//		outportb(PEL_DATA,*palette++);

	__asm {
		mov	dx,PEL_WRITE_ADR
		mov	al,0
		out	dx,al
		mov	dx,PEL_DATA
		lds	si,[palette]

		test	[ss:fastpalette],1
	//asm	jz	slowset
	//
	// set palette fast for cards that can take it
	//
	//asm	mov	cx,768
	//asm	rep outsb
	//asm	jmp	done

	//
	// set palette slowly for some video cards
	//
#ifdef __BORLANDC__
	}
#endif
	slowset:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	cx,256
#ifdef __BORLANDC__
	}
#endif
	setloop:
#ifdef __BORLANDC__
	__asm {
#endif
		lodsb
		out	dx,al
		lodsb
		out	dx,al
		lodsb
		out	dx,al
		loop	setloop

#ifdef __BORLANDC__
	}
#endif
	done:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	ax,ss
		mov	ds,ax
	}

}


//===========================================================================

/*
 * =================
 * =
 * = VL_GetPalette
 * =
 * = This does not use the port string instructions,
 * = due to some incompatabilities
 * =
 * =================
 */

void VL_GetPalette (byte far *palette)
{
	int	i;

	outportb (PEL_READ_ADR,0);
	for (i=0;i<768;i++)
		*palette++ = inportb(PEL_DATA);
}


//===========================================================================

/*
 * =================
 * =
 * = VL_FadeOut
 * =
 * = Fades the current palette to the given color in the given number of steps
 * =
 * =================
 */

void VL_FadeOut (int start, int end, int red, int green, int blue, int steps)
{
	int		i,j,orig,delta;
	byte	far *origptr, far *newptr;

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	_fmemcpy (palette2,palette1,768);

	//
	// fade through intermediate frames
	//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start][0];
		newptr = &palette2[start][0];
		for (j=start;j<=end;j++)
		{
			orig = *origptr++;
			delta = red-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = green-orig;
			*newptr++ = orig + delta * i / steps;
			orig = *origptr++;
			delta = blue-orig;
			*newptr++ = orig + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette (&palette2[0][0]);
	}

	//
	// final color
	//
	VL_FillPalette (red,green,blue);

	screenfaded = true;
}


/*
 * =================
 * =
 * = VL_FadeIn
 * =
 * =================
 */

void VL_FadeIn (int start, int end, byte far *palette, int steps)
{
	int		i,j,delta;

	VL_WaitVBL(1);
	VL_GetPalette (&palette1[0][0]);
	_fmemcpy (&palette2[0][0],&palette1[0][0],sizeof(palette1));

	start *= 3;
	end = end*3+2;

	//
	// fade through intermediate frames
	//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j]-palette1[0][j];
			palette2[0][j] = palette1[0][j] + delta * i / steps;
		}

		VL_WaitVBL(1);
		VL_SetPalette (&palette2[0][0]);
	}

	//
	// final color
	//
	VL_SetPalette (palette);
	screenfaded = false;
}



/*
 * =================
 * =
 * = VL_TestPaletteSet
 * =
 * = Sets the palette with outsb, then reads it in and compares
 * = If it compares ok, fastpalette is set to true.
 * =
 * =================
 */

void VL_TestPaletteSet (void)
{
	int	i;

	for (i=0;i<768;i++)
		palette1[0][i] = i;

	fastpalette = true;
	VL_SetPalette (&palette1[0][0]);
	VL_GetPalette (&palette2[0][0]);
	if (_fmemcmp (&palette1[0][0],&palette2[0][0],768))
		fastpalette = false;
}


/*
 * ==================
 * =
 * = VL_ColorBorder
 * =
 * ==================
 */

void VL_ColorBorder (int color)
{
	_AH=0x10;
	_AL=1;
	_BH=color;
	geninterrupt (0x10);
	bordercolor = color;
}



/*
 * =============================================================================
 *
 *							PIXEL OPS
 *
 * =============================================================================
 */

byte	pixmasks[4] = {1,2,4,8};
byte	leftmasks[4] = {15,14,12,8};
byte	rightmasks[4] = {1,3,7,15};


/*
 * =================
 * =
 * = VL_Plot
 * =
 * =================
 */

void VL_Plot (int x, int y, int color)
{
	byte mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);
	*(byte far *)MK_FP(SCREENSEG,bufferofs+(ylookup[y]+(x>>2))) = color;
	VGAMAPMASK(15);
}


/*
 * =================
 * =
 * = VL_Hlin
 * =
 * =================
 */

void VL_Hlin (unsigned x, unsigned y, unsigned width, unsigned color)
{
	unsigned		xbyte;
	byte			far *dest;
	byte			leftmask,rightmask;
	int				midbytes;

	xbyte = x>>2;
	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	midbytes = ((x+width+3)>>2) - xbyte - 2;

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+xbyte);

	if (midbytes<0)
	{
		// all in one byte
		VGAMAPMASK(leftmask&rightmask);
		*dest = color;
		VGAMAPMASK(15);
		return;
	}

	VGAMAPMASK(leftmask);
	*dest++ = color;

	VGAMAPMASK(15);
	_fmemset (dest,color,midbytes);
	dest+=midbytes;

	VGAMAPMASK(rightmask);
	*dest = color;

	VGAMAPMASK(15);
}


/*
 * =================
 * =
 * = VL_Vlin
 * =
 * =================
 */

void VL_Vlin (int x, int y, int height, int color)
{
	byte	far *dest,mask;

	mask = pixmasks[x&3];
	VGAMAPMASK(mask);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	while (height--)
	{
		*dest = color;
		dest += linewidth;
	}

	VGAMAPMASK(15);
}


/*
 * =================
 * =
 * = VL_Bar
 * =
 * =================
 */

void VL_Bar (int x, int y, int width, int height, int color)
{
	byte	far *dest;
	byte	leftmask,rightmask;
	int		midbytes,linedelta;

	leftmask = leftmasks[x&3];
	rightmask = rightmasks[(x+width-1)&3];
	midbytes = ((x+width+3)>>2) - (x>>2) - 2;
	linedelta = linewidth-(midbytes+1);

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2));

	if (midbytes<0)
	{
		// all in one byte
		VGAMAPMASK(leftmask&rightmask);
		while (height--)
		{
			*dest = color;
			dest += linewidth;
		}
		VGAMAPMASK(15);
		return;
	}

	while (height--)
	{
		VGAMAPMASK(leftmask);
		*dest++ = color;

		VGAMAPMASK(15);
		_fmemset (dest,color,midbytes);
		dest+=midbytes;

		VGAMAPMASK(rightmask);
		*dest = color;

		dest+=linedelta;
	}

	VGAMAPMASK(15);
}

/*
 * ============================================================================
 *
 *							MEMORY OPS
 *
 * ============================================================================
 */

/*
 * =================
 * =
 * = VL_MemToLatch
 * =
 * =================
 */

void VL_MemToLatch (byte far *source, int width, int height, unsigned dest)
{
	unsigned	count;
	byte	plane,mask;

	count = ((width+3)/4)*height;
	mask = 1;
	for (plane = 0; plane<4 ; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;

		asm	mov	cx,count
		asm mov ax,SCREENSEG
		asm mov es,ax
		asm	mov	di,[dest]
		asm	lds	si,[source]
		asm	rep movsb
		asm mov	ax,ss
		asm	mov	ds,ax

		source+= count;
	}
}


//===========================================================================


/*
 * =================
 * =
 * = VL_MemToScreen
 * =
 * = Draws a block of data to the screen.
 * =
 * =================
 */

void VL_MemToScreen (byte far *source, int width, int height, int x, int y)
{
	byte    far *screen,far *dest,mask;
	int		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	mask = 1 << (x&3);

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
}

//==========================================================================


/*
 * =================
 * =
 * = VL_MaskedToScreen
 * =
 * = Masks a block of main memory to the screen.
 * =
 * =================
 */

void VL_MaskedToScreen (byte far *source, int width, int height, int x, int y)
{
	byte    far *screen,far *dest,mask;
	byte	far *maskptr;
	int		plane;

	width>>=2;
	dest = MK_FP(SCREENSEG,bufferofs+ylookup[y]+(x>>2) );
	//	mask = 1 << (x&3);

	//	maskptr = source;

	for (plane = 0; plane<4; plane++)
	{
		VGAMAPMASK(mask);
		mask <<= 1;
		if (mask == 16)
			mask = 1;

		screen = dest;
		for (y=0;y<height;y++,screen+=linewidth,source+=width)
			_fmemcpy (screen,source,width);
	}
}

//==========================================================================

/*
 * =================
 * =
 * = VL_LatchToScreen
 * =
 * =================
 */

void VL_LatchToScreen (unsigned source, int width, int height, int x, int y)
{
	VGAWRITEMODE(1);
	VGAMAPMASK(15);

	__asm {
		mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
		shl	di,1
		mov	di,[WORD PTR ylookup+di]
		add	di,[bufferofs]
		mov	ax,[x]
		shr	ax,1
		shr	ax,1
		add	di,ax

		mov	si,[source]
		mov	ax,[width]
		mov	bx,[linewidth]
		sub	bx,ax
		mov	dx,[height]
		mov	cx,SCREENSEG
		mov	ds,cx
		mov	es,cx

#ifdef __BORLANDC__
	}
#endif
	drawline:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	cx,ax
		rep movsb
		add	di,bx
		dec	dx
		jnz	drawline

		mov	ax,ss
		mov	ds,ax
}

	VGAWRITEMODE(0);
}


//===========================================================================

#if 0

/*
 * =================
 * =
 * = VL_ScreenToScreen
 * =
 * =================
 */

void VL_ScreenToScreen (unsigned source, unsigned dest,int width, int height)
{
VGAWRITEMODE(1);
VGAMAPMASK(15);

asm	mov	si,[source]
asm	mov	di,[dest]
asm	mov	ax,[width]
asm	mov	bx,[linewidth]
asm	sub	bx,ax
asm	mov	dx,[height]
asm	mov	cx,SCREENSEG
asm	mov	ds,cx
asm	mov	es,cx

drawline:
asm	mov	cx,ax
asm	rep movsb
asm	add	si,bx
asm	add	di,bx
asm	dec	dx
asm	jnz	drawline

asm	mov	ax,ss
asm	mov	ds,ax

VGAWRITEMODE(0);
}


#endif

/*
 * =============================================================================
 *
 *						STRING OUTPUT ROUTINES
 *
 * =============================================================================
 */




/*
 * ===================
 * =
 * = VL_DrawTile8String
 * =
 * ===================
 */

void VL_DrawTile8String (char *str, char far *tile8ptr, int printx, int printy)
{
	int		i;
	unsigned	far *dest,far *screen,far *src;

	dest = MK_FP(SCREENSEG,bufferofs+ylookup[printy]+(printx>>2));

	while (*str)
	{
		src = (unsigned far *)(tile8ptr + (*str<<6));
		// each character is 64 bytes

		VGAMAPMASK(1);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(2);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(4);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;
		VGAMAPMASK(8);
		screen = dest;
		for (i=0;i<8;i++,screen+=linewidth)
			*screen = *src++;

		str++;
		printx += 8;
		dest+=2;
	}
}



/*
 * ===================
 * =
 * = VL_DrawLatch8String
 * =
 * ===================
 */

void VL_DrawLatch8String (char *str, unsigned tile8ptr, int printx, int printy)
{
	int		i;
	unsigned	src,dest;

	dest = bufferofs+ylookup[printy]+(printx>>2);

	VGAWRITEMODE(1);
	VGAMAPMASK(15);

	while (*str)
	{
		src = tile8ptr + (*str<<4);		// each character is 16 latch bytes

		asm	mov	si,[src]
		asm	mov	di,[dest]
		asm	mov	dx,[linewidth]

		asm	mov	ax,SCREENSEG
		asm	mov	ds,ax

		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx
		asm	lodsw
		asm	mov	[di],ax
		asm	add	di,dx

		asm	mov	ax,ss
		asm	mov	ds,ax

		str++;
		printx += 8;
		dest+=2;
	}

	VGAWRITEMODE(0);
}


/*
 * ===================
 * =
 * = VL_SizeTile8String
 * =
 * ===================
 */

void VL_SizeTile8String (char *str, int *width, int *height)
{
	*height = 8;
	*width = 8*strlen(str);
}

//assembly routines
#if 0
#define VGAWRITEMODE(x) asm{
cli
mov	dx,GC_INDEX
mov	al,GC_MODE
out	dx,al
inc	dx
in	al,dx
and	al,252
or	al,x
out	dx,al
sti
}

#define VGAMAPMASK(x) asm{
cli
mov	dx,SC_INDEX
mov	al,SC_MAPMASK
mov	ah,x
out	dx,ax
sti
}

#define VGAREADMAP(x) asm{
cli
mov	dx,GC_INDEX
mov	al,GC_READMAP
mov	ah,x
out	dx,ax
sti
}

#define EGABITMASK(x) asm{
mov	dx,GC_INDEX
mov	ax,GC_BITMASK+256*x
out	dx,ax
sti
}
#endif
void VGAWRITEMODE(byte x)
{
	__asm {
		cli
		mov	dx,GC_INDEX
		mov	al,GC_MODE
		out	dx,al
		inc	dx
		in	al,dx
		and	al,252
		or	al,x
		out	dx,al
		sti
	}
}

void VGAMAPMASK(byte x)
{
	__asm {
		cli
		mov	dx,SC_INDEX
		mov	al,SC_MAPMASK
		mov	ah,x
		out	dx,ax
		sti
	}
}

void VGAREADMAP(byte x)
{
	__asm {
		cli
		mov	dx,GC_INDEX
		mov	al,GC_READMAP
		mov	ah,x
		out	dx,ax
		sti
	}
}

void VGABITMASK(byte x)
{
	word q = 256*x;
	__asm {
		mov	dx,GC_INDEX
		mov	ax,GC_BITMASK+q
		out	dx,ax
		sti
	}
}






//===========================================================================


//==============
//
// VL_WaitVBL			******** NEW *********
//
// Wait for the vertical retrace (returns before the actual vertical sync)
//
//==============

void VL_WaitVBL (word num)
{
#ifdef __WATCOMC__
	__asm {
#endif
@@wait:
#ifdef __BORLANDC__
	__asm {
#endif
	mov	dx,STATUS_REGISTER_1

	mov	cx,[num]
//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
#ifdef __BORLANDC__
	}
#endif
@@waitnosync:
#ifdef __BORLANDC__
	__asm {
#endif
	in	al,dx
	test	al,8
	jnz	@@waitnosync
#ifdef __BORLANDC__
	}
#endif
@@waitsync:
#ifdef __BORLANDC__
	__asm {
#endif
	in	al,dx
	test	al,8
	jz	@@waitsync

	loop	@@waitnosync
	}
}


//===========================================================================

//==============
//
// VL_SetCRTC
//
//==============

void	VL_SetCRTC (word crtc)
{
	__asm {
//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
	cli

	mov	dx,STATUS_REGISTER_1
#ifdef __BORLANDC__
	}
#endif
@@waitdisplay:
#ifdef __BORLANDC__
	__asm {
#endif
	in	al,dx
	test	al,1	//1 = display is disabled (HBL / VBL)
	jnz	@@waitdisplay


//
// set CRTC start
//
// for some reason, my XT's EGA card doesn't like word outs to the CRTC
// index...
//
	mov	cx,[crtc]
	mov	dx,CRTC_INDEX
	mov	al,0ch		//start address high register
	out	dx,al
	inc	dx
	mov	al,ch
	out	dx,al
	dec	dx
	mov	al,0dh		//start address low register
	out	dx,al
	mov	al,cl
	inc	dx
	out	dx,al


	sti

}
}



//===========================================================================

//==============
//
// VL_SetScreen
//
//==============

//PROC	VL_SetScreen  crtc:WORD, pel:WORD
void	VL_SetScreen (word crtc, word pel)
{
	__asm {


	mov	cx,[timecount]		// if timecount goes up by two, the retrace
	add	cx,2				// period was missed (an interrupt covered it)

	mov	dx,STATUS_REGISTER_1

//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
#ifdef __BORLANDC__
	}
#endif
@@waitdisplay:
#ifdef __BORLANDC__
	__asm {
#endif
	in	al,dx
	test	al,1	//1 = display is disabled (HBL / VBL)
	jnz	@@waitdisplay

#ifdef __BORLANDC__
	}
#endif
@@loop:
#ifdef __BORLANDC__
	__asm {
#endif
	sti
	jmp	comparetimecount
	cli

#ifdef __BORLANDC__
	}
#endif
comparetimecount:
#ifdef __BORLANDC__
	__asm {
#endif
	cmp	[timecount],cx		// will only happen if an interrupt is
	jae	@@setcrtc			// straddling the entire retrace period

//
// when several succesive display not enableds occur,
// the bottom of the screen has been hit
//

	in	al,dx
	test	al,8
	jnz	@@waitdisplay
	test	al,1
	jz	@@loop

	in	al,dx
	test	al,8
	jnz	@@waitdisplay
	test	al,1
	jz	@@loop

	in	al,dx
	test	al,8
	jnz	@@waitdisplay
	test	al,1
	jz	@@loop

	in	al,dx
	test	al,8
	jnz	@@waitdisplay
	test	al,1
	jz	@@loop

	in	al,dx
	test	al,8
	jnz	@@waitdisplay
	test	al,1
	jz	@@loop

#ifdef __BORLANDC__
	}
#endif
@@setcrtc:
#ifdef __BORLANDC__
	__asm {
#endif

//
// set CRTC start
//
// for some reason, my XT's EGA card doesn't like word outs to the CRTC
// index...
//
	mov	cx,[crtc]
	mov	dx,CRTC_INDEX
	mov	al,0ch		//start address high register
	out	dx,al
	inc	dx
	mov	al,ch
	out	dx,al
	dec	dx
	mov	al,0dh		//start address low register
	out	dx,al
	mov	al,cl
	inc	dx
	out	dx,al

//
// set horizontal panning
//
	mov	dx,ATR_INDEX
	mov	al,ATR_PELPAN or 20h
	out	dx,al
	jmp	end
	mov	al,[BYTE ptr pel]		//pel pan value
	out	dx,al

#ifdef __BORLANDC__
	}
#endif
end:
#ifdef __BORLANDC__
	__asm {
#endif

	sti

	}
}


//===========================================================================


//============================================================================
//
// VL_ScreenToScreen
//
// Basic block copy routine.  Copies one block of screen memory to another,
// using write mode 1 (sets it and returns with write mode 0).  bufferofs is
// NOT accounted for.
//
//============================================================================

//PROC	VL_ScreenToScreen	source:WORD, dest:WORD, wide:WORD, height:WORD
void	VL_ScreenToScreen (word source, word dest, word wide, word height)
{
	__asm {
//USES	SI,DI

	pushf
	cli

	mov	dx,SC_INDEX
	mov	ax,SC_MAPMASK+15*256
	out	dx,ax
	mov	dx,GC_INDEX
	mov	al,GC_MODE
	out	dx,al
	inc	dx
	in	al,dx
	and	al,NOT 3
	or	al,1
	out	dx,al

	popf

	mov	bx,[linewidth]
	sub	bx,[wide]

	mov	ax,SCREENSEG
	mov	es,ax
	mov	ds,ax

	mov	si,[source]
	mov	di,[dest]				//start at same place in all planes
	mov	dx,[height]				//scan lines to draw
	mov	ax,[wide]

#ifdef __BORLANDC__
	}
#endif
@@lineloop:
#ifdef __BORLANDC__
	__asm {
#endif
	mov	cx,ax
	rep	movsb
	add	si,bx
	add	di,bx

	dec	dx
	jnz	@@lineloop

	mov	dx,GC_INDEX+1
	in	al,dx
	and	al,NOT 3
	out	dx,al

	mov	ax,ss
	mov	ds,ax					//restore turbo's data segment

	}
}

//===========================================================================


//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//
// Name:	VL_VideoID
//
// Function:	Detects the presence of various video subsystems
//
// int VideoID;
//
// Subsystem ID values:
// 	 0  = (none)
// 	 1  = MDA
// 	 2  = CGA
// 	 3  = EGA
// 	 4  = MCGA
// 	 5  = VGA
// 	80h = HGC
// 	81h = HGC+
// 	82h = Hercules InColor
//
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//
// Equates
//
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ



//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//
// Program
//
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ


//from: https://dhw.wolfenstein3d.com/viewtopic.php?printertopic=1&t=5577&start=0&postdays=0&postorder=asc&vote=viewresult
int VL_VideoID (void)
{
	__asm {
   ; Get display combination code.
   ; See RBIL INTERRUP.A - V-101A00.

   mov   ax,1A00h
   int   10h

   cmp   al,1Ah   ; AL = 1Ah if function is supported.
   jne   @@novga

   ; BL = Active display code.
   ; BH = Alternate display code.

   cmp   bl,8      ; Is color analog VGA active?
   jne   @@novga

   ; Make sure an ATI EGA Wonder isnt lying to us.
   ; See RBIL INTERRUP.A - V-101A00 and V-101C.

   mov   ax,1C00h   ; Video save/restore state function.
   mov   cx,2       ; Only video hardware (CX=0) is supported on EGA Wonder.
   int   10h        ; So lets check for color registers and DAC state. (CX=2)

   cmp   al,1Ch     ; AL = 1Ch if function is supported.
   jne   @@novga

   ; Yes, we have an active color analog VGA!

   mov   ax,5    ; Original Wolf sources expect (5) to indicate VGA support.
   jmp   @@done

#ifdef __BORLANDC__
	}
#endif
@@novga:
#ifdef __BORLANDC__
	__asm {
#endif
   xor   ax,ax   ; Indicate failure, no VGA was detected!

#ifdef __BORLANDC__
	}
#endif
@@done:
#ifdef __BORLANDC__
	__asm {
#endif
   ret
	}
	return _AX;
}

/*//=================
//
// SyncVBL
//
//=================


extern dword	TimeCount;
extern word	jerk;
extern word	nopan;


void SyncVBL ()
{
	unsigned i;
	__asm {
	mov	dx,STATUS_REGISTER_1
	mov	bx,[WORD ptr TimeCount]
	add	bx,3
#ifdef __BORLANDC__
	}
#endif
@@waitloop:
#ifdef __BORLANDC__
	__asm {
#endif
	sti
	jmp	$+2
	cli
	cmp	[WORD ptr TimeCount],bx
	je	@@done
#ifdef __BORLANDC__
	}
#endif
@@waitnovert:
#ifdef __BORLANDC__
	__asm {
#endif
	in	al,dx
	test	al,1
	jnz	@@waitnovert
#ifdef __BORLANDC__
	}
#endif
@@waitvert:
#ifdef __BORLANDC__
__asm {
#endif
	in	al,dx
	test	al,1
	jz	@@waitvert
	}

	for(i=0;i<5;i++)
	{
		__asm {
	in	al,dx
	test	al,8
	jnz	@@waitloop
	test	al,1
	jz	@@waitloop
		}
	}

	__asm {
	test	[jerk],1
	jz	@@done
	}

	for(i=0;i<5;i++)
	{
		__asm {
	in	al,dx
	test	al,8
	jnz	@@waitloop
	test	al,1
	jz	@@waitloop
		}
	}

#ifdef __BORLANDC__
	}
#endif
@@done:
#ifdef __BORLANDC__
	__asm {
#endif
	ret
}}*/

word	nopan;
//==============
//
// VW_SetScreen
//
//==============

//PROC	VW_SetScreen  crtc:WORD, pel:WORD
void	VW_SetScreen (word crtc, word pel)
{
	VL_WaitVBL(1);
	__asm {
//
// set CRTC start
//
// for some reason, my XT's EGA card doesn't like word outs to the CRTC
// index...
//
	mov	cx,[crtc]
	mov	dx,CRTC_INDEX
	mov	al,0ch		;start address high register
	out	dx,al
	inc	dx
	mov	al,ch
	out	dx,al
	dec	dx
	mov	al,0dh		;start address low register
	out	dx,al
	mov	al,cl
	inc	dx
	out	dx,al

	test	[nopan],1
	jnz	@@done
//
// set horizontal panning
//

	mov	dx,ATR_INDEX
	mov	al,ATR_PELPAN or 20h
	out	dx,al
	jmp	@@done
	mov	al,[BYTE ptr pel]		//pel pan value
	out	dx,al

#ifdef __BORLANDC__
	}
#endif
@@done:
#ifdef __BORLANDC__
__asm {
#endif
	sti

	ret
}
}




