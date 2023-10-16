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

// ID_MM.H

#ifndef __16_MM__
#define __16_MM__

#include <string.h>
#include "src/lib/16_head.h"
#include "src/lib/16_hc.h"
#include "src/lib/16_tail.h"

#ifdef __DEBUG__		// 1 == Debug/Dev  ;  0 == Production/final
#define OUT_OF_MEM_MSG	"MM_GetPtr: Out of memory!\nYou were short :%lu bytes\n"
#else
#define OUT_OF_MEM_MSG	"\n"
#endif

#define SAVENEARHEAP	0x400		// space to leave in data segment
#define SAVEFARHEAP	0		// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

#define MAXBLOCKS		1024


//--------

#define	EMS_INT			0x67
#define	EMM_INT			0x21

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_MAPXPAGE		0x50
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

//--------

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

//==========================================================================

typedef void _seg * memptr;

typedef struct
{
	dword	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

//==========================================================================

extern	mminfotype	mminfo;
extern	memptr		bufferseg;
extern	boolean		mmerror;
extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);
#ifdef __16_PM__
extern	word		totalEMSpages,freeEMSpages,EMSpageframe,EMSpagesmapped,EMShandle;
#endif

//==========================================================================

boolean MM_CheckForEMS(void);
boolean MM_CheckForXMS(void);

void MML_UseSpace (unsigned segstart, unsigned seglength);
void MML_ClearBlock (void);

void MM_Reset (void);
void MM_Startup (void);
void MM_Shutdown (void);

void MM_GetPtr (memptr *baseptr,dword size);
void MM_FreePtr (memptr *baseptr);
void MM_SetPurge (memptr *baseptr, int purge);
void MM_SetLock (memptr *baseptr, boolean locked);
void MM_SortMem (void);
void MM_ShowMemory (void);

void MM_DumpData (void);
dword MM_UnusedMemory (void);
dword MM_TotalFree (void);
void MM_Report_ (void);
void MM_EMSerr (byte *stri, byte err);
void MM_BombOnError (boolean bomb);
//void MM_GetNewBlock(mminfo_t *mm);
//void MM_FreeBlock(mmblocktype *x, mminfo_t *mm);
void xms_call (byte v);

void MML_UseSpace (unsigned segstart, unsigned seglength);

#endif
