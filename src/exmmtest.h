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
/*
	exmm test
*/
#ifndef __EXMMTEST_H__
#define __EXMMTEST_H__

#include "src/lib/16_head.h"
#include "src/lib/16_tail.h"
#include "src/lib/16_pm.h"
#include "src/lib/16_ca.h"
#include "src/lib/16_mm.h"
#include "src/lib/16_hc.h"
#include "src/lib/hp_dbg.h"

#pragma hdrstop

#pragma warn -pro
#pragma warn -use

//file load or read definition
#define FILEREADLOAD
#define FILEREAD
#define EXMMVERBOSE
#define BUFFDUMPPAUSE
//#define EXMMVERBOSE__
//
#define PRINTBBDUMP
#define BUFFDUMP

#define KEYP printf("\n\npress any key to continue!\n"); getch();

#define BBUFNAME bigbufferseg
#define INITBBUF 	memptr BBUFNAME;
#define BBUFPTR	MEMPTRCONV BBUFNAME

#ifdef __BORLANDC__
#define BBUF		(memptr *)BBUFPTR
#define BBUFSTRING	(memptr *)BBUFNAME
#endif
#ifdef __WATCOMC__
#define BBUF		BBUFNAME
#define BBUFSTRING	BBUF
#endif


//printf("*	%Fp\t", *BBUF);
//printf("*	     %04x\t", *BBUF);
#define PRINTBB {\
	printf("-------------------------------------------------------------------------------\n");\
	printf("&main()=	%Fp\n", argv[0]);\
	printf("buffer:\n");\
	printf("	%Fp\t", BBUF);\
	printf("&%Fp\n", BBUFPTR);\
	printf("	     %04x\t", BBUF);\
	printf("&     %04x\n", BBUFPTR);\
	printf("-------------------------------------------------------------------------------\n");\
}
	//printf("&main()=	%Fp\n", *argv[0]);
	//printf("bigbuffer=	%Fp\n", bigbuffer);
	//printf("&bigbuffer=	%Fp\n", &bigbuffer);
	//printf("bigbuffer=	%04x\n", bigbuffer);
	//printf("&bigbuffer=	%04x\n", &bigbuffer);

extern void far	*farheap;
extern void near	*nearheap;

#endif /*__EXMMTEST_H__*/
