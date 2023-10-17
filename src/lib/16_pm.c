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

//
//	ID_PM.C
//	Id Engine's Page Manager v1.0
//	Primary coder: Jason Blochowiak
//

#include "src/lib/16_pm.h"
#pragma hdrstop


//	Main Mem specific variables
	boolean			MainPresent;
	memptr			MainMemPages[PMMaxMainMem];
	PMBlockAttr		MainMemUsed[PMMaxMainMem];
	int				MainPagesAvail;

//	EMS specific variables
	boolean			EMSPresent;
	word			EMSAvail,EMSPagesAvail,EMSHandle,
					EMSPageFrame,EMSPhysicalPage,EMSVer,
					totalEMSpages, freeEMSpages, EMSpagesmapped;
	EMSListStruct	EMSList[EMSFrameCount];

//	XMS specific variables
	boolean			XMSPresent;
	word			XMSAvail,XMSPagesAvail,XMSHandle;
	dword			XMSDriver;
	word				XMSVer;
	int				XMSProtectPage = -1;

//	File specific variables
	char			PageFileName[13] = {"VSWAP."};
	int				PageFile = -1;
	word			ChunksInFile;
	word			PMSpriteStart,PMSoundStart;

//	General usage variables
	boolean			PMStarted,
					PMPanicMode,
					PMThrashing;
	word			XMSPagesUsed,
					EMSPagesUsed,
					MainPagesUsed,
					PMNumBlocks;
	long			PMFrameCount;
	PageListStruct	far *PMPages,
					_seg *PMSegPages;

static	char		*ParmStrings[] = {"nomain","noems","noxms",nil};

/////////////////////////////////////////////////////////////////////////////
//
//	EMS Management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_MapEMS() - Maps a logical page to a physical page
//
byte
PML_MapEMS(word logical, byte physical)
{
	byte	err=0, str[160];
	//int	i;

	boolean	errorflag=false;

	__asm {
		mov	ah,EMS_MAPPAGE
		mov	al,physical
		mov	bx,logical
		mov	dx,EMSHandle
		int	EMS_INT
		or	ah,ah
		jnz	errorme
		jmp	Endme
#ifdef __BORLANDC__
	}
#endif
		errorme:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	err,ah
		mov	errorflag,1
#ifdef __BORLANDC__
	}
#endif
		Endme:
#ifdef __WATCOMC__
	}
#endif
	if(errorflag==true)
	{
		strcpy(str,"MM_MapEMS: EMS error ");
		MM_EMSerr(str, err);
		printf("%s\n",str);
		Quit ("PML_MapEMS: Page mapping failed");
		return err;
	}
	return 0;
}

//
//	PML_StartupEMS() - Sets up EMS for Page Mgr's use
//		Checks to see if EMS driver is present
//      Verifies that EMS hardware is present
//		Make sure that EMS version is 3.2 or later
//		If there's more than our minimum (2 pages) available, allocate it (up
//			to the maximum we need)
//
//	Please call MM_CheckForEMS() before calling this function.
//	MM_CheckForEMS is not local despite the name wwww.
//

boolean
PML_StartupEMS(void)
{
	int		i;
#ifdef __PM__NOHOGEMS__
	long	size;
#endif
	byte	err=0, str[64];

	boolean errorflag=false;
	totalEMSpages = freeEMSpages = EMSPageFrame = EMSHandle = EMSAvail = EMSVer = 0;	// set all to 0~
	EMSPresent = false;			// Assume that we'll fail
	EMSAvail = mminfo.EMSmem = 0;

	__asm {
		//MM_CheckForEMS() takes care of what the code did here
		mov	ah,EMS_STATUS
		int	EMS_INT
		jc	error1			// make sure EMS hardware is present

		mov	ah,EMS_VERSION
		int	EMS_INT			// only work on EMS 3.2 or greater (silly, but...)
		or	ah,ah
		jnz	error1
		mov	[EMSVer],ax		// set EMSVer
		cmp	al,0x32			// only work on ems 3.2 or greater
		jb	error1

		mov	ah,EMS_GETFRAME
		int	EMS_INT			// find the page frame address
		or	ah,ah
		jnz	error1
		mov	[EMSPageFrame],bx

		mov	ah,EMS_GETPAGES
		int	EMS_INT			// find out how much EMS is there
		or	ah,ah
		jnz	error1
		or	bx,bx
		jz	noEMS			// no EMS at all to allocate
		cmp	bx,2
		jl	noEMS			// Require at least 2 pages (32k)
		mov	[totalEMSpages],dx
		mov	[freeEMSpages],bx
		mov	[EMSAvail],bx
		jmp	End1
#ifdef __BORLANDC__
	}
#endif
	error1:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	err,ah
		mov	errorflag,1
		jmp	End1
#ifdef __BORLANDC__
	}
#endif
noEMS:
End1:
#ifdef __WATCOMC__
	}
#endif
#ifdef __PM__NOHOGEMS__
	if(errorflag==false)
	{
		// Don't hog all available EMS
		size = EMSAvail * (long)EMSPageSize;
		if (size - (EMSPageSize * 2) > (ChunksInFile * (long)PMPageSize))
		{
			size = (ChunksInFile * (long)PMPageSize) + EMSPageSize;
			EMSAvail = size / EMSPageSize;
		}
	}
#endif
	__asm {
		mov	ah,EMS_ALLOCPAGES
		mov	bx,[EMSAvail];
		int	EMS_INT
		or	ah,ah
		jnz	error2
		mov	[EMSHandle],dx
		jmp	End2
#ifdef __BORLANDC__
	}
#endif
	error2:
#ifdef __BORLANDC__
	__asm {
#endif
		mov	err,ah
		mov	errorflag,1
		jmp	End2
#ifdef __BORLANDC__
	}//end of assembly
#endif
End2:
#ifdef __WATCOMC__
	}//end of assembly
#endif
	if(errorflag==true)
	{
		strcpy(str,"PML_StartupEMS: EMS error ");
		MM_EMSerr(str, err);
		printf("%s\n",str);
		return(EMSPresent);
	}
	mminfo.EMSmem = EMSAvail * (dword)EMSPageSize;

	// Initialize EMS mapping cache
	for (i = 0;i < EMSFrameCount;i++)
		EMSList[i].baseEMSPage = -1;

	EMSPresent = true;			// We have EMS

	return(EMSPresent);
}

//
//	PML_ShutdownEMS() - If EMS was used, deallocate it
//
void
PML_ShutdownEMS(void)
{
	byte err=0, str[64];
	boolean errorflag=false;

	if (EMSPresent)
	{
		__asm {
			mov	ah,EMS_FREEPAGES
			mov	dx,[EMSHandle]
			int	EMS_INT
			jc	errores
			jmp	Endes
#ifdef __BORLANDC__
		}
#endif
			errores:
#ifdef __BORLANDC__
		__asm {
#endif
			mov	err,ah
			mov	errorflag,1
			jmp	Endes
#ifdef __BORLANDC__
		}
#endif
			Endes:
#ifdef __WATCOMC__
		}
#endif
		if(errorflag==true)
		{
			strcpy(str,"PML_ShutdownEMS: Error freeing EMS ");
			MM_EMSerr(str, err);
			printf("%s\n",str);
			Quit ("PML_ShutdownEMS: Error freeing EMS");
		}
	}
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
//TODO: translate the _REG into working assembly
//#define STARTUPXMSASM
	byte err;
	//word XMSAvail, XMSHandle, XMSVer;
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
		//printf("	%u", XMSHandle);
		printf("	err=%02X	e=%u\n", err, e);
#endif
	}
	return(XMSPresent);
}

//
//	PML_XMSCopy() - Copies a main/EMS page to or from XMS
//		Will round an odd-length request up to the next even value
//
void
PML_XMSCopy(boolean toxms,byte far *addr,word xmspage,word length)
{
	dword	xoffset;
	struct
	{
		dword	length;
		word		source_handle;
		dword	source_offset;
		word		target_handle;
		dword	target_offset;
	} copy;

	if (!addr)
		Quit("PML_XMSCopy: zero address");

	xoffset = (dword)xmspage * PMPageSize;

	copy.length = (length + 1) & ~1;
	copy.source_handle = toxms? 0 : XMSHandle;
	copy.source_offset = toxms? (long)addr : xoffset;
	copy.target_handle = toxms? XMSHandle : 0;
	copy.target_offset = toxms? xoffset : (long)addr;

	__asm {
		push si
	}
	_SI = (word)&copy;
	XMS_CALL(XMS_MOVE);
	__asm {
		pop	si
	}
	if (!_AX)
		Quit ("PML_XMSCopy: Error on copy");
}

#if 1
#define	PML_CopyToXMS(s,t,l)	PML_XMSCopy(true,(s),(t),(l))
#define	PML_CopyFromXMS(t,s,l)	PML_XMSCopy(false,(t),(s),(l))
#else
//
//	PML_CopyToXMS() - Copies the specified number of bytes from the real mode
//		segment address to the specified XMS page
//
void
PML_CopyToXMS(byte far *source,int targetpage,word length)
{
	PML_XMSCopy(true,source,targetpage,length);
}

//
//	PML_CopyFromXMS() - Copies the specified number of bytes from an XMS
//		page to the specified real mode address
//
void
PML_CopyFromXMS(byte far *target,int sourcepage,word length)
{
	PML_XMSCopy(false,target,sourcepage,length);
}
#endif

//
//	PML_ShutdownXMS()
//
void
PML_ShutdownXMS(void)
{
	boolean errorflag=false;
	if (XMSPresent)
	{
		__asm {
			mov	dx,[XMSHandle]
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
			//mov	err,ah
			mov	errorflag,1
			jmp	Endxs
#ifdef __BORLANDC__
		}
#endif
			Endxs:
#ifdef __WATCOMC__
		}
#endif
		if(errorflag==true)
			Quit ("PML_ShutdownXMS: Error freeing XMS");
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Main memory code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_SetMainMemPurge() - Sets the purge level for all allocated main memory
//		blocks. This shouldn't be called directly - the PM_LockMainMem() and
//		PM_UnlockMainMem() macros should be used instead.
//
void
PM_SetMainMemPurge(int level)
{
	int	i;

	if(MainPresent)
	for (i = 0;i < PMMaxMainMem;i++)
	{
#ifdef __DEBUG_PM__
#ifdef __DEBUG_PM_MAIN__
		printf("PM_SetMainMemPurge()	info of MainMemPages[i]\n");
		printf("&	%Fp,	%Fp\n", &MainMemPages[i],	&MainMemPages[i]);
#endif
#endif
		if (MainMemPages[i])
			MM_SetPurge(&MainMemPages[i],level);
	}

	else
		Quit ("MainPresent IS NULL");
}

//
//	PM_CheckMainMem() - If something besides the Page Mgr makes requests of
//		the Memory Mgr, some of the Page Mgr's blocks may have been purged,
//		so this function runs through the block list and checks to see if
//		any of the blocks have been purged. If so, it marks the corresponding
//		page as purged & unlocked, then goes through the block list and
//		tries to reallocate any blocks that have been purged.
//	This routine now calls PM_LockMainMem() to make sure that any allocation
//		attempts made during the block reallocation sweep don't purge any
//		of the other blocks. Because PM_LockMainMem() is called,
//		PM_UnlockMainMem() needs to be called before any other part of the
//		program makes allocation requests of the Memory Mgr.
//
void
PM_CheckMainMem(void)
{
	boolean			allocfailed;
	int				i,n;
	memptr			*p;
	PMBlockAttr		*used;
	PageListStruct	far *page;

	if (!MainPresent)
		return;

	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
	{
		n = page->mainPage;
		if (n != -1)						// Is the page using main memory?
		{
			if (!MainMemPages[n])			// Yep, was the block purged?
			{
				page->mainPage = -1;		// Yes, mark page as purged & unlocked
				page->locked = pml_Unlocked;
			}
		}
	}

	// Prevent allocation attempts from purging any of our other blocks
	PM_LockMainMem();
	allocfailed = false;
	for (i = 0,p = MainMemPages,used = MainMemUsed; i < PMMaxMainMem;i++,p++,used++)
	{
		if (!*p)							// If the page got purged
		{
			if (*used & pmba_Allocated)		// If it was allocated
			{
				*used &= ~pmba_Allocated;	// Mark as unallocated
				MainPagesAvail--;			// and decrease available count
			}

			if (*used & pmba_Used)			// If it was used
			{
				*used &= ~pmba_Used;		// Mark as unused
				MainPagesUsed--;			// and decrease used count
			}

			if (!allocfailed)
			{
				MM_BombOnError(false);
				MM_GetPtr(p,PMPageSize);		// Try to reallocate
				if (mmerror)					// If it failed,
					allocfailed = true;			//  don't try any more allocations
				else							// If it worked,
				{
					*used |= pmba_Allocated;	// Mark as allocated
					MainPagesAvail++;			// and increase available count
				}
				MM_BombOnError(true);
			}
		}
	}
	if (mmerror)
		mmerror = false;
}

//
//	PML_StartupMainMem() - Allocates as much main memory as is possible for
//		the Page Mgr. The memory is allocated as non-purgeable, so if it's
//		necessary to make requests of the Memory Mgr, PM_UnlockMainMem()
//		needs to be called.
//
void
PML_StartupMainMem(void)
{
	int		i,n;
	memptr	*p;

	MainPagesAvail = 0;
	MainPresent = false;
	MM_BombOnError(false);
	for (i = 0,p = MainMemPages;i < PMMaxMainMem;i++,p++)
	{
		MM_GetPtr(p,PMPageSize);
		if (mmerror)
			break;

		MainPagesAvail++;
		MainMemUsed[i] = pmba_Allocated;
	}
	MM_BombOnError(true);
	if (mmerror)
		mmerror = false;
	if (MainPagesAvail < PMMinMainMem)
		Quit ("PM_SetupMainMem: Not enough main memory");
	MainPresent = true;
}

//
//	PML_ShutdownMainMem() - Frees all of the main memory blocks used by the
//		Page Mgr.
//
void
PML_ShutdownMainMem(void)
{
	int		i;
	memptr	*p;

	// DEBUG - mark pages as unallocated & decrease page count as appropriate
	for (i = 0,p = MainMemPages;i < PMMaxMainMem;i++,p++)
		if (*p)
			MM_FreePtr(p);
}

/////////////////////////////////////////////////////////////////////////////
//
//	File management code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_ReadFromFile() - Reads some data in from the page file
//
void
PML_ReadFromFile(byte far *buf,long offset,word length)
{
	if (!buf)
		Quit ("PML_ReadFromFile: Null pointer");
	if (!offset)
		Quit ("PML_ReadFromFile: Zero offset");
	if (lseek(PageFile,offset,SEEK_SET) != offset)
		Quit ("PML_ReadFromFile: Seek failed");
	if (!CA_FarRead(PageFile,buf,length))
		Quit ("PML_ReadFromFile: Read failed");
}

//
//	PML_OpenPageFile() - Opens the page file and sets up the page info
//
#if 0
void
PML_OpenPageFile(void)
{
	int				i;
	long			size;
	void			_seg *buf;
	dword		far *offsetptr;
	word			far *lengthptr;
	PageListStruct	far *page;

	PageFile = open(PageFileName,O_RDONLY + O_BINARY);
	if (PageFile == -1)
		Quit ("PML_OpenPageFile: Unable to open page file");

	// Read in header variables
	read(PageFile,&ChunksInFile,sizeof(ChunksInFile));
	read(PageFile,&PMSpriteStart,sizeof(PMSpriteStart));
	read(PageFile,&PMSoundStart,sizeof(PMSoundStart));

	// Allocate and clear the page list
	PMNumBlocks = ChunksInFile;
	MM_GetPtr((memptr)&PMSegPages,sizeof(PageListStruct) * PMNumBlocks);
	MM_SetLock((memptr)&PMSegPages,true);
	PMPages = (PageListStruct far *)PMSegPages;
	_fmemset(PMPages,0,sizeof(PageListStruct) * PMNumBlocks);

	// Read in the chunk offsets
	size = sizeof(dword) * ChunksInFile;
	MM_GetPtr(&buf, size);
	if (!CA_FarRead(PageFile,(byte far *)buf,size))
		Quit ("PML_OpenPageFile: Offset read failed");
	offsetptr = (dword far *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->offset = *offsetptr++;
	MM_FreePtr(&buf);

	// Read in the chunk lengths
	size = sizeof(word) * ChunksInFile;
	MM_GetPtr(&buf,size);
	if (!CA_FarRead(PageFile,(byte far *)buf,size))
		Quit ("PML_OpenPageFile: Length read failed");
	lengthptr = (word far *)buf;
	for (i = 0,page = PMPages;i < ChunksInFile;i++,page++)
		page->length = *lengthptr++;
	MM_FreePtr(&buf);
}

//
//  PML_ClosePageFile() - Closes the page file
//
void
PML_ClosePageFile(void)
{
	if (PageFile != -1)
		close(PageFile);
	if (PMSegPages)
	{
		MM_SetLock((memptr)&PMSegPages,false);
		MM_FreePtr((void _seg *)&PMSegPages);
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
//	Allocation, etc., code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PML_GetEMSAddress()
//
// 		Page is in EMS, so figure out which EMS physical page should be used
//  		to map our page in. If normal page, use EMS physical page 3, else
//  		use the physical page specified by the lock type
//
#ifndef __DEBUG_2__
#pragma argsused	// DEBUG - remove lock parameter
memptr
PML_GetEMSAddress(int page,PMLockType lock)
{
	int		i,emspage;
	word	emsoff,emsbase,offset;

	emsoff = page & (PMEMSSubPage - 1);
	emsbase = page - emsoff;

	emspage = -1;
	// See if this page is already mapped in
	for (i = 0;i < EMSFrameCount;i++)
	{
		if (EMSList[i].baseEMSPage == emsbase)
		{
			emspage = i;	// Yep - don't do a redundant remapping
			break;
		}
	}

	// If page isn't already mapped in, find LRU EMS frame, and use it
	if (emspage == -1)
	{
		dword last = LONG_MAX;
		for (i = 0;i < EMSFrameCount;i++)
		{
			if (EMSList[i].lastHit < last)
			{
				emspage = i;
				last = EMSList[i].lastHit;
			}
		}

		EMSList[emspage].baseEMSPage = emsbase;
		PML_MapEMS(page / PMEMSSubPage,emspage);
	}

	if (emspage == -1)
		Quit ("PML_GetEMSAddress: EMS find failed");

	EMSList[emspage].lastHit = PMFrameCount;
	offset = emspage * EMSPageSizeSeg;
	offset += emsoff * PMPageSizeSeg;
	return((memptr)(EMSPageFrame + offset));
}
#else
memptr
PML_GetEMSAddress(int page,PMLockType lock)
{
	word	emspage;

	emspage = (lock < pml_EMSLock)? 3 : (lock - pml_EMSLock);

	PML_MapEMS(page / PMEMSSubPage,emspage);

	return((memptr)(EMSPageFrame + (emspage * EMSPageSizeSeg)
			+ ((page & (PMEMSSubPage - 1)) * PMPageSizeSeg)));
}
#endif

//
//	PM_GetPageAddress() - Returns the address of a given page
//		Maps in EMS if necessary
//		Returns nil if block isn't cached into Main Memory or EMS
//
//
memptr
PM_GetPageAddress(int pagenum)
{
	PageListStruct	far *page;

	page = &PMPages[pagenum];
	if (page->mainPage != -1)
		return(MainMemPages[page->mainPage]);
	else if (page->emsPage != -1)
		return(PML_GetEMSAddress(page->emsPage,page->locked));
	else
		return(nil);
}

//
//	PML_GiveLRUPage() - Returns the page # of the least recently used
//		present & unlocked main/EMS page (or main page if mainonly is true)
//
int
PML_GiveLRUPage(boolean mainonly)
{
	int				i,lru;
	long			last;
	PageListStruct	far *page;

	for (i = 0,page = PMPages,lru = -1,last = LONG_MAX;i < ChunksInFile;i++,page++)
	{
		if
		(
			(page->lastHit < last)
		&&	((page->emsPage != -1) || (page->mainPage != -1))
		&& 	(page->locked == pml_Unlocked)
		&&	(!(mainonly && (page->mainPage == -1)))
		)
		{
			last = page->lastHit;
			lru = i;
		}
	}

	if (lru == -1)
		Quit ("PML_GiveLRUPage: LRU Search failed");
	return(lru);
}

//
//	PML_GiveLRUXMSPage() - Returns the page # of the least recently used
//		(and present) XMS page.
//	This routine won't return the XMS page protected (by XMSProtectPage)
//
int
PML_GiveLRUXMSPage(void)
{
	int				i,lru;
	long			last;
	PageListStruct	far *page;

	for (i = 0,page = PMPages,lru = -1,last = LONG_MAX;i < ChunksInFile;i++,page++)
	{
		if
		(
			(page->xmsPage != -1)
		&&	(page->lastHit < last)
		&&	(i != XMSProtectPage)
		)
		{
			last = page->lastHit;
			lru = i;
		}
	}
	return(lru);
}

//
//	PML_PutPageInXMS() - If page isn't in XMS, find LRU XMS page and replace
//		it with the main/EMS page
//
void
PML_PutPageInXMS(int pagenum)
{
	int				usexms;
	PageListStruct	far *page;

	if (!XMSPresent)
		return;

	page = &PMPages[pagenum];
	if (page->xmsPage != -1)
		return;					// Already in XMS

	if (XMSPagesUsed < XMSPagesAvail)
		page->xmsPage = XMSPagesUsed++;
	else
	{
		usexms = PML_GiveLRUXMSPage();
		if (usexms == -1)
			Quit ("PML_PutPageInXMS: No XMS LRU");
		page->xmsPage = PMPages[usexms].xmsPage;
		PMPages[usexms].xmsPage = -1;
	}
	PML_CopyToXMS(PM_GetPageAddress(pagenum),page->xmsPage,page->length);
}

//
//	PML_TransferPageSpace() - A page is being replaced, so give the new page
//		the old one's address space. Returns the address of the new page.
//
memptr
PML_TransferPageSpace(int orig,int new)
{
	memptr			addr;
	PageListStruct	far *origpage,far *newpage;

	if (orig == new)
		Quit ("PML_TransferPageSpace: Identity replacement");

	origpage = &PMPages[orig];
	newpage = &PMPages[new];

	if (origpage->locked != pml_Unlocked)
		Quit ("PML_TransferPageSpace: Killing locked page");

	if ((origpage->emsPage == -1) && (origpage->mainPage == -1))
		Quit ("PML_TransferPageSpace: Reusing non-existent page");

	// Copy page that's about to be purged into XMS
	PML_PutPageInXMS(orig);

	// Get the address, and force EMS into a physical page if necessary
	addr = PM_GetPageAddress(orig);

	// Steal the address
	newpage->emsPage = origpage->emsPage;
	newpage->mainPage = origpage->mainPage;

	// Mark replaced page as purged
	origpage->mainPage = origpage->emsPage = -1;

	if (!addr)
		Quit ("PML_TransferPageSpace: Zero replacement");

	return(addr);
}

//
//	PML_GetAPageBuffer() - A page buffer is needed. Either get it from the
//		main/EMS free pool, or use PML_GiveLRUPage() to find which page to
//		steal the buffer from. Returns a far pointer to the page buffer, and
//		sets the fields inside the given page structure appropriately.
//		If mainonly is true, free EMS will be ignored, and only main pages
//		will be looked at by PML_GiveLRUPage().
//
byte far *
PML_GetAPageBuffer(int pagenum,boolean mainonly)
{
	byte			far *addr = nil;
	int				i,n;
	PMBlockAttr		*used;
	PageListStruct	far *page;

	page = &PMPages[pagenum];
	if ((EMSPagesUsed < EMSPagesAvail) && !mainonly)
	{
		// There's remaining EMS - use it
		page->emsPage = EMSPagesUsed++;
		addr = PML_GetEMSAddress(page->emsPage,page->locked);
	}
	else if (MainPagesUsed < MainPagesAvail)
	{
		// There's remaining main memory - use it
		for (i = 0,n = -1,used = MainMemUsed;i < PMMaxMainMem;i++,used++)
		{
			if ((*used & pmba_Allocated) && !(*used & pmba_Used))
			{
				n = i;
				*used |= pmba_Used;
				break;
			}
		}
		if (n == -1)
			Quit ("PML_GetPageBuffer: MainPagesAvail lied");
		addr = MainMemPages[n];
		if (!addr)
			Quit ("PML_GetPageBuffer: Purged main block");
		page->mainPage = n;
		MainPagesUsed++;
	}
	else
		addr = PML_TransferPageSpace(PML_GiveLRUPage(mainonly),pagenum);

	if (!addr)
		Quit ("PML_GetPageBuffer: Search failed");
	return(addr);
}

//
//	PML_GetPageFromXMS() - If page is in XMS, find LRU main/EMS page and
//		replace it with the page from XMS. If mainonly is true, will only
//		search for LRU main page.
//	XMSProtectPage is set to the page to be retrieved from XMS, so that if
//		the page from which we're stealing the main/EMS from isn't in XMS,
//		it won't copy over the page that we're trying to get from XMS.
//		(pages that are being purged are copied into XMS, if possible)
//
memptr
PML_GetPageFromXMS(int pagenum,boolean mainonly)
{
	byte			far *checkaddr;
	memptr			addr = nil;
	PageListStruct	far *page;

	page = &PMPages[pagenum];
	if (XMSPresent && (page->xmsPage != -1))
	{
		XMSProtectPage = pagenum;
		checkaddr = PML_GetAPageBuffer(pagenum,mainonly);
		if (FP_OFF(checkaddr))
			Quit ("PML_GetPageFromXMS: Non segment pointer");
		addr = (memptr)FP_SEG(checkaddr);
		PML_CopyFromXMS(addr,page->xmsPage,page->length);
		XMSProtectPage = -1;
	}

	return(addr);
}

//
//	PML_LoadPage() - A page is not in main/EMS memory, and it's not in XMS.
//		Load it into either main or EMS. If mainonly is true, the page will
//		only be loaded into main.
//
void
PML_LoadPage(int pagenum,boolean mainonly)
{
	byte			far *addr;
	PageListStruct	far *page;

	addr = PML_GetAPageBuffer(pagenum,mainonly);
	page = &PMPages[pagenum];
	PML_ReadFromFile(addr,page->offset,page->length);
}

//
//	PM_GetPage() - Returns the address of the page, loading it if necessary
//		First, check if in Main Memory or EMS
//		Then, check XMS
//		If not in XMS, load into Main Memory or EMS
//
#pragma warn -pia
memptr
PM_GetPage(int pagenum)
{
	memptr	result;

	if (pagenum >= ChunksInFile)
		Quit ("PM_GetPage: Invalid page request");

#ifdef __DEBUG_2__	// for debugging
	__asm {
		mov	dx,STATUS_REGISTER_1
		in	al,dx
		mov	dx,ATR_INDEX
		mov	al,ATR_OVERSCAN
		out	dx,al
		mov	al,10	// bright green
		out	dx,al
	}
#endif

	if (!(result = PM_GetPageAddress(pagenum)))
	{
		boolean mainonly = (pagenum >= PMSoundStart);
if (!PMPages[pagenum].offset)	// JDC: sparse page
	Quit ("Tried to load a sparse page!");
		if (!(result = PML_GetPageFromXMS(pagenum,mainonly)))
		{
			if (PMPages[pagenum].lastHit ==  PMFrameCount)
				PMThrashing++;

			PML_LoadPage(pagenum,mainonly);
			result = PM_GetPageAddress(pagenum);
		}
	}
	PMPages[pagenum].lastHit =  PMFrameCount;

#ifdef __DEBUG_2__	// for debugging
	__asm{
		mov	dx,STATUS_REGISTER_1
		in	al,dx
		mov	dx,ATR_INDEX
		mov	al,ATR_OVERSCAN
		out	dx,al
		mov	al,3	// blue
		out	dx,al
		mov	al,0x20	// normal
		out	dx,al
	}
#endif

	return(result);
}
#pragma warn +pia

//
//	PM_SetPageLock() - Sets the lock type on a given page
//		pml_Unlocked: Normal, page can be purged
//		pml_Locked: Cannot be purged
//		pml_EMS?: Same as pml_Locked, but if in EMS, use the physical page
//					specified when returning the address. For sound stuff.
//
void
PM_SetPageLock(int pagenum,PMLockType lock)
{
	if (pagenum < PMSoundStart)
		Quit ("PM_SetPageLock: Locking/unlocking non-sound page");

	PMPages[pagenum].locked = lock;
}

//
//	PM_Preload() - Loads as many pages as possible into all types of memory.
//		Calls the update function after each load, indicating the current
//		page, and the total pages that need to be loaded (for thermometer).
//
void
PM_Preload(boolean (*update)(word current,word total))
{
	int				i,j,
					page,oogypage;
	word			current,total,
					totalnonxms,totalxms,
					mainfree,maintotal,
					emsfree,emstotal,
					xmsfree,xmstotal;
	memptr			addr;
	PageListStruct	__far *p;

	mainfree = (MainPagesAvail - MainPagesUsed) + (EMSPagesAvail - EMSPagesUsed);
	xmsfree = (XMSPagesAvail - XMSPagesUsed);

	xmstotal = maintotal = 0;

	for (i = 0;i < ChunksInFile;i++)
	{
		if (!PMPages[i].offset)
			continue;			// sparse

		if ( PMPages[i].emsPage != -1 || PMPages[i].mainPage != -1 )
			continue;			// already in main mem

		if ( mainfree )
		{
			maintotal++;
			mainfree--;
		}
		else if ( xmsfree && (PMPages[i].xmsPage == -1) )
		{
			xmstotal++;
			xmsfree--;
		}
	}


	total = maintotal + xmstotal;

	if (!total)
		return;

	page = 0;
	current = 0;

//
// cache main/ems blocks
//
	while (maintotal)
	{
		while ( !PMPages[page].offset || PMPages[page].mainPage != -1
			||	PMPages[page].emsPage != -1 )
			page++;

		if (page >= ChunksInFile)
			Quit ("PM_Preload: Pages>=ChunksInFile");

		PM_GetPage(page);

		page++;
		current++;
		maintotal--;
		update(current,total);
	}

//
// load stuff to XMS
//
	if (xmstotal)
	{
		for (oogypage = 0 ; PMPages[oogypage].mainPage == -1 ; oogypage++)
		;
		addr = PM_GetPage(oogypage);
		if (!addr)
			Quit ("PM_Preload: XMS buffer failed");

		while (xmstotal)
		{
			while ( !PMPages[page].offset || PMPages[page].xmsPage != -1 )
				page++;

			if (page >= ChunksInFile)
				Quit ("PM_Preload: Pages>=ChunksInFile");

			p = &PMPages[page];

			p->xmsPage = XMSPagesUsed++;
			if (XMSPagesUsed > XMSPagesAvail)
				Quit ("PM_Preload: Exceeded XMS pages");
			if (p->length > PMPageSize)
				Quit ("PM_Preload: Page too long");

			PML_ReadFromFile((byte far *)addr,p->offset,p->length);
			PML_CopyToXMS((byte far *)addr,p->xmsPage,p->length);

			page++;
			current++;
			xmstotal--;
			update(current,total);
		}

		p = &PMPages[oogypage];
		PML_ReadFromFile((byte far *)addr,p->offset,p->length);
	}

	update(total,total);
}

/////////////////////////////////////////////////////////////////////////////
//
//	General code
//
/////////////////////////////////////////////////////////////////////////////

//
//	PM_NextFrame() - Increments the frame counter and adjusts the thrash
//		avoidence variables
//
//		If currently in panic mode (to avoid thrashing), check to see if the
//			appropriate number of frames have passed since the last time that
//			we would have thrashed. If so, take us out of panic mode.
//
//
void
PM_NextFrame(void)
{
	int	i;

	// Frame count overrun - kill the LRU hit entries & reset frame count
	if (++PMFrameCount >= LONG_MAX - 4)
	{
		for (i = 0;i < PMNumBlocks;i++)
			PMPages[i].lastHit = 0;
		PMFrameCount = 0;
	}

//#if 0
	for (i = 0;i < PMSoundStart;i++)
	{
		if (PMPages[i].locked)
		{
			char buf[40];
			sprintf(buf,"PM_NextFrame: Page %d is locked",i);
			Quit (buf);
		}
	}
//#endif

	if (PMPanicMode)
	{
		// DEBUG - set border color
		if ((!PMThrashing) && (!--PMPanicMode))
		{
			// DEBUG - reset border color
		}
	}
	if (PMThrashing >= PMThrashThreshold)
		PMPanicMode = PMUnThrashThreshold;
	PMThrashing = false;
}

//
//	PM_Reset() - Sets up caching structures
//
void
PM_Reset(void)
{
	int				i;
	PageListStruct	far *page;

	XMSPagesAvail = XMSAvail / PMPageSizeKB;

	EMSPagesAvail = EMSAvail * (EMSPageSizeKB / PMPageSizeKB);
	EMSPhysicalPage = 0;

	MainPagesUsed = EMSPagesUsed = XMSPagesUsed = 0;

	PMPanicMode = false;

	PageFile = -1;
	XMSProtectPage = -1;

	// Initialize page list
	for (i = 0,page = PMPages;i < PMNumBlocks;i++,page++)
	{
		page->mainPage = -1;
		page->emsPage = -1;
		page->xmsPage = -1;
		page->locked = false;
	}
}

//
//	PM_Startup() - Start up the Page Mgr
//
void
PM_Startup(void)
{
	boolean	nomain,noems,noxms;
	int		i;

	if (PMStarted)
		return;

	//0000+=+=strcpy(&PageFileName, "VSWAP.");

	nomain = noems = noxms = false;
	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],ParmStrings))
		{
		case 0:
			nomain = true;
			break;
		case 1:
			noems = true;
			break;
		case 2:
			noxms = true;
			break;
		}
	}

	//0000+=+=PML_OpenPageFile();

	if (!noems && MML_CheckForEMS())
		PML_StartupEMS();
	if (!noxms && MML_CheckForXMS())
		PML_StartupXMS();
	if(!nomain)
		PML_StartupMainMem();

	if (!MainPresent && !EMSPresent && !XMSPresent)
		Quit ("PM_Startup: No main or EMS");

	PM_Reset();

	PMStarted = true;
}

//
//	PM_Shutdown() - Shut down the Page Mgr
//
void
PM_Shutdown(void)
{
	if(MML_CheckForXMS()) PML_ShutdownXMS();
	if(MML_CheckForEMS()) PML_ShutdownEMS();

	if (!PMStarted)
		return;

	//0000+=+=PML_ClosePageFile();

	PML_ShutdownMainMem();
}
