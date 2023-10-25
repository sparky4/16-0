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

#include "src/lib/bakapee.h"

//static word far* clockw= (word far*) 0x046C; /* 18.2hz clock */
//char global_temp_status_text[512];
//char global_temp_status_text2[512];

//==========================================================================

// clrstdin() clear any leftover chars tha may be in stdin stream //
/*void clrstdin()
{
   int ch = 0;
   while( ( ch = getchar() ) != '\n' && ch != EOF );
}*/

//===========================================================================

//color ‚Ä‚·‚Æ
void colortest(page_t *page, bakapee_t *pee)
{
	//if(pee->color < 256)
	//{
//		modexcls(page, pee->color, VGA);
		VL_ClearVideo (pee->color);
		pee->color++;
	//}else pee->color = 0;
}

//color ‚Ä‚·‚Æ
void colorz(page_t *page, bakapee_t *pee)
{
	if(pee->color <= pee->hgq)
	{
//		modexcls(page, pee->color, VGA);
		VL_ClearVideo (pee->color);
		pee->color++;
	}else pee->color = pee->lgq;
}

//slow spectrum down
void ssd(page_t *page, bakapee_t *pee, word svq)
{
	if(pee->sy < page->sh+1)
	{
		if(pee->sx < page->sw+1)
		{
			//mxPutPixel(sx, sy, color);
			//printf("%d %d %d %d\n", pee->sx, pee->sy, svq, pee->color);
			dingpp(page, pee);
			pee->sx++;
		}else pee->sx = 0;
		if(pee->sx == page->sw)
		{
			pee->sy++;
			if(svq == 7) pee->color++;
			if(pee->sy == page->sh && svq == 8) pee->color = rand()%256;
		}
	}else pee->sy = 0;
}

//plot pixel or plot tile
void dingpp(page_t *page, bakapee_t *pee)
{
	if(pee->tile)
	{
		VL_ClearRegion(page, pee->xx, pee->yy, TILEWH, TILEWH, pee->color);
	}
	else
		VL_Plot (pee->xx, pee->yy, pee->color);
		//modexputPixel(page, pee->xx, pee->yy, pee->color);
}

// appears to be the limitator
// jumps pixel to other side of screen
void keeppixelonscreen(page_t *page, bakapee_t *pee)
{
	if(pee->tile)
	{
		if(pee->xx<TILEWH) pee->xx=(page->sw);
		if(pee->yy<TILEWH) pee->yy=(page->sh);
		if(pee->xx>(page->sw)) pee->xx=TILEWH;
		if(pee->yy>(page->sh)/*+(TILEWH*BUFFMX)*/) pee->yy=TILEWH;
	}
		else
	{
		if(pee->xx<TILEWH) pee->xx=page->sw+TILEWH;
		if(pee->yy<TILEWH) pee->yy=page->sh+TILEWH;
		if(pee->xx>page->sw+TILEWH) pee->xx=TILEWH;
		if(pee->yy>page->sh+TILEWH) pee->yy=TILEWH;
	}
}

//assigning values from randomizer
void dingas(bakapee_t *pee)
{
	if(pee->gq == pee->keepcurrentcolor) dingu(pee);
	if(!pee->bakax)
	{
		if(pee->tile)
		pee->xx-=TILEWH;
		else pee->xx--;
	}
	else if(pee->bakax>1)
	{
		if(pee->tile)
		pee->xx+=TILEWH;
		else pee->xx++;
	}
	if(!pee->bakay)
	{
		if(pee->tile)
		pee->yy-=TILEWH;
		else pee->yy--;
	}
	else if(pee->bakay>1)
	{
		if(pee->tile)
		pee->yy+=TILEWH;
		else pee->yy++;
	}
}

void dingaso(bakapee_t *pee)
{
	if(pee->gq == pee->keepcurrentcolor) dingu(pee);
	if(!pee->bakax)
	{
		if(pee->tile)
		pee->xx-=TILEWH;
		else pee->xx--;
	}
	else
	{
		if(pee->tile)
		pee->xx+=TILEWH;
		else pee->xx++;
	}
	if(!pee->bakay)
	{
		if(pee->tile)
		pee->yy-=TILEWH;
		else pee->yy--;
	}
	else
	{
		if(pee->tile)
		pee->yy+=TILEWH;
		else pee->yy++;
	}
}

//incremention of the color
void dingu(bakapee_t *pee)
{
	if(pee->color < pee->hgq && pee->color < pee->lgq) pee->color = pee->lgq;
	if(pee->color < pee->hgq)
	{
		pee->color++;
	}else{
		pee->color = pee->lgq;
	}
}

//randomizer
void dingq(bakapee_t *pee)
{
	if(pee->gq<pee->keepcurrentcolor)
	{
		pee->gq++;
	}
	else
	{
		dingu(pee);
		pee->gq = 0;
	}
	pee->bakax = rand()%3; pee->bakay = rand()%3;
}

void dingqo(bakapee_t *pee)
{
	if(pee->gq<pee->keepcurrentcolor)
	{
		pee->gq++;
		pee->bakax = rand()%3; pee->bakay = rand()%3;
	}
	else
	{
		dingu(pee);
		pee->gq = 0;
	}
	//either one will do wwww --4
	pee->bakax = rand()&0x1; pee->bakay = rand()&0x1;
	//pee->bakax = rand()%2; pee->bakay = rand()%2;
}

/*-----------ding-------------*/
void ding(page_t *page, bakapee_t *pee, word q)
{
	word tx=0,ty=0;//d3y,

//++++  if(q <= 4 && q!=2 && gq == pee->keepcurrentcolor-1) color = rand()%pee->hgq;
	switch(q)
	{
		case 1:/*
			dingq(pee);
			if(pee->xx==page->sw){pee->bakax=0;}
			if(pee->xx==0){pee->bakax=1;}
			if(pee->yy==page->sh){pee->bakay=0;}
			if(pee->yy==0){pee->bakay=1;}*/
			dingqo(pee);
			dingaso(pee);
			keeppixelonscreen(page, pee);
			dingpp(page, pee);	//plot the pixel/tile
			if(pee->tile)
			VL_ClearRegion(page, (rand()*TILEWH)%page->width, (rand()*TILEWH)%(page->height), TILEWH, TILEWH, 0);
			else
				VL_Plot (rand()%page->width, rand()%page->height, 0);
			//modexputPixel(page, rand()%page->width, rand()%page->height, 0);
		break;
		case 2:
			dingq(pee);
			dingas(pee);
			keeppixelonscreen(page, pee);
			dingpp(page, pee);	//plot the pixel/tile
			if(pee->tile)
			VL_ClearRegion(page, (rand()*TILEWH)%page->width, (rand()*TILEWH)%(page->height), TILEWH, TILEWH, 0);
			else
				VL_Plot (rand()%page->width, rand()%page->height, 0);
			//modexputPixel(page, rand()%page->width, rand()%page->height, 0);
		break;
		case 3:
			/*dingq(pee);
			if(pee->xx!=page->sw||pee->yy!=page->sh)
			{
				if(pee->xx==0){pee->bakax=1;pee->bakay=-1;d3y=1;}
				if(pee->yy==0){pee->bakax=1;pee->bakay=0;d3y=1;}
				if(pee->xx==page->sw){pee->bakax=-1;pee->bakay=-1;d3y=1;}
				if(pee->yy==page->sh){pee->bakax=1;pee->bakay=0;d3y=1;}
			}else if(pee->xx==page->sw&&pee->yy==page->sh) pee->xx=pee->yy=0;
			if(d3y)
			{
				if(pee->bakay<0)
				{
					pee->yy--;
					d3y--;
				}else
				if(pee->bakay>0)
				{
					pee->yy++;
					d3y--;
				}
			}
			if(pee->bakax<0)
			{
				pee->xx--;
			}else
			if(pee->bakax>0)
			{
				pee->xx++;
			}
			dingpp(page, pee);	//plot the pixel/tile*/
			dingqo(pee);
			dingaso(pee);
			keeppixelonscreen(page, pee);
			dingpp(page, pee);	//plot the pixel/tile
		break;
		case 4:
			dingq(pee);
			dingas(pee);
			keeppixelonscreen(page, pee);
			dingpp(page, pee);	//plot the pixel/tile
		break;
		case 5:
			colortest(page, pee);
		break;
		case 6:
// 			pee->color = rand()%256;
// 			modexcls(page, pee->color, VGA);
			colorz(page, pee);
//			modexprint(page, page->sw/2, page->sh/2, 1, 0, 47, 0, 1, "bakapi");
		break;
		case 7:
			if(pee->color <= pee->hgq)
			{
				ssd(page, pee, q);
				pee->color++;
			}else pee->color = pee->lgq;
		break;
		case 8:
			colorz(page, pee);
//			modexprint(page, page->sw/2, page->sh/2, 1, 0, 47, 0, 1, "bakapi");
		break;
/*		case 9:
			VL_ClearRegion(&(ggvv->video.page[0]), 0, 0, ggvv->video.page[0].width/2, ggvv->video.page[0].height/2, 15);
#ifdef BAKAFIZZUNSIGNED
//			baka_FizzleFade (ggvv->video.ofs.bufferofs, ggvv->video.ofs.displayofs, vga_state.vga_width, vga_state.vga_height, 70, true, ggvv);
			baka_FizzleFade (ggvv->video.ofs.bufferofs, ggvv->video.ofs.displayofs, ggvv->video.page[0].width, ggvv->video.page[0].height, 70, true, ggvv);
#else
			baka_FizzleFade (&ggvv->video.page[1], &ggvv->video.page[0], vga_state.vga_width, vga_state.vga_height, 70, true, ggvv);
#endif
		break;*/
		case 10:
			ssd(page, pee, q); /*printf("%d\n", pee->color);*/
		break;
		case 11:
			colorz(page, pee); delay(100);
		break;

		case 16:	//interesting effects
			dingq(pee);
			if(!pee->bakax){ pee->xx--;}
			else if(pee->bakax>0){ pee->xx++; }
			if(!pee->bakay){ pee->yy--;}
			else if(pee->bakay>0){ pee->yy++; }
			dingas(pee);
			tx+=pee->xx+TILEWH+4;
			ty+=pee->yy+TILEWH+4;
			VL_ClearRegion(page, tx, ty, 4, 4, pee->color);
			if(pee->tile)
			VL_ClearRegion(page, (rand()*4)%page->width, (rand()*4)%(page->height), 4, 4, 0);
			else
				VL_Plot (rand()%page->width, rand()%page->height, 0);
			//modexputPixel(page, rand()%page->width, rand()%(page->height), 0);
			//printf("%d %d %d %d %d %d\n", pee->xx, pee->yy, tx, ty, TILEWH);
		break;
		default:
		break;
	}
	//printf("	%dx%d	%dx%d\n", pee->xx, pee->yy, tx, ty);
	//pee->color++;
}


/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
	//unsigned		finscreen;
	memptr	screen=0;

	//ClearMemory ();
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

	if (error && *error)
	{
		//movedata((unsigned)screen,7,0xb800,0,7*160);
		gotoxy (10,4);
		fprintf(stderr, "%s\n", error);
		gotoxy (1,8);
		exit(1);
	}
	else
	if (!error || !(*error))
	{
		clrscr();
#ifndef JAPAN
		movedata ((unsigned)screen,7,0xb800,0,4000);
		gotoxy(1,24);
#endif
//asm	mov	bh,0
//asm	mov	dh,23	// row
//asm	mov	dl,0	// collumn
//asm	mov ah,2
//asm	int	0x10
	}

	exit(0);
}
