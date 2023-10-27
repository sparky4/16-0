////////////////////////////////////////////////////////////
//
// STANDARD VGA GRABBING
//
////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

////////////////////////////////////////////////////////////
//
// VGAblit : blits a VGA shape from memory to the VGA screen
//
////////////////////////////////////////////////////////////
void VGAblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned ESreg,DIreg,SIreg,DSreg,where,owidth,addx;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 if (x>319 || y>199)
   globalx=globaly=globalmaxh=x=y=0;

 addx=0;
 if (x+width>319)
   {
    owidth=width;
    width=320-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    height=200-y;
   }

 DSreg=FP_SEG(buffer);
 SIreg=FP_OFF(buffer);
 where=y*320+x;

 asm	push	si
 asm	push	di
 asm	push	ds
 asm	pushf

 asm	cld
 asm	mov	ax,0x0a000
 asm	mov	es,ax
 asm	mov	si,[SIreg]
 asm	mov	ax,[DSreg]
 asm	mov	ds,ax

 asm	mov	bx,[y]
 asm	mov	di,[where]
 asm	mov	dx,[height]
 LOOP1:
 asm	mov	cx,[width]
 asm	rep movsb
 asm	mov	cx,[addx]	// any to finish up horizontally?
 asm	jcxz	LOOP1a
 asm	rep lodsb

 LOOP1a:
 asm	sub	di,[width]
 asm	add	di,320

 LOOP1a0:
 asm	dec	dx
 asm	jnz	LOOP1

 asm	popf
 asm	pop	ds
 asm	pop	di
 asm	pop	si
}


////////////////////////////////////////////////////////////
//
// DoVGAblit : handles output of VGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoVGAblit(int x,int y,int width,int height)
{
 if (noshow || SkipToStart)
   return;

 if (nostacking)
   {
    globalx=x;
    globaly=y;
   }

 VGAblit(globalx,globaly,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>320-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// VGAgrab : grabs any VGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
// NOTE: I expect X & WIDTH to be in BYTE values, not pixels!
//
////////////////////////////////////////////////////////////
void VGAgrab(int x,int y,int width,int height,unsigned offset)
{
 unsigned ESreg,DIreg,SIreg,DSreg,scrnwid,loc;

 scrnwid=CurrentLBM.width;
 loc=y*scrnwid+x;

 // FROM
 SIreg=FP_OFF(lbmscreen)+(loc&15);
 DSreg=FP_SEG(lbmscreen)+(loc/16);

 // TO
 ESreg=FP_SEG(databuffer+offset);
 DIreg=FP_OFF(databuffer+offset);

 asm		push	si
 asm		push	di
 asm		push	ds

 asm		mov	bx,height
 asm		mov	dx,width

 asm		mov	es,ESreg
 asm		mov	di,DIreg
 asm		mov	si,SIreg
 asm		mov	ax,DSreg
 asm		mov	ds,ax
 asm		cld

 LOOP1:

 asm		mov	cx,dx
 asm		rep movsb

 asm		sub	si,dx
 asm		add	si,scrnwid
 asm		dec	bx
 asm		jnz	LOOP1

 asm		pop	ds
 asm		pop	di
 asm		pop	si

 offset+=width*height;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
// MASKED STUFF
//
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// VGAMblit : blits a VGA masked shape from memory to the VGA screen
//
////////////////////////////////////////////////////////////
void VGAMblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned MASKoff,DATAoff,ESreg,DIreg,SIreg,DSreg,where,oheight,owidth,addx;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 addx=0;
 owidth=width;
 oheight=height;

 if (x+width>319)
   {
    width=320-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    height=200-y;
   }

 DSreg=FP_SEG(buffer);
 MASKoff=FP_OFF(buffer);
 DATAoff=FP_OFF((char far *)buffer+owidth*oheight);
 where=y*320+x;

 asm	push	si
 asm	push	di
 asm	push	ds
 asm	pushf

 asm	mov	dx,[DATAoff]
 asm	mov	ds,[DSreg]

 asm	mov	si,[MASKoff]
 asm	mov	di,[where]
 asm	mov	ax,0xa000
 asm	mov	es,ax

 asm	mov	bl,[BYTE PTR y]
 asm	mov	bh,[BYTE PTR height]
 LOOP1:
 asm	mov	cx,[width]
 LOOP1c:
 asm	mov	al,[es:di]
 asm	and	al,[si]		// get mask byte (SI=mask data)
 asm	inc	si
 asm	xchg	si,dx		// SI now = VGA data
 asm	or	al,[si]
 asm	inc	si
 asm	xchg	si,dx		// SI now = MASK data
 asm	stosb
 asm	loop LOOP1c
 asm	mov	cx,[addx]	// any to finish up horizontally?
 asm	jcxz	LOOP1a
 asm	add	si,cx
 asm	add	dx,cx

 LOOP1a:
 asm	sub	di,[width]	// calculate next VGA line
 asm	add	di,320

 LOOP1a0:
 asm	dec	bh
 asm	jnz	LOOP1

 asm	popf
 asm	pop	ds
 asm	pop	di
 asm	pop	si
}


////////////////////////////////////////////////////////////
//
// DoVGAMblit : handles output of masked EGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoVGAMblit(int x,int y,int width,int height,int yadd,int hadd)
{
 int i,j;

 if (noshow || SkipToStart)
   return;

 if (nostacking)
   {
    globalx=x;
    globaly=y;
    yadd=hadd=0;
   }
 else
 if (yadd || hadd)
   {
    char huge *VGAscrn=MK_FP(0xa000,0);

    for (j=globaly;j<globaly+yadd;j++)
      for (i=globalx;i<globalx+width;i++)
	*(VGAscrn+j*320+i)^=0x55;

    for (j=globaly+yadd+height;j<height+globaly+yadd+hadd;j++)
      for (i=globalx;i<globalx+width;i++)
	*(VGAscrn+j*320+i)^=0x55;
   }

 VGAMblit(globalx,globaly+yadd,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>320-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// VGAMgrab: grabs any masked VGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
////////////////////////////////////////////////////////////
void VGAMgrab(int x,int y,int width,int height,unsigned offset,int optimize)
{
 unsigned j,maskoff,ESreg,DIreg,SIreg,DSreg,scrnwid,tmpset=0;

 scrnwid=CurrentLBM.width;

 //
 // Does caller want vertical-seek optimization?
 //
 if (optimize)
   {
    Optimum.height=Optimum.y=0;

    for (i=y;i<y+height;i++)
      {
       for (j=x;j<x+width;j++)
	 if (*(maskscreen+i*scrnwid+j)!=0xff)
	   { Optimum.y=i; break; }
       if (Optimum.y)
	 break;
      }

    for (i=y+height-1;i>=0;i--)
      {
       for (j=x;j<x+width;j++)
	 if (*(maskscreen+i*scrnwid+j)!=0xff)
	   { Optimum.height=i-y; break; }
       if (Optimum.height)
	 break;
      }

    if (Optimum.height && Optimum.height!=height)
      height=Optimum.height-(Optimum.y-y)+1;

    if (Optimum.y && Optimum.y!=y)
      y=Optimum.y;
   }


 // FROM
 SIreg=FP_OFF(maskscreen)+((scrnwid*y+x)&15);
 DSreg=FP_SEG(maskscreen)+((scrnwid*y+x)/16);

 // TO
 ESreg=FP_SEG(databuffer+offset);
 DIreg=FP_OFF(databuffer+offset);

 asm		push	si
 asm		push	di
 asm		push	ds

 asm		mov	bx,[height]
 asm		mov	dx,[width]

 asm		mov	es,[ESreg]
 asm		mov	di,[DIreg]
 asm		mov	si,[SIreg]
 asm		mov	ax,[DSreg]
 asm		mov	ds,ax
 asm		cld

 LOOP0:

 asm		mov	cx,dx
 LOOP00:
 asm		lodsb
 asm		or	al,al
 asm		jz	LOOP01
 asm		mov	al,0xff		// Make MASK color ALWAYS 0xFF!
 asm		mov	[tmpset],1

 LOOP01:
 asm		stosb
 asm		loop	LOOP00

 asm		sub	si,dx
 asm		add	si,[scrnwid]
 asm		dec	bx
 asm		jnz	LOOP0

 asm		pop	ds
 asm		pop	di
 asm		pop	si

 maskoff=DIreg;

 // FROM
 SIreg=FP_OFF(lbmscreen)+((scrnwid*y+x)&15);
 DSreg=FP_SEG(lbmscreen)+((scrnwid*y+x)/16);

 // TO
 DIreg+=width*height;

 asm		push	si
 asm		push	di
 asm		push	ds

 asm		mov	dx,[width]

 asm		mov	es,[ESreg]
 asm		mov	di,[DIreg]	// DI=VGA data offset
 asm		mov	si,[SIreg]	// SI=VGA screen offset
 asm		mov	ds,[DSreg]
 asm		mov	bx,[maskoff]	// BX=mask offset
 asm		mov	ah,[BYTE PTR height]

 LOOP1:
 asm		mov	cx,dx

 LOOP2:
 asm		lodsb			// get VGA data
 asm		mov	ch,[es:bx]	// get mask byte
 asm		xor	ch,0xff		// and invert it!
 asm		and	al,ch		// AND with mask
 asm		xor	ch,ch		// make sure LOOP keeps going!
 asm		stosb			// store
 asm		inc	bx		// next mask byte
 asm		loop LOOP2		// do all VGA bytes

 asm		sub	si,dx
 asm		add	si,[scrnwid]
 asm		dec	ah
 asm		jnz	LOOP1

 asm		pop	ds
 asm		pop	di
 asm		pop	si

 //
 // SEE IF WE NEED TO ELIMINATE THE MASK & SET THE
 // BIT IN THE PACKED-BIT ARRAY. MASK-ELIMINATION IS
 // DONE BY MOVING THE TILE DATA BACK OVER THE MASK.
 //
 setbit=tmpset^1;

 if (bit && setbit)
   {
    char masks[8]={0x80,0x40,0x20,0x10,8,4,2,1};
    unsigned psize=width*height,domove=0;

    switch(type)
    {
     case TILE8MTYPE:
     case ALT8MTYPE:
       if (!cmpt8)
	 break;

       T8bit[T8whichbit/8]|=masks[T8whichbit%8];
       domove=1;
       break;
     case TILE16MTYPE:
     case ALT16MTYPE:
       T16bit[T16whichbit/8]|=masks[T16whichbit%8];
       domove=1;
       break;
     case TILE32MTYPE:
     case ALT32MTYPE:
       T32bit[T32whichbit/8]|=masks[T32whichbit%8];
       domove=1;
    }

    if (domove)
      {
       asm	push	si
       asm	push	di
       asm	push	ds

       asm	mov	si,[DIreg]
       asm	mov	ax,[ESreg]
       asm	mov	ds,ax
       asm	mov	es,ax		// ES:DI - mask data
       asm	mov	di,si
       asm	add	si,[psize]	// DS:SI = tile data

       asm	mov	cx,[psize]	// move all tile data
       asm	cld			// move forwards
       asm	rep movsb

       asm	pop	ds
       asm	pop	di
       asm	pop	si
      }
   }

}
