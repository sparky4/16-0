/* Project 16 Source Code~
 * Copyright (C) 2012-2015 sparky4 & pngwen & andrius4669
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

#include "src/lib/dos_kb.h"
#include "src/lib/mapread.h"
#include "src/lib/wcpu/wcpu.h"
#include "src/lib/planar.h"
//====#include "src\lib\ems.c"

//word far *clock= (word far*) 0x046C; /* 18.2hz clock */

//optimize scroll*!!!!

typedef struct {
	map_t *map;
	page_t *page;
	int tx; //appears to be the top left tile position on the viewable screen map
	int ty; //appears to be the top left tile position on the viewable screen map
	word dxThresh; //????
	word dyThresh; //????
} map_view_t;

typedef struct {
	int x; //player exact position on the viewable map
	int y; //player exact position on the viewable map
	int tx; //player tile position on the viewable map
	int ty; //player tile position on the viewable map
	int triggerx; //player's trigger box tile position on the viewable map
	int triggery; //player's trigger box tile position on the viewable map
	int setx; //NOT USED YET! player sprite sheet set on the image x
	int sety; //NOT USED YET! player sprite sheet set on the image y
	word q; //loop variable
	word d; //direction
	bitmap_t data; //supposively the sprite sheet data
	int hp; //hitpoints of the player
} actor_t;

typedef struct
{
	map_view_t *mv;
} map_view_db_t;

map_t allocMap(int w, int h);
void initMap(map_t *map);
void mapScrollRight(map_view_t *mv, byte offset, word id);
void mapScrollLeft(map_view_t *mv, byte offest, word id);
void mapScrollUp(map_view_t *mv, byte offset, word id);
void mapScrollDown(map_view_t *mv, byte offset, word id);
void mapGoTo(map_view_t *mv, int tx, int ty);
void mapDrawTile(tiles_t *t, word i, page_t *page, word x, word y);
void mapDrawRow(map_view_t *mv, int tx, int ty, word y);
void mapDrawCol(map_view_t *mv, int tx, int ty, word x);
void qclean();
void pdump(map_view_t *pee);
void animatePlayer(map_view_t *src, map_view_t *dest, /*map_view_t *top, */sword d, short scrolloffsetswitch, int x, int y, int ls, int lp, bitmap_t *bmp);

#define TILEWH 16
#define QUADWH			TILEWH/2
#define SPEED 4
//#define LOOPMAX (TILEWH/SPEED)

//place holder definitions
//#define MAPX 200
//#define MAPY 150
//#define TRIGGX 10
//#define TRIGGY 9

void main() {
//++++	size_t oldfreemem=GetFreeSize();
	/*long emmhandle;
	long emsavail;
	char teststr[80];*/
	word panswitch=0, panq=1, pand=0, panpagenum=0; //for panning!
	int i;
	static word paloffset=0;
	bitmap_t ptmp;//, npctmp; // player sprite
	planar_buf_t *p;
	const char *cpus;
	static int persist_aniframe = 0;    /* gonna be increased to 1 before being used, so 0 is ok for default */
	page_t screen, screen2, screen3;
	map_t map;
	map_view_t mv[3];//mv, mv2, mv3;
	map_view_t *bg, *spri, *mask;//, *tmp;
	//map_view_db_t pgid[4];
	byte *dpal, *gpal;
	byte *ptr;
	byte *mappalptr;
	actor_t player;
	//actor_t npc0;

//	atexit(qclean());
	/*if(!emmtest())
	{
		printf("Expanded memory is not present\n");
		exit(0);
	}

	if(!emmok())
	{
		printf("Expanded memory manager is not present\n");
		exit(0);
	}

	emsavail = emmavail();
	if(emsavail == -1)
	{
		printf("Expanded memory manager error\n");
		exit(0);
	}
	printf("There are %ld pages available\n",emsavail);

	if((emmhandle = emmalloc(emsavail)) < 0)
	{
		printf("Insufficient pages available\n");
		exit(0);
	}*/

	/* create the map */
//0000	printf("Total used @ before map load:			%zu\n", oldfreemem-GetFreeSize());
//0000	fprintf(stderr, "testing~\n");
	loadmap("data/test.map", &map);
//0000	fprintf(stderr, "yay map loaded~~\n");
//----	map = allocMap(map.width,map.height); //20x15 is the resolution of the screen you can make maps smaller than 20x15 but the null space needs to be drawn properly
	//if(isEMS()) printf("%d tesuto\n", coretotalEMS());
//----	initMap(&map);
	mv[0].map = &map;
	mv[1].map = &map;
	mv[2].map = &map;

	/* draw the tiles */
	ptr = map.data;
	mappalptr = map.tiles->data->palette;
	/* data */
//0000	printf("Total used @ before image loading:		%zu\n", oldfreemem-GetFreeSize());
	ptmp = bitmapLoadPcx("data/ptmp.pcx"); // load sprite
	//npctmp = bitmapLoadPcx("ptmp1.pcx"); // load sprite

	/* create the planar buffer */
//0000	printf("Total used @ before planar buffer creation:	%zu\n", oldfreemem-GetFreeSize());
	p = planar_buf_from_bitmap(&ptmp);
//0000	printf("Total used @ after planar buffer creation:	%zu\n", oldfreemem-GetFreeSize());

	/*if(isEMS())
	{
		XMOVE mm;
		mm.length=sizeof(map);
		mm.sourceH=0;
		mm.sourceOff=(long)&map;
		mm.destH=emmhandle;
		mm.destOff=1;
		//halp!
		ist = move_emem(&mm);
		printf("%d\n", coretotalEMS());
		if(!ist){ dealloc_emem(emmhandle); exit(5); }
		//printf("%d\n", emmhandle);
	}

	if(isEMS())
	{
		XMOVE mm;
		mm.length=emmhandle;
		mm.sourceH=0;
		mm.sourceOff=(long)&ptmp;
		mm.destH=emmhandle;
		mm.destOff=0;
		//halp!
		ist = move_emem(&mm);
		printf("%d\n", coretotalEMS());
		if(!ist){ dealloc_emem(emmhandle); exit(5); }
		//printf("%d\n", emmhandle);
	}
*/

	/* save the palette */
	dpal = modexNewPal();
	modexPalSave(dpal);
	modexFadeOff(4, dpal);

	setkb(1);
	modexEnter();
	modexPalBlack();	//reset the palette~
//	printf("Total used @ before palette initiation:		%zu\n", oldfreemem-GetFreeSize());
	ptmp.offset=(paloffset/3);
	modexPalUpdate(&ptmp, &paloffset, 0, 0);
	//printf("	%d\n", sizeof(ptmp.data));
	//printf("1:	%d\n", paloffset);
	map.tiles->data->offset=(paloffset/3);
	//XTmodexPalUpdate(map.tiles->data, &paloffset, 0, 0);
	printf("\n====\n");
	printf("0	paloffset=	%d\n", paloffset/3);
	printf("====\n\n");
	gpal = modexNewPal();
	modexPalSave(gpal);
	modexSavePalFile("data/g.pal", gpal);
	modexPalBlack();	//so player will not see loadings~

	/* setup camera and screen~ */
	screen = modexDefaultPage();
	screen.width += (TILEWH*2);
	screen.height += (TILEWH*2);//+QUADWH;
	mv[0].page = &screen;
	screen2 = modexNextPage(mv[0].page);
	mv[1].page = &screen2;
	screen3 = modexNextPage0(mv[1].page, 320, 240);	//(352*176)+1024 is the remaining amount of memory left wwww
	//screen3 = modexNextPage0(mv2.page, 320, 192);	//(352*176)+1024 is the remaining amount of memory left wwww
	mv[2].page = &screen3;

	/* set up paging */
	bg = &mv[0];
	spri = &mv[1];
	mask = &mv[2];

//TODO: LOAD map data and position the map in the middle of the screen if smaller then screen
	mapGoTo(bg, 0, 0);
	mapGoTo(spri, 0, 0);
	//mapGoTo(mask, 0, 0);

	//TODO: put player in starting position of spot
	//default player position on the viewable map
	player.tx = bg->tx + 10;
	player.ty = bg->ty + 8;
	player.x = player.tx*TILEWH;
	player.y = player.ty*TILEWH;
	player.triggerx = player.tx;
	player.triggery = player.ty+1;
	player.q=1;
	player.d=0;
	player.hp=4;
	//npc
	/*npc0.tx = bg->tx + 1;
	npc0.ty = bg->ty + 1;
	npc0.x = npc0.tx*TILEWH;
	npc0.y = npc0.ty*TILEWH;
	npc0.triggerx = npc0.tx;
	npc0.triggery = npc0.ty+1;
	npc0.q=1;
	npc0.d=0;
	modexDrawSpriteRegion(spri->page, npc0.x-4, npc0.y-TILEWH, 24, 64, 24, 32, &npctmp);*/
	modexDrawSpriteRegion(spri->page, player.x-4, player.y-TILEWH, 24, 64, 24, 32, &ptmp);

	modexShowPage(spri->page);
//	printf("Total used @ before loop:			%zu\n", oldfreemem-GetFreeSize());
	modexClearRegion(mv[2].page, 0, 0, mv[2].page->width, mv[2].page->height, 1);
	modexFadeOn(4, gpal);
	while(!keyp(1) && player.hp>0)
	{
	//top left corner & bottem right corner of map veiw be set as map edge trigger since maps are actually square
	//to stop scrolling and have the player position data move to the edge of the screen with respect to the direction
	//when player.tx or player.ty == 0 or player.tx == 20 or player.ty == 15 then stop because that is edge of map and you do not want to walk of the map
	#define INC_PER_FRAME if(player.q&1) persist_aniframe++; if(persist_aniframe>4) persist_aniframe = 1;

	//player movement
	//TODO: make movement into a function!
	if(!panswitch){
	//right movement
	if((keyp(77) && !keyp(75) && player.d == 0) || player.d == 2)
	{
		if(player.d == 0){ player.d = 2; }
		if(bg->tx >= 0 && bg->tx+20 < map.width && player.tx == bg->tx + 10 &&
		!(bg->map->data[(player.tx)+(map.width*(player.ty-1))] == 0))//!(player.tx+1 == TRIGGX && player.ty == TRIGGY))	//collision detection!
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				//animatePlayer(bg, spri, mask, 1, 1, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 1, player.x, player.y, persist_aniframe, player.q, &ptmp);
				mapScrollRight(mv, SPEED, 0);
				mapScrollRight(mv, SPEED, 1);
				//mapScrollRight(mask, SPEED);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.tx++; }
		}
		else if(player.tx < map.width && !(bg->map->data[(player.tx)+(map.width*(player.ty-1))] == 0))//!(player.tx+1 == TRIGGX && player.ty == TRIGGY))
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				player.x+=SPEED;
				//animatePlayer(bg, spri, mask, 1, 0, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 0, player.x, player.y, persist_aniframe, player.q, &ptmp);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.tx++; }
		}
		else
		{
			modexCopyPageRegion(spri->page, bg->page, player.x-4, player.y-TILEWH, player.x-4, player.y-TILEWH, 24, 32);
			modexDrawSpriteRegion(spri->page, player.x-4, player.y-TILEWH, 24, 32, 24, 32, &ptmp);
			modexShowPage(spri->page);
			player.d = 0;
		}
		player.triggerx = player.tx+1;
		player.triggery = player.ty;
	}

	//left movement
	if((keyp(75) && !keyp(77) && player.d == 0) || player.d == 4)
	{
		if(player.d == 0){ player.d = 4; }
		if(bg->tx > 0 && bg->tx+20 <= map.width && player.tx == bg->tx + 10 &&
		!(bg->map->data[(player.tx-2)+(map.width*(player.ty-1))] == 0))//!(player.tx-1 == TRIGGX && player.ty == TRIGGY))	//collision detection!
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				//animatePlayer(bg, spri, mask, 3, 1, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 1, player.x, player.y, persist_aniframe, player.q, &ptmp);
				mapScrollLeft(mv, SPEED, 0);
				mapScrollLeft(mv, SPEED, 1);
				//mapScrollLeft(mask, SPEED);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.tx--; }
		}
		else if(player.tx > 1 && !(bg->map->data[(player.tx-2)+(map.width*(player.ty-1))] == 0))//!(player.tx-1 == TRIGGX && player.ty == TRIGGY))
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				player.x-=SPEED;
				//animatePlayer(bg, spri, mask, 3, 0, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 0, player.x, player.y, persist_aniframe, player.q, &ptmp);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.tx--; }
		}
		else
		{
			modexCopyPageRegion(spri->page, bg->page, player.x-4, player.y-TILEWH, player.x-4, player.y-TILEWH, 24, 32);
			modexDrawSpriteRegion(spri->page, player.x-4, player.y-TILEWH, 24, 96, 24, 32, &ptmp);
			modexShowPage(spri->page);
			player.d = 0;
		}
		player.triggerx = player.tx-1;
		player.triggery = player.ty;
	}

	//down movement
	if((keyp(80) && !keyp(72) && player.d == 0) || player.d == 3)
	{
		if(player.d == 0){ player.d = 3; }
		if(bg->ty >= 0 && bg->ty+15 < map.height && player.ty == bg->ty + 8 &&
		!(bg->map->data[(player.tx-1)+(map.width*(player.ty))] == 0))//!(player.tx == TRIGGX && player.ty+1 == TRIGGY))	//collision detection!
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				//animatePlayer(bg, spri, mask, 2, 1, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 1, player.x, player.y, persist_aniframe, player.q, &ptmp);
				mapScrollDown(mv, SPEED, 0);
				mapScrollDown(mv, SPEED, 1);
				//mapScrollDown(mask, SPEED);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.ty++; }
		}
		else if(player.ty < map.height && !(bg->map->data[(player.tx-1)+(map.width*(player.ty))] == 0))//!(player.tx == TRIGGX && player.ty+1 == TRIGGY))
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				player.y+=SPEED;
				//animatePlayer(bg, spri, mask, 2, 0, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 0, player.x, player.y, persist_aniframe, player.q, &ptmp);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.ty++; }
		}
		else
		{
			modexCopyPageRegion(spri->page, bg->page, player.x-4, player.y-TILEWH, player.x-4, player.y-TILEWH, 24, 32);
			modexDrawSpriteRegion(spri->page, player.x-4, player.y-TILEWH, 24, 64, 24, 32, &ptmp);
			modexShowPage(spri->page);
			player.d = 0;
		}
		player.triggerx = player.tx;
		player.triggery = player.ty+1;
	}

	//up movement
	if((keyp(72) && !keyp(80) && player.d == 0) || player.d == 1)
	{
		if(player.d == 0){ player.d = 1; }
		if(bg->ty > 0 && bg->ty+15 <= map.height && player.ty == bg->ty + 8 &&
		!(bg->map->data[(player.tx-1)+(map.width*(player.ty-2))] == 0))//!(player.tx == TRIGGX && player.ty-1 == TRIGGY))	//collision detection!
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				//animatePlayer(bg, spri, mask, 0, 1, player.x, player.y, persist_aniframe, q, &ptmp);
				animatePlayer(bg, spri, player.d-1, 1, player.x, player.y, persist_aniframe, player.q, &ptmp);
				mapScrollUp(mv, SPEED, 0);
				mapScrollUp(mv, SPEED, 1);
				//mapScrollUp(mask, SPEED);
				modexShowPage(spri->page);
				player.q++;
			} else { player.q = 1; player.d = 0; player.ty--; }
		}
		else if(player.ty > 1 && !(bg->map->data[(player.tx-1)+(map.width*(player.ty-2))] == 0))//!(player.tx == TRIGGX &&  player.ty-1 == TRIGGY))
		{
			if(player.q<=(TILEWH/SPEED))
			{
				INC_PER_FRAME;
				player.y-=SPEED;
				//animatePlayer(bg, spri, mask, 0, 0, player.x, player.y, persist_aniframe, q, &ptmp);
				modexShowPage(spri->page);
				animatePlayer(bg, spri, player.d-1, 0, player.x, player.y, persist_aniframe, player.q, &ptmp);
				player.q++;
			} else { player.q = 1; player.d = 0; player.ty--; }
		}
		else
		{
			modexCopyPageRegion(spri->page, bg->page, player.x-4, player.y-TILEWH, player.x-4, player.y-TILEWH, 24, 32);
			modexDrawSpriteRegion(spri->page, player.x-4, player.y-TILEWH, 24, 0, 24, 32, &ptmp);
			modexShowPage(spri->page);
			player.d = 0;
		}
		player.triggerx = player.tx;
		player.triggery = player.ty-1;
	}
}else{
//88 switch!
	//right movement
	if((keyp(77) && !keyp(75) && pand == 0) || pand == 2)
	{
		if(pand == 0){ pand = 2; }
			if(panq<=(TILEWH/SPEED))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dx++;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dx++;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dx++;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//left movement
	if((keyp(75) && !keyp(77) && pand == 0) || pand == 4)
	{
		if(pand == 0){ pand = 4; }
			if(panq<=(TILEWH/SPEED))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dx--;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dx--;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dx--;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//down movement
	if((keyp(72) && !keyp(80) && pand == 0) || pand == 3)
	{
		if(pand == 0){ pand = 3; }
			if(panq<=(TILEWH/SPEED))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dy--;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dy--;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dy--;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
	//up movement
	if((keyp(80) && !keyp(72) && pand == 0) || pand == 1)
	{
		if(pand == 0){ pand = 1; }
			if(panq<=(TILEWH/SPEED))
			{
				switch(panpagenum)
				{
					case 0:
						//bg
						bg->page->dy++;
						modexShowPage(bg->page);
					break;
					case 1:
						//spri
						spri->page->dy++;
						modexShowPage(spri->page);
					break;
					case 2:
						//fg
						mask->page->dy++;
						modexShowPage(mask->page);
					break;
				}
				panq++;
			} else { panq = 1; pand = 0; }
	}
}

	//the scripting stuf....

	//if(((player.triggerx == TRIGGX && player.triggery == TRIGGY) && keyp(0x1C))||(player.tx == 5 && player.ty == 5))
	if(((bg->map->data[(player.triggerx-1)+(map.width*(player.triggery-1))] == 0) && keyp(0x1C))||(player.tx == 5 && player.ty == 5))
	{
		short i;
		for(i=800; i>=400; i--)
		{
			sound(i);
		}
		nosound();
	}
	if(player.q == (TILEWH/SPEED)+1 && player.d > 0 && (player.triggerx == 5 && player.triggery == 5)){ player.hp--; }
	//debugging binds!
	//if(keyp(0x0E)) while(1){ if(xmsmalloc(24)) break; }
	if(keyp(2)){ modexShowPage(bg->page); panpagenum=0; }
	if(keyp(3)){ modexShowPage(spri->page); panpagenum=1; }
	if(keyp(4)){ modexShowPage(mask->page); panpagenum=2; }
	if(keyp(25)){ pdump(bg); pdump(spri); }	//p
	if(keyp(24)){ modexPalUpdate0(gpal); paloffset=0; pdump(bg); pdump(spri); }
	if(keyp(22)){
	paloffset=0; modexPalBlack(); modexPalUpdate(&ptmp, &paloffset, 0, 0);
	printf("1paloffset	=	%d\n", paloffset/3);
	 modexPalUpdate(map.tiles->data, &paloffset, 0, 0);
	printf("2paloffset	=	%d\n", paloffset/3);
	 pdump(bg); pdump(spri); }
	//pan switch
	if(keyp(88)){if(!panswitch) panswitch++; else panswitch--; }	//f12
	//TSR
	if(keyp(87))	//f11
	{
		modexLeave();
		setkb(0);
		__asm
		{
			mov ah,31h
			int 21h
		}
	}

	if((player.q==1) && !(player.x%TILEWH==0 && player.y%TILEWH==0)) break;	//incase things go out of sync!

	}

	/* fade back to text mode */
	/* but 1st lets save the game palette~ */
	modexPalSave(gpal);
	modexSavePalFile("data/g.pal", gpal);
	modexFadeOff(4, gpal);
	modexLeave();
	setkb(0);
	printf("Project 16 scroll.exe\n");
	printf("tx: %d\n", bg->tx);
	printf("ty: %d\n", bg->ty);
	printf("player.x: %d", player.x); printf("		player.y: %d\n", player.y);
	//if(player.hp==0) printf("%d wwww\n", player.y+8);
	//else printf("\nplayer.y: %d\n", player.y);
	printf("player.tx: %d", player.tx); printf("		player.ty: %d\n", player.ty);
	printf("player.triggx: %d", player.triggerx); printf("	player.triggy: %d\n", player.triggery);
	printf("player.hp: %d", player.hp);	printf("	player.q: %d", player.q);	printf("	player.d: %d\n", player.d);
	printf("tile data value at player trigger position: %d\n", bg->map->data[(player.triggerx-1)+(map.width*(player.triggery-1))]);
	printf("palette offset:	%d\n", paloffset/3);
//++++	printf("Total used: %zu\n", oldfreemem-GetFreeSize());
//++++	printf("Total free: %zu\n", GetFreeSize());
	printf("temporary player sprite 0: http://www.pixiv.net/member_illust.php?mode=medium&illust_id=45556867\n");
	printf("temporary player sprite 1: http://www.pixiv.net/member_illust.php?mode=medium&illust_id=44606385\n");
	printf("Screen: %dx", screen.width);	printf("%d\n", screen.height);
	printf("Screen2: %dx", screen2.width);	printf("%d\n", screen2.height);
	//printf("map.width=%d	map.height=%d	map.data[0]=%d\n", bg->map->width, bg->map->height, bg->map->data[0]);
	//xmsfree(&map);
	//xmsfree(bg);
	//xmsfree(spri);
	//xmsfree(mask);
	//xmsreport();
	//emmclose(emmhandle);
	switch(detectcpu())
	{
		case 0: cpus = "8086/8088 or 186/88"; break;
		case 1: cpus = "286"; break;
		case 2: cpus = "386 or newer"; break;
		default: cpus = "internal error"; break;
	}
	printf("detected CPU type: %s\n", cpus);
	modexFadeOn(4, dpal);
}


map_t
allocMap(int w, int h) {
	map_t result;

	result.width =w;
	result.height=h;
	result.data = malloc(sizeof(byte) * w * h);
	//result.data = (byte *)alloc_emem(((int)sizeof(byte) * w * h)/1024);
	/*if(isEMS() || checkEMS())
	{
		XMOVE mm;
		//emmhandle = mallocEMS(coretotalEMS());//alloc_emem((int)sizeof(map))
		mm.length=sizeof(result);
		mm.sourceH=0;
		mm.sourceOff=ptr2long(&result);
		mm.destH=emmhandle;
		mm.destOff=0;
		ist = move_emem(&mm);
		if(!ist){ dealloc_emem(emmhandle); exit(5); }
		printf("%d\n", coretotalEMS());
	}*/

	return result;
}

void
initMap(map_t *map) {
	/* just a place holder to fill out an alternating pattern */
	int x, y, xx, yy;
	int i, q;
//	int tile = 1;
	//if(!isEMS() || !checkEMS())
//		map->tiles = malloc(sizeof(tiles_t));
	//else
	//	map->tiles = (tiles_t *)alloc_emem(sizeof(tiles_t));

	/* create the tile set */
	//if(!isEMS() || !checkEMS())
//		map->tiles->data = malloc(sizeof(bitmap_t));
	//else
	//	map->tiles->data = (bitmap_t *)alloc_emem(sizeof(bitmap_t));
//	map->tiles->data->width = (TILEWH/**2*/);
//	map->tiles->data->height= TILEWH;
	//if(!isEMS() || !checkEMS())
//		map->tiles->data->data = malloc((TILEWH*2)*TILEWH);
	//else
	//	map->tiles->data->data = (byte *)alloc_emem((TILEWH*2)*TILEWH);
//	map->tiles->tileHeight = TILEWH;
//	map->tiles->tileWidth =TILEWH;
//	map->tiles->rows = 1;
//	map->tiles->cols = 1;//2;

	/*q=0;
	//for(y=0; y<map->height; y++) {
	//for(x=0; x<map->width; x++) {
	i=0;
	for(yy=0; yy<TILEWH; yy++) {
	for(xx=0; xx<(TILEWH); xx++) {
		//if(x<TILEWH){
		  map->tiles->data->data[i+1] = map->data[q];//28;//0x24;
//		  printf("[%d]", map->tiles->data->data[i]);
		//}else{
		  //map->tiles->data->data[i] = map->data[q];//0;//0x34;
		  //printf("]%d[==[%d]", i, map->tiles->data->data[i]);
		//}
		i++;
	}
//	printf("\n");
	}
//	printf("[%d]", map->data[q]);
	q++;
//	}
	//printf("\n\n");
//	}*/

	/*i=0;
	for(y=0; y<map->height; y++) {
		for(x=0; x<map->width; x++) {
//			map->data[i]=255;
			printf("[%d]", map->data[i]);
			//tile = tile ? 0 : 1;
			i++;
		}
		//tile = tile ? 0 : 1;
	}*/
}


void
mapScrollRight(map_view_t *mv, byte offset, word id)
{
	word x, y;  /* coordinate for drawing */

	/* increment the pixel position and update the page */
	mv[id].page->dx += offset;

	/* check to see if this changes the tile */
	if(mv[id].page->dx >= mv[id].dxThresh ) {
	/* go forward one tile */
	mv[id].tx++;
	/* Snap the origin forward */
	mv[id].page->data += 4;
	mv[id].page->dx = mv[id].map->tiles->tileWidth;

	/* draw the next column */
	x= SCREEN_WIDTH + mv[id].map->tiles->tileWidth;
		if(id==0)
			mapDrawCol(&mv[0], mv[0].tx + 20 , mv[0].ty-1, x);
		else
			modexCopyPageRegion(mv[id].page, mv[0].page, x, 0, x, 0, mv[id].map->tiles->tileWidth, mv[id].map->tiles->tileHeight*17);
	}
}


void
mapScrollLeft(map_view_t *mv, byte offset, word id)
{
	word x, y;  /* coordinate for drawing */

	/* increment the pixel position and update the page */
	mv[id].page->dx -= offset;

	/* check to see if this changes the tile */
	if(mv[id].page->dx == 0) {
	/* go backward one tile */
	mv[id].tx--;

	/* Snap the origin backward */
	mv[id].page->data -= 4;
	mv[id].page->dx = mv[id].map->tiles->tileWidth;

	/* draw the next column */
	x= 0;
		if(id==0)
			mapDrawCol(&mv[0], mv[0].tx-1, mv[0].ty-1, 0);
		else
			modexCopyPageRegion(mv[id].page, mv[0].page, x, 0, x, 0, mv[id].map->tiles->tileWidth, mv[id].map->tiles->tileHeight*17);
	}
}


void
mapScrollUp(map_view_t *mv, byte offset, word id)
{
	word x, y;  /* coordinate for drawing */

	/* increment the pixel position and update the page */
	mv[id].page->dy -= offset;

	/* check to see if this changes the tile */
	if(mv[id].page->dy == 0 ) {
	/* go down one tile */
	mv[id].ty--;
	/* Snap the origin downward */
	mv[id].page->data -= mv[id].page->width*4;
	mv[id].page->dy = mv[id].map->tiles->tileHeight;

	/* draw the next row */
	y= 0;
		if(id==0)
			mapDrawRow(&mv[0], mv[0].tx-1 , mv[0].ty-1, 0);
		else
			modexCopyPageRegion(mv[id].page, mv[0].page, 0, y, 0, y, mv[id].map->tiles->tileWidth*22, mv[id].map->tiles->tileHeight);
	}
}


void
mapScrollDown(map_view_t *mv, byte offset, word id)
{
	word x, y;  /* coordinate for drawing */

	/* increment the pixel position and update the page */
	mv[id].page->dy += offset;

	/* check to see if this changes the tile */
	if(mv[id].page->dy >= mv[id].dyThresh ) {
	/* go down one tile */
	mv[id].ty++;
	/* Snap the origin downward */
	mv[id].page->data += mv[id].page->width*4;
	mv[id].page->dy = mv[id].map->tiles->tileHeight;

	/* draw the next row */
	y= SCREEN_HEIGHT + mv[id].map->tiles->tileHeight;
		if(id==0)
			mapDrawRow(&mv[0], mv[0].tx-1 , mv[0].ty+15, y);
		else
			modexCopyPageRegion(mv[id].page, mv[0].page, 0, y, 0, y, mv[id].map->tiles->tileWidth*22, mv[id].map->tiles->tileHeight);
	}
}


void
mapGoTo(map_view_t *mv, int tx, int ty) {
	int px, py;
	unsigned int i;

	/* set up the coordinates */
	mv->tx = tx;
	mv->ty = ty;
	mv->page->dx = mv->map->tiles->tileWidth;
	mv->page->dy = mv->map->tiles->tileHeight;

	/* set up the thresholds */
	mv->dxThresh = mv->map->tiles->tileWidth * 2;
	mv->dyThresh = mv->map->tiles->tileHeight * 2;

	/* draw the tiles */
	modexClearRegion(mv->page, 0, 0, mv->page->width, mv->page->height, 0);
	py=0;
	i=mv->ty * mv->map->width + mv->tx;
	for(ty=mv->ty-1; py < SCREEN_HEIGHT+mv->dyThresh && ty < mv->map->height; ty++, py+=mv->map->tiles->tileHeight) {
		mapDrawRow(mv, tx-1, ty, py);
	i+=mv->map->width - tx;
	}
}


void
mapDrawTile(tiles_t *t, word i, page_t *page, word x, word y) {
	word rx;
	word ry;
	//if(i==0) i=2;
	if(i==0)
	{
		//wwww
		modexClearRegion(page, x, y, t->tileWidth, t->tileHeight, 0); //currently the over scan color!
	}
	else
	{
	rx = (((i-1) % ((t->data->width)/t->tileWidth)) * t->tileWidth);
	ry = (((i-1) / ((t->data->height)/t->tileHeight)) * t->tileHeight);
////0000	printf("i=%d\n", i);
	//mxPutTile(t->data, x, y, t->tileWidth, t->tileHeight);
	modexDrawBmpRegion(page, x, y, rx, ry, t->tileWidth, t->tileHeight, (t->data));
	}
}


void
mapDrawRow(map_view_t *mv, int tx, int ty, word y) {
	word x;
	int i;

	/* the position within the map array */
	i=ty * mv->map->width + tx;
	for(x=0; x<SCREEN_WIDTH+mv->dxThresh && tx < mv->map->width; x+=mv->map->tiles->tileWidth, tx++) {
	if(i>=0) {
		/* we are in the map, so copy! */
		mapDrawTile(mv->map->tiles, mv->map->data[i], mv->page, x, y);
	}
	i++; /* next! */
	}
}

void
mapDrawCol(map_view_t *mv, int tx, int ty, word x) {
	int y;
	int i;

	/* location in the map array */
	i=ty * mv->map->width + tx;

	/* We'll copy all of the columns in the screen,
	   i + 1 row above and one below */
	for(y=0; y<SCREEN_HEIGHT+mv->dyThresh && ty < mv->map->height; y+=mv->map->tiles->tileHeight, ty++) {
	if(i>=0) {
		/* we are in the map, so copy away! */
		mapDrawTile(mv->map->tiles, mv->map->data[i], mv->page, x, y);
	}
	i += mv->map->width;
	}
}

void qclean()
{
	modexLeave();
	setkb(0);
}

void pdump(map_view_t *pee)
{
	int mult=(QUADWH);
	int palq=(mult)*TILEWH;
	int palcol=0;
	int palx, paly;
	for(paly=0; paly<palq; paly+=mult){
		for(palx=0; palx<palq; palx+=mult){
				modexClearRegion(pee->page, palx+TILEWH, paly+TILEWH, mult, mult, palcol);
			palcol++;
		}
	}
}

void
animatePlayer(map_view_t *src, map_view_t *dest, /*map_view_t *top, */sword d, short scrolloffsetswitch, int x, int y, int ls, int lp, bitmap_t *bmp)
{
	sword dire=32*d; //direction
	sword qq; //scroll offset

	if(scrolloffsetswitch==0) qq = 0;
	else qq = ((lp)*SPEED);
	switch (d)
	{
		case 0:
			//up
			x=x-4;
			y=y-qq-TILEWH;
		break;
		case 1:
			// right
			x=x+qq-4;
			y=y-TILEWH;
		break;
		case 2:
			//down
			x=x-4;
			y=y+qq-TILEWH;
		break;
		case 3:
			//left
			x=x-qq-4;
			y=y-TILEWH;
		break;
	}
	modexCopyPageRegion(dest->page, src->page, x-4, y-4, x-4, y-4, 28, 40);
	if(2>ls && ls>=1) { modexDrawSpriteRegion(dest->page, x, y, 48, dire, 24, 32, bmp); }else
	if(3>ls && ls>=2) { modexDrawSpriteRegion(dest->page, x, y, 24, dire, 24, 32, bmp); }else
	if(4>ls && ls>=3) { modexDrawSpriteRegion(dest->page, x, y, 0, dire, 24, 32, bmp); }else
	if(5>ls && ls>=4) { modexDrawSpriteRegion(dest->page, x, y, 24, dire, 24, 32, bmp); }
	//TODO: mask copy //modexCopyPageRegion(dest->page, src->page, x-4, y-4, x-4, y-4, 28, 40);
	//modexClearRegion(top->page, 66, 66, 2, 40, 0);
	//modexCopyPageRegion(dest->page, top->page, 66, 66, 66, 66, 2, 40);
	//turn this off if XT
	//XTif(detectcpu() > 0)
	modexWaitBorder();
}
