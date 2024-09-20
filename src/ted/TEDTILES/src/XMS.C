////////////////////////////////////////////////////
//
// XMS routines
//
////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop


unsigned XMSavail;
void far *XMSdriver;


////////////////////////////////////////////////////
//
// Initialize the XMS memory
//
////////////////////////////////////////////////////
int InitXMS(void)
{
 //
 // See if XMS driver is present...
 //
 asm	mov	ax,0x4300
 asm	int	0x2f
 asm	cmp	al,0x80		// installed?
 asm	je	Present

 return -1;

 //
 // YES! We found an XMS driver! Now we
 // need to get the XMS driver's entry address...
 //
 Present:
 asm	mov	ax,0x4310
 asm	int	0x2f
 asm	mov	WORD PTR XMSdriver,bx
 asm	mov	WORD PTR XMSdriver+2,es

 return 0;
}


////////////////////////////////////////////////////
//
// Report an XMS error, if any
//
////////////////////////////////////////////////////
void Quit(char *string)
{
 _AX=3;
 geninterrupt(0x10);
 printf("XMS ERROR: %s\n",string);
 exit(1);
}

void XMSerror(void)
{
 if (_AX==0)
   switch(_BL)
   {
    case 0xa0: Quit("Not enough Extended Memory available!");
    case 0xa7: Quit("Invalid length!");
    case 0xa2: Quit("Invalid handle!");
    case 0xa3:
    case 0xa4:
    case 0xa5:
    case 0xa6: Quit("Invalid source or dest handle/offset!");
    default: Quit("Unknown XMS Memory Error!");
   }
}

////////////////////////////////////////////////////
//
// Allocate XMS memory
//
////////////////////////////////////////////////////
int XMSAllocate(long size)
{
 unsigned rdx=(size+1023)/1024;

 asm	mov	dx,[rdx]
 asm	mov	ah,9
 asm	call	[ss:DWORD PTR XMSdriver]

 XMSerror();
 return _DX;
}

////////////////////////////////////////////////////
//
// Free XMS memory
//
////////////////////////////////////////////////////
void XMSFreeMem(int handle)
{
 asm	mov  dx,[handle]
 asm	mov  ah,10
 asm	call [DWORD PTR XMSdriver]
 XMSerror();
}


////////////////////////////////////////////////////
//
// Return XMS memory available
//
////////////////////////////////////////////////////
unsigned XMSTotalFree(void)
{
 asm	mov ah,8
 asm	call [DWORD PTR XMSdriver]
 if (!_AX && _BL!=0xa0)
   XMSerror();
 XMSavail=_DX;
 return XMSavail;
}

////////////////////////////////////////////////////
//
// Move XMS memory
//
////////////////////////////////////////////////////
struct { long bsize;
	 int shandle;
	 long soff;
	 int dhandle;
	 long doff;
       } XMSparms;


void XMSmove(int srchandle,long srcoff,int desthandle,long destoff,long size)
{
 unsigned DSreg,SIreg;

 //
 // FOR SOME FUCKING REASON, THE NEW MS-DOS 5.0
 // HIMEM.SYS DRIVER DOESN'T ACCEPT ODD XMSMOVE LENGTHS!!!
 // JOHN & I SPENT 5.5 HOURS DEBUGGING THIS BULLSHIT!!
 // ---- FUCK YOU MICROSOFT!!! ---- (ASSHOLES!)
 //
 XMSparms.bsize=(size+1)&~1;
 XMSparms.shandle=srchandle;
 XMSparms.dhandle=desthandle;
 XMSparms.soff=srcoff;
 XMSparms.doff=destoff;

 DSreg=FP_SEG(&XMSparms);
 SIreg=FP_OFF(&XMSparms);

 asm	mov	si,SIreg
 asm	mov	ds,DSreg

 asm	mov	ah,11
 asm	call	[ss:DWORD PTR XMSdriver]

 XMSerror();
}


////////////////////////////////////////////////////
//
// XMS information
//
////////////////////////////////////////////////////
void XMSHandleInfo(int handle)
{
 asm	mov   dx,[handle]
 asm	mov   ah,0xe
 asm	call  [DWORD PTR XMSdriver]
}
