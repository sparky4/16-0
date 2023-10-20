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

unsigned	bufferofs;
unsigned	displayofs,pelpan;

unsigned	screenseg=SCREENSEG;		// set to 0xa000 for asm convenience

unsigned	linewidth;
unsigned	ylookup[MAXSCANLINES];

boolean		screenfaded;
unsigned	bordercolor;

boolean		fastpalette;				// if true, use outsb to set

byte		far	palette1[256][3],far palette2[256][3];

//===========================================================================

// asm

int	 VL_VideoID (void);
void VL_SetCRTC (int crtc);
void VL_SetScreen (int crtc, int pelpan);
word VL_WaitVBL (word vbls);

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
dword far*ptr=(dword far*)SCREENSEG;      /* used for faster screen clearing */
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
outportw(SC_INDEX, 0x0604);

/* synchronous reset while setting Misc Output */
outportw(SC_INDEX, 0x0100);

/* select 25 MHz dot clock & 60 Hz scanning rate */
outportb(MISC_OUTPUT, 0xe3);

/* undo reset (restart sequencer) */
outportw(SC_INDEX, 0x0300);

/* reprogram the CRT controller */
outport(CRTC_INDEX, 0x11); /* VSync End reg contains register write prot */
outport(CRTC_DATA, 0x7f);  /* get current write protect on varios regs */

/* send the CRTParms */
for(i=0; i<CRTParmCount; i++) {
	outportw(CRTC_INDEX, CRTParms[i]);
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

	asm	mov	dx,PEL_WRITE_ADR
	asm	mov	al,0
	asm	out	dx,al
	asm	mov	dx,PEL_DATA
	asm	lds	si,[palette]

	asm	test	[ss:fastpalette],1
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
	slowset:
	asm	mov	cx,256
	setloop:
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	lodsb
	asm	out	dx,al
	asm	loop	setloop

	done:
	asm	mov	ax,ss
	asm	mov	ds,ax

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

	asm	mov	di,[y]				// dest = bufferofs+ylookup[y]+(x>>2)
	asm	shl	di,1
	asm	mov	di,[WORD PTR ylookup+di]
	asm	add	di,[bufferofs]
	asm	mov	ax,[x]
	asm	shr	ax,1
	asm	shr	ax,1
	asm	add	di,ax

	asm	mov	si,[source]
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
	asm	add	di,bx
	asm	dec	dx
	asm	jnz	drawline

	asm	mov	ax,ss
	asm	mov	ds,ax

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
/*void VL_VideoID()
__asm {
	push	bp	; preserve caller registers
	mov	bp,sp
	push	ds
	push	si
	push	di

	push	cs
	pop	ds
	ASSUME	DS:@Code

	; initialize the data structure that will contain the results

	lea	di,Results	; DS:DI -> start of data structure

	mov	Device0,0	; zero these variables
	mov	Device1,0

	; look for the various subsystems using the subroutines whose addresses are
	; tabulated in TestSequence; each subroutine sets flags in TestSequence
	; to indicate whether subsequent subroutines need to be called

	mov	byte ptr CGAflag,TRUE
	mov	byte ptr EGAflag,TRUE
	mov	byte ptr Monoflag,TRUE

	mov	cx,NumberOfTests
	mov	si,offset TestSequence

	@@L01:	lodsb		; AL := flag
	test	al,al
	lodsw		; AX := subroutine address
	jz	@@L02	; skip subroutine if flag is false

	push	si
	push	cx
	call	ax	; call subroutine to detect subsystem
	pop	cx
	pop	si

	@@L02:	loop	@@L01

	; determine which subsystem is active

	call	FindActive

	mov	al,Results.Video0Type
	mov	ah,0	; was:  Results.Display0Type

	pop	di	; restore caller registers and return
	pop	si
	pop	ds
	mov	sp,bp
	pop	bp
	ret
}
}*/

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

//	VL_WaitVBL  num:WORD
word	VL_WaitVBL(

@@wait:

	mov	dx,STATUS_REGISTER_1

	mov	cx,[num]
//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
@@waitnosync:
	in	al,dx
	test	al,8
	jnz	@@waitnosync


@@waitsync:
	in	al,dx
	test	al,8
	jz	@@waitsync

	loop	@@waitnosync
	}
return num

}


//===========================================================================

//==============
//
// VL_SetCRTC
//
//==============

PROC	VL_SetCRTC  crtc:WORD
PUBLIC	VL_SetCRTC

//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
	cli

	mov	dx,STATUS_REGISTER_1

@@waitdisplay:
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

	ret

ENDP



//===========================================================================

//==============
//
// VL_SetScreen
//
//==============

PROC	VL_SetScreen  crtc:WORD, pel:WORD
PUBLIC	VL_SetScreen


	mov	cx,[timecount]		// if timecount goes up by two, the retrace
	add	cx,2				// period was missed (an interrupt covered it)

	mov	dx,STATUS_REGISTER_1

//
// wait for a display signal to make sure the raster isn't in the middle
// of a sync
//
@@waitdisplay:
	in	al,dx
	test	al,1	//1 = display is disabled (HBL / VBL)
	jnz	@@waitdisplay


@@loop:
	sti
	jmp	$+2
	cli

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


@@setcrtc:


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
	jmp	$+2
	mov	al,[BYTE pel]		//pel pan value
	out	dx,al

	sti

	ret

ENDP


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

PROC	VL_ScreenToScreen	source:WORD, dest:WORD, wide:WORD, height:WORD
PUBLIC	VL_ScreenToScreen
USES	SI,DI

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

@@lineloop:
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

	ret

ENDP


//===========================================================================


	MASM
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
VIDstruct	STRUC		// corresponds to C data structure

Video0Type	DB	?	// first subsystem type
Display0Type	DB	? 	// display attached to first subsystem

Video1Type	DB	?	// second subsystem type
Display1Type	DB	?	// display attached to second subsystem

VIDstruct	ENDS


Device0	EQU	word ptr Video0Type[di]
Device1	EQU	word ptr Video1Type[di]


MDA	EQU	1	// subsystem types
CGA	EQU	2
EGA	EQU	3
MCGA	EQU	4
VGA	EQU	5
HGC	EQU	80h
HGCPlus	EQU	81h
InColor	EQU	82h

MDADisplay	EQU	1	// display types
CGADisplay	EQU	2
EGAColorDisplay	EQU	3
PS2MonoDisplay	EQU	4
PS2ColorDisplay	EQU	5

TRUE	EQU	1
FALSE	EQU	0

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//
// Program
//
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

Results	VIDstruct <>	//results go here!

EGADisplays	DB	CGADisplay	// 0000b, 0001b	(EGA switch values)
	DB	EGAColorDisplay	// 0010b, 0011b
	DB	MDADisplay	// 0100b, 0101b
	DB	CGADisplay	// 0110b, 0111b
	DB	EGAColorDisplay	// 1000b, 1001b
	DB	MDADisplay	// 1010b, 1011b

DCCtable	DB	0,0	// translate table for INT 10h func 1Ah
	DB	MDA,MDADisplay
	DB	CGA,CGADisplay
	DB	0,0
	DB	EGA,EGAColorDisplay
	DB	EGA,MDADisplay
	DB	0,0
	DB	VGA,PS2MonoDisplay
	DB	VGA,PS2ColorDisplay
	DB	0,0
	DB	MCGA,EGAColorDisplay
	DB	MCGA,PS2MonoDisplay
	DB	MCGA,PS2ColorDisplay

TestSequence	DB	TRUE	// this list of flags and addresses
	DW	FindPS2	//  determines the order in which this
			//  program looks for the various
EGAflag	DB	?	//  subsystems
	DW	FindEGA

CGAflag	DB	?
	DW	FindCGA

Monoflag	DB	?
	DW	FindMono

NumberOfTests	EQU	($-TestSequence)/3


PUBLIC	VL_VideoID
VL_VideoID	PROC

	push	bp	// preserve caller registers
	mov	bp,sp
	push	ds
	push	si
	push	di

	push	cs
	pop	ds
	ASSUME	DS:@Code

// initialize the data structure that will contain the results

	lea	di,Results	// DS:DI -> start of data structure

	mov	Device0,0	// zero these variables
	mov	Device1,0

// look for the various subsystems using the subroutines whose addresses are
// tabulated in TestSequence// each subroutine sets flags in TestSequence
// to indicate whether subsequent subroutines need to be called

	mov	byte ptr CGAflag,TRUE
	mov	byte ptr EGAflag,TRUE
	mov	byte ptr Monoflag,TRUE

	mov	cx,NumberOfTests
	mov	si,offset TestSequence

@@L01:	lodsb		// AL := flag
	test	al,al
	lodsw		// AX := subroutine address
	jz	@@L02	// skip subroutine if flag is false

	push	si
	push	cx
	call	ax	// call subroutine to detect subsystem
	pop	cx
	pop	si

@@L02:	loop	@@L01

// determine which subsystem is active

	call	FindActive

	mov	al,Results.Video0Type
	mov	ah,0	// was:  Results.Display0Type

	pop	di	// restore caller registers and return
	pop	si
	pop	ds
	mov	sp,bp
	pop	bp
	ret

VL_VideoID	ENDP


//
// FindPS2
//
// This subroutine uses INT 10H function 1Ah to determine the video BIOS
// Display Combination Code (DCC) for each video subsystem present.
//

FindPS2	PROC	near

	mov	ax,1A00h
	int	10h	// call video BIOS for info

	cmp	al,1Ah
	jne	@@L13	// exit if function not supported (i.e.,
			//  no MCGA or VGA in system)

// convert BIOS DCCs into specific subsystems & displays

	mov	cx,bx
	xor	bh,bh	// BX := DCC for active subsystem

	or	ch,ch
	jz	@@L11	// jump if only one subsystem present

	mov	bl,ch	// BX := inactive DCC
	add	bx,bx
	mov	ax,[bx+offset DCCtable]

	mov	Device1,ax

	mov	bl,cl
	xor	bh,bh	// BX := active DCC

@@L11:	add	bx,bx
	mov	ax,[bx+offset DCCtable]

	mov	Device0,ax

// reset flags for subsystems that have been ruled out

	mov	byte ptr CGAflag,FALSE
	mov	byte ptr EGAflag,FALSE
	mov	byte ptr Monoflag,FALSE

	lea	bx,Video0Type[di]  // if the BIOS reported an MDA ...
	cmp	byte ptr [bx],MDA
	je	@@L12

	lea	bx,Video1Type[di]
	cmp	byte ptr [bx],MDA
	jne	@@L13

@@L12:	mov	word ptr [bx],0    // ... Hercules can't be ruled out
	mov	byte ptr Monoflag,TRUE

@@L13:	ret

FindPS2	ENDP


//
// FindEGA
//
// Look for an EGA.  This is done by making a call to an EGA BIOS function
//  which doesn't exist in the default (MDA, CGA) BIOS.

FindEGA	PROC	near	// Caller:	AH = flags
			// Returns:	AH = flags
			//		Video0Type and
			//		 Display0Type updated

	mov	bl,10h	// BL := 10h (return EGA info)
	mov	ah,12h	// AH := INT 10H function number
	int	10h	// call EGA BIOS for info
			// if EGA BIOS is present,
			//  BL <> 10H
			//  CL = switch setting
	cmp	bl,10h
	je	@@L22	// jump if EGA BIOS not present

	mov	al,cl
	shr	al,1	// AL := switches/2
	mov	bx,offset EGADisplays
	xlat		// determine display type from switches
	mov	ah,al	// AH := display type
	mov	al,EGA	// AL := subystem type
	call	FoundDevice

	cmp	ah,MDADisplay
	je	@@L21	// jump if EGA has a monochrome display

	mov	CGAflag,FALSE	// no CGA if EGA has color display
	jmp	short @@L22

@@L21:	mov	Monoflag,FALSE	// EGA has a mono display, so MDA and
			//  Hercules are ruled out
@@L22:	ret

FindEGA	ENDP

//
// FindCGA
//
// This is done by looking for the CGA's 6845 CRTC at I/O port 3D4H.
//
FindCGA	PROC	near	// Returns:	VIDstruct updated

	mov	dx,3D4h	// DX := CRTC address port
	call	Find6845
	jc	@@L31	// jump if not present

	mov	al,CGA
	mov	ah,CGADisplay
	call	FoundDevice

@@L31:	ret

FindCGA	ENDP

//
// FindMono
//
// This is done by looking for the MDA's 6845 CRTC at I/O port 3B4H.  If
// a 6845 is found, the subroutine distinguishes between an MDA
// and a Hercules adapter by monitoring bit 7 of the CRT Status byte.
// This bit changes on Hercules adapters but does not change on an MDA.
//
// The various Hercules adapters are identified by bits 4 through 6 of
// the CRT Status value:
//
// 000b = HGC
// 001b = HGC+
// 101b = InColor card
//

FindMono	PROC	near	// Returns:	VIDstruct updated

	mov	dx,3B4h	// DX := CRTC address port
	call	Find6845
	jc	@@L44	// jump if not present

	mov	dl,0BAh	// DX := 3BAh (status port)
	in	al,dx
	and	al,80h
	mov	ah,al	// AH := bit 7 (vertical sync on HGC)

	mov	cx,8000h	// do this 32768 times
@@L41:	in	al,dx
	and	al,80h	// isolate bit 7
	cmp	ah,al
	loope	@@L41	// wait for bit 7 to change
	jne	@@L42	// if bit 7 changed, it's a Hercules

	mov	al,MDA	// if bit 7 didn't change, it's an MDA
	mov	ah,MDADisplay
	call	FoundDevice
	jmp	short @@L44

@@L42:	in	al,dx
	mov	dl,al	// DL := value from status port
	and	dl,01110000b	// mask bits 4 thru 6

	mov	ah,MDADisplay	// assume it's a monochrome display

	mov	al,HGCPlus	// look for an HGC+
	cmp	dl,00010000b
	je	@@L43	// jump if it's an HGC+

	mov	al,HGC	// look for an InColor card or HGC
	cmp	dl,01010000b
	jne	@@L43	// jump if it's not an InColor card

	mov	al,InColor	// it's an InColor card
	mov	ah,EGAColorDisplay

@@L43:	call	FoundDevice

@@L44:	ret

FindMono	ENDP

//
// Find6845
//
// This routine detects the presence of the CRTC on a MDA, CGA or HGC.
// The technique is to write and read register 0Fh of the chip (cursor
// low).  If the same value is read as written, assume the chip is
// present at the specified port addr.
//

Find6845	PROC	near	// Caller:  DX = port addr
			// Returns: cf set if not present
	mov	al,0Fh
	out	dx,al	// select 6845 reg 0Fh (Cursor Low)
	inc	dx
	in	al,dx	// AL := current Cursor Low value
	mov	ah,al	// preserve in AH
	mov	al,66h	// AL := arbitrary value
	out	dx,al	// try to write to 6845

	mov	cx,100h
@@L51:	loop	@@L51	// wait for 6845 to respond

	in	al,dx
	xchg	ah,al	// AH := returned value
			// AL := original value
	out	dx,al	// restore original value

	cmp	ah,66h	// test whether 6845 responded
	je	@@L52	// jump if it did (cf is reset)

	stc		// set carry flag if no 6845 present

@@L52:	ret

Find6845	ENDP


//
// FindActive
//
// This subroutine stores the currently active device as Device0.  The
// current video mode determines which subsystem is active.
//

FindActive	PROC	near

	cmp	word ptr Device1,0
	je	@@L63	// exit if only one subsystem

	cmp	Video0Type[di],4	// exit if MCGA or VGA present
	jge	@@L63	//  (INT 10H function 1AH
	cmp	Video1Type[di],4	//  already did the work)
	jge	@@L63

	mov	ah,0Fh
	int	10h	// AL := current BIOS video mode

	and	al,7
	cmp	al,7	// jump if monochrome
	je	@@L61	//  (mode 7 or 0Fh)

	cmp	Display0Type[di],MDADisplay
	jne	@@L63	// exit if Display0 is color
	jmp	short @@L62

@@L61:	cmp	Display0Type[di],MDADisplay
	je	@@L63	// exit if Display0 is monochrome

@@L62:	mov	ax,Device0	// make Device0 currently active
	xchg	ax,Device1
	mov	Device0,ax

@@L63:	ret

FindActive	ENDP


//
// FoundDevice
//
// This routine updates the list of subsystems.
//

FoundDevice	PROC	near	// Caller:    AH = display #
			//	     AL = subsystem #
			// Destroys:  BX
	lea	bx,Video0Type[di]
	cmp	byte ptr [bx],0
	je	@@L71	// jump if 1st subsystem

	lea	bx,Video1Type[di]	// must be 2nd subsystem

@@L71:	mov	[bx],ax	// update list entry
	ret

FoundDevice	ENDP

IDEAL



END
