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

#ifndef __16_PM__
#define __16_PM__

//
//	ID_PM.H
//	Header file for Id Engine's Page Manager
//

#include "src/lib/16_head.h"
#include "src/lib/16_hc.h"
#include "src/lib/16_mm.h"
#include "src/lib/16_ca.h"
#include <dos.h>

//#define __PM__NOHOGEMS__
//	NOTE! PMPageSize must be an even divisor of EMSPageSize, and >= 1024
#define	EMSPageSize		16384
#define	EMSPageSizeSeg	(EMSPageSize >> 4)
#define	EMSPageSizeKB	(EMSPageSize >> 10)
#define	EMSFrameCount	4
#define	PMPageSize		4096
#define	PMPageSizeSeg	(PMPageSize >> 4)
#define	PMPageSizeKB	(PMPageSize >> 10)
#define	PMEMSSubPage	(EMSPageSize / PMPageSize)

#define	PMMinMainMem	10			// Min acceptable # of pages from main
#define	PMMaxMainMem	100			// Max number of pages in main memory

#define	PMThrashThreshold	1	// Number of page thrashes before panic mode
#define	PMUnThrashThreshold	5	// Number of non-thrashing frames before leaving panic mode

typedef	enum
		{
			pml_Unlocked,
			pml_Locked
		} PMLockType;

typedef	enum
		{
			pmba_Unused = 0,
			pmba_Used = 1,
			pmba_Allocated = 2
		} PMBlockAttr;

typedef	struct
		{
			dword	offset;		// Offset of chunk into file
			word		length;		// Length of the chunk

			int			xmsPage;	// If in XMS, (xmsPage * PMPageSize) gives offset into XMS handle

			PMLockType	locked;		// If set, this page can't be purged
			int			emsPage;	// If in EMS, logical page/offset into page
			int			mainPage;	// If in Main, index into handle array

			dword	lastHit;	// Last frame number of hit
		} PageListStruct;

typedef	struct
		{
			int			baseEMSPage;	// Base EMS page for this phys frame
			dword	lastHit;		// Last frame number of hit
		} EMSListStruct;

extern	boolean			XMSPresent,EMSPresent,MainPresent;;
extern	word			XMSPagesAvail,EMSPagesAvail,EMSVer,EMSPageFrame,XMSVer,XMSHandle;
extern	longword			XMSDriver;

extern	word			ChunksInFile,
						PMSpriteStart,PMSoundStart;
extern	PageListStruct	far *PMPages;

#define	PM_GetSoundPage(v)	PM_GetPage(PMSoundStart + (v))
#define	PM_GetSpritePage(v)	PM_GetPage(PMSpriteStart + (v))

#define	PM_LockMainMem()	PM_SetMainMemPurge(0)
#define	PM_UnlockMainMem()	PM_SetMainMemPurge(3)


extern	char	PageFileName[13];


extern	void	PM_Startup(void),
				PM_Shutdown(void),
				PM_Reset(void),
				PM_Preload(boolean (*update)(word current,word total)),
				PM_NextFrame(void),
				PM_SetPageLock(int pagenum,PMLockType lock),
				PM_SetMainPurge(int level),
				PM_CheckMainMem(void);
extern	memptr	PM_GetPageAddress(int pagenum),
				PM_GetPage(int pagenum);		// Use this one to cache page

void PM_SetMainMemPurge(int level);
void PML_StartupMainMem(void);
#endif
