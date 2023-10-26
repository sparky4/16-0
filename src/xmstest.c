#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define __DEBUG_PM__

#ifdef __WATCOMC__
#include <i86.h>
//----#define _FCORELEFT 0x90000UL+16UL

#define _AX CPURegs.x.ax
#define _BX CPURegs.x.bx
#define _CX CPURegs.x.cx
#define _DX CPURegs.x.dx

#define _SI CPURegs.x.si
#define _DI CPURegs.x.di

#define _AH CPURegs.h.ah
#define _AL CPURegs.h.al
#define _BH CPURegs.h.bh
#define _BL CPURegs.h.bl
#define _CH CPURegs.h.ch
#define _CL CPURegs.h.cl
#define _DH CPURegs.h.dh
#define _DL CPURegs.h.dl

#define _CFLAG CPURegs.x.cflag

#define geninterrupt(n) int86(n,&CPURegs,&CPURegs);

typedef union REGPACK	regs_t;
regs_t CPURegs;
#endif

typedef	enum	{false,true}	boolean;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef	unsigned	long		longword;

typedef struct
{
	dword	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

	boolean			XMSPresent;
	word			XMSAvail,XMSPagesAvail,XMSHandle;
	longword			XMSDriver;
	word				XMSVer;
	int				XMSProtectPage = -1;

mminfotype	mminfo;

#define	XMS_INT			0x2f
#define	XMS_CALL(v)		_AH = (v);\
						__asm call [DWORD PTR XMSDriver]
/*__asm { mov ah,[v]\*///}

#define	XMS_VERSION		0x00

#define	XMS_ALLOCHMA		0x01
#define	XMS_FREEHMA		0x02

#define	XMS_GENABLEA20	0x03
#define	XMS_GDISABLEA20	0x04
#define	XMS_LENABLEA20	0x05
#define	XMS_LDISABLEA20	0x06
#define	XMS_QUERYA20		0x07

#define	XMS_QUERYFREE	0x08
#define	XMS_ALLOC		0x09
#define	XMS_FREE			0x0A
#define	XMS_MOVE			0x0B
#define	XMS_LOCK			0x0C
#define	XMS_UNLOCK		0x0D
#define	XMS_GETINFO		0x0E
#define	XMS_RESIZE		0x0F

#define	XMS_ALLOCUMB		0x10
#define	XMS_FREEUMB		0x11

#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)

void ClearMemory (void)
{
#ifdef __16_PM__
	PM_UnlockMainMem();
#endif
	//sd
	//MM_SortMem ();
}

void Quit (char *error)
{
	//unsigned		finscreen;
	//memptr	screen=0;

	//ClearMemory ();
	ClearMemory();
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
		//gotoxy (10,4);
		fprintf(stderr, "%s\n", error);
		//gotoxy (1,8);
		exit(1);
	}
	else
	if (!error || !(*error))
	{
		//clrscr();
#ifndef JAPAN
		//movedata ((unsigned)screen,7,0xb800,0,4000);
		//gotoxy(1,24);
#endif
//asm	mov	bh,0
//asm	mov	dh,23	// row
//asm	mov	dl,0	// collumn
//asm	mov ah,2
//asm	int	0x10
	}

	exit(0);
}

/////////////////////////////////////////////////////////////////////////////
//
//	XMS Management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_StartupXMS() - Starts up XMS for the Page Mgr's use
//		Checks for presence of an XMS driver
//		Makes sure that there's at least a page of XMS available
//		Allocates any remaining XMS (rounded down to the nearest page size)
//
boolean
PML_StartupXMS(void)
{
//TODO:		translate the _REG into working assembly
//FIXME:	FIX THE XMS STUFF WITH BORLANDC
//INFO:		XMS strrangely works on my 286 named cherry4 and no where else...
#define STARTUPXMSASM
	byte err = 0;
	boolean errorflag=false;
	word e=0;
	XMSPresent = false;					// Assume failure
	XMSAvail = mminfo.XMSmem = 0;

	__asm {
		mov	ax,0x4300
		int	XMS_INT         				// Check for presence of XMS driver
		cmp	al,0x80
		jne	error1
		mov	e,1

		mov	ax,0x4310
		int	XMS_INT							// Get address of XMS driver
		mov	[WORD PTR XMSDriver],bx
		mov	[WORD PTR XMSDriver+2],es		// function pointer to XMS driver

		mov	ah,XMS_VERSION
		call	[DWORD PTR XMSDriver]						//; Get XMS Version Number
		mov	[XMSVer],ax
		mov	e,2

#ifdef STARTUPXMSASM
		mov	ah,XMS_QUERYFREE			// Find out how much XMS is available
		call	[DWORD PTR XMSDriver]
		mov	XMSAvail,ax
		or	ax,ax				// AJR: bugfix 10/8/92
		jz	error1
		mov	e,3
#endif
		jmp	End1
#ifdef __BORLANDC__
	}
#endif
	error1:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	err,bl
		mov	errorflag,1
		jmp End1
#ifdef __BORLANDC__
	}
#endif
End1:
#ifdef __WATCOMC__
	}
#endif
	if(errorflag==true) goto error;
#ifndef STARTUPXMSASM
	XMS_CALL(XMS_QUERYFREE);			// Find out how much XMS is available
	XMSAvail = _AX;
	if (!_AX)				// AJR: bugfix 10/8/92
	{
		errorflag = true;
		err = _BL;
		goto error;
	}
	e++;
#endif

#ifdef __DEBUG_PM__
	printf("XMSVer=%02X	", XMSVer);
	printf("XMSAvail=%u\n", XMSAvail);
//	getch();
#endif
	XMSAvail &= ~(PMPageSizeKB - 1);	// Round off to nearest page size
	if (XMSAvail < (PMPageSizeKB * 2))	// Need at least 2 pages
	{
		errorflag=true;
		goto error;
	}
#ifdef STARTUPXMSASM
	__asm {
		mov	dx,XMSAvail
		mov	ah,XMS_ALLOC				// And do the allocation
		call	[DWORD PTR XMSDriver]
		mov	XMSHandle,dx
		or	ax,ax				// AJR: bugfix 10/8/92
		jz	error2
		mov	e,4
		jmp	End2
#ifdef __BORLANDC__
	}
#endif
	error2:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	err,bl
		mov	errorflag,1
		jmp End2
#ifdef __BORLANDC__
	}
#endif
End2:
#ifdef __WATCOMC__
	}
#endif
#else
	_DX = XMSAvail;
	XMS_CALL(XMS_ALLOC);				// And do the allocation
	XMSHandle = _DX;
	if (!_AX)				// AJR: bugfix 10/8/92
	{
		errorflag=true;
		err = _BL;
		goto error;
	}
	e++;
#endif
error:
	if(errorflag==false)
	{
		mminfo.XMSmem = (dword)(XMSAvail) * 1024;
		XMSPresent = true;
#ifdef __DEBUG_PM__
		printf("	mminfo.XMSmem=%lu	XMSAvail=%u\n", mminfo.XMSmem, XMSAvail);
#endif
	}
	else
	{
#ifdef __DEBUG_PM__
		//printf("XMSHandle\n");
		//printf("	1=%u	2=%u	3=%u	4=%u\n", XMSHandle1, XMSHandle2, XMSHandle3, XMSHandle4);
		//printf("	2=%u	", XMSHandle);
		printf("	%u\n", XMSHandle);
		printf("XMSDriver:	%Fp\n", XMSDriver);
		printf("XMSDriver:	%lu\n", XMSDriver);
		printf("	err=%02X	e=%u\n", err, e);
#endif
	}
	return(XMSPresent);
}

//
//	PML_ShutdownXMS()
//
void
PML_ShutdownXMS(void)
{
	byte err = 0;
	boolean errorflag=false;
	if (XMSPresent)
	{
#ifdef STARTUPXMSASM
		__asm {
			mov	dx,XMSHandle
			//XMS_CALL(XMS_FREE);
			mov	ah,XMS_FREE
			call	[DWORD PTR XMSDriver]
			or	bl,bl
			jz	errorxs
			jmp	Endxs
#ifdef __BORLANDC__
		}
#endif
			errorxs:
#ifdef __BORLANDC__
		__asm {
#endif
			mov	err,ah
			mov	errorflag,1
			jmp	Endxs
#ifdef __BORLANDC__
		}
#endif
			Endxs:
#ifdef __WATCOMC__
		}
#endif
#else	// STARTUPXMSASM
		_DX = XMSHandle;
		XMS_CALL(XMS_FREE);
		if (_BL)
		{
			err = _AL;
			errorflag=true;
		}
#endif	// STARTUPXMSASM
		if(errorflag==true)
			Quit ("PML_ShutdownXMS: Error freeing XMS");
	}
}

void main ()
{
	PML_StartupXMS();
	PML_ShutdownXMS();

}
