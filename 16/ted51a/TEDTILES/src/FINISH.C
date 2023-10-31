/////////////////////////////////////////////////////////
//
// Finish-Up Grabbing Stuff
//
/////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

long huge *fileoffs;
int xms;
unsigned numall;
char grname[14],headname[14];
long startlen,currentlen=0,oldlen=0,constcomp,constlen;
unsigned vtab,start=0,allsparse=0;
int oldhtab,oldvtab,xmsSource,xmsDest,xmsok;

/////////////////////////////////////////////////////////
//
// Here it is! Compresses and saves one big fat file!
//
/////////////////////////////////////////////////////////
void FinishUp(void)
{
 SetupFinish();
 CreateOffsets();
 CompressSpecial();
 CompressData();
 CompressMisc();
 CreateGraphFiles();
 CreateHeaders();
 UpdateWindow();

 //
 // deallocate all the memory!
 //
 farfree((void far *)lbmscreen);
 farfree((void far *)databuffer);
 if (!leavetmp)
   DeleteTmpFiles();

 CreateOBJs();

 //
 // OUTTA HERE!
 //
 gotoxy(1,23);
 poke(0,0x41a,peek(0,0x41c));	// clear keyboard
 nosound();
 exit(0);
}



/////////////////////////////////////////////////////////
//
// Search the START string for the GRAB TYPE
//
/////////////////////////////////////////////////////////
void FindType(char *string)
{
 if (!strncmp(string,"FONTM",5))
   type=FONTMTYPE;
 else
 if (!strncmp(string,"FONT",4))
    type=FONTTYPE;
 else

 if (!strncmp(string,"TILE8M",6))
    type=TILE8MTYPE;
 else
 if (!strncmp(string,"TILE8",5))
    type=TILE8TYPE;
 else
 if (!strncmp(string,"ALT8M",5))
    type=ALT8MTYPE;
 else
 if (!strncmp(string,"ALT8",4))
    type=ALT8TYPE;
 else

 if (!strncmp(string,"TILE16M",7))
    type=TILE16MTYPE;
 else
 if (!strncmp(string,"TILE16",6))
    type=TILE16TYPE;
 else
 if (!strncmp(string,"ALT16M",6))
    type=ALT16MTYPE;
 else
 if (!strncmp(string,"ALT16",5))
    type=ALT16TYPE;
 else

 if (!strncmp(string,"TILE32M",7))
    type=TILE32MTYPE;
 else
 if (!strncmp(string,"TILE32",6))
    type=TILE32TYPE;
 else
 if (!strncmp(string,"ALT32M",6))
    type=ALT32MTYPE;
 else
 if (!strncmp(string,"ALT32",5))
    type=ALT32TYPE;
 else

 if (!strncmp(string,"PICM",4))
    type=PICMTYPE;
 else
 if (!strncmp(string,"PIC",3))
    type=PICTYPE;
 else

 if (!strncmp(string,"SPRITES",7))
    type=SPRITETYPE;
 else
   {
    char msg[80]="YOU CAN'T 'START ";

    strcat(msg,string);
    strcat(msg,"'!");
    errout(msg);
   }
}


/////////////////////////////////////////////////////////
//
// Load an LBM screen (& maskscreen if applicable)
//
/////////////////////////////////////////////////////////
void LoadKeyword(char *string,int grabbed)
{
 char huge *EGAscrn=MK_FP(0xa000,0),huge *CGAscrn=MK_FP(0xb800,0);

 //
 // GET A KEYPRESS
 //
 if ((!fastgrab) && grabbed)
   {
    int key;

    if (bioskey(1))
      bioskey(0);

    if (!SkipToStart)
    {
     key=bioskey(0);

     switch(key&255)
     {
      case 13:
	SkipToStart=1;
	break;
      case 27:
	errout("IGRAB ABORTED!");
      case  9:
	if (!noshow)
	{
	 settext();
	 DispStatusScreen();
	 fastgrab=noshow=1;
	 poke(0,0x41a,peek(0,0x41c));	// clear keyboard
	}
     }
    }
   }

 switch(bioskey(1)>>8)
 {
  case 1:
    errout("IGRAB ABORTED!");
  case 15:
    if (!noshow)
    {
     settext();
     DispStatusScreen();
     fastgrab=noshow=1;
     poke(0,0x41a,peek(0,0x41c));	// clear keyboard
    }
 }

 //
 // create screen suffix
 //
 strcpy(picname,path);
 strcat(picname,string);
 picname[strlen(picname)-2]=0;
 switch(format[0])
   {
    case 'C': strcat(picname,"C.LBM");
	      break;
    case 'E': strcat(picname,"E.LBM");
	      break;
    case 'V': strcat(picname,"V.LBM");
   }

 //
 // all PIC,PICM,SPRITE data is flushed!
 //
 FlushData();

 //
 // blast any buffers still intact
 //
 if (lbmscreen!=NULL)
    farfree((void far *)lbmscreen);
 if (maskscreen!=NULL)
    farfree((void far *)maskscreen);

 lbmscreen=maskscreen=NULL;	// do this OR ELSE!

 lbmscreen=LoadLBM(picname,&CurrentLBM);

 //
 // load the mask screen for current type,
 // if applicable
 //
 if (!noshow && !SkipToStart)
   //
   // CLEAR THE SCREEN
   //
   switch(format[0])
     {
      case 'C':
	for (i=0;i<0x4000;i++)
	  *(CGAscrn+i)=0;
	break;

      case 'E':
	outport(GCindex,GCmode);
	outport(SCindex,SCmapmask + 0xf00);
	for (i=0;i<8000;i++)
	  *(EGAscrn+i)=0;
	break;
      case 'V':
	asm	pushf
	asm	push	di
	asm	mov	ax,0xa000
	asm	mov	es,ax
	asm	xor	di,di
	asm	cld
	asm	mov	cx,(320*200)/2
	asm	xor	ax,ax
	asm	rep stosw
	asm	pop	di
	asm	popf
     }

 if (type==FONTMTYPE ||
     type==TILE8MTYPE || type==ALT8MTYPE ||
     type==TILE16MTYPE || type==ALT16MTYPE ||
     type==TILE32MTYPE || type==ALT32MTYPE ||
     type==PICMTYPE || type==SPRITETYPE)
   {
    char maskname[64];

    strcpy(maskname,path);
    strcat(maskname,string);
    maskname[strlen(maskname)-2]=0;
    switch(format[0])
      {
       case 'C': strcat(maskname,"MC.LBM");
		 if (!noshow)
		   {
		    for (i=0;i<0x4000;i++)
		      *(CGAscrn+i)=0xaa;
		   }
		 break;

       case 'E': strcat(maskname,"ME.LBM");
		 if (!noshow)
		   {
		    outport(GCindex,GCmode);
		    ScreenColor=random(15)+1;
		    outport(SCindex,SCmapmask | (ScreenColor*256));
		    for (i=0;i<8000;i++)
		      *(EGAscrn+i)=0xff;
		   }
		 break;
       case 'V': strcat(maskname,"MV.LBM");
		 if (!noshow)
		   {
		    char color=(random(13)+1)*16;

		    asm		pushf
		    asm		push	di

		    asm		cld
		    asm		mov	ax,0xa000
		    asm		mov	es,ax
		    asm		xor	di,di
		    asm		mov	al,[color]
		    asm		mov	bx,200/12
		    COLOOP:
		    asm		mov	cx,320*12
		    asm		rep stosb

		    asm		inc	al
		    asm		dec	bx
		    asm		cmp	bx,0
		    asm		jnz	COLOOP

		    asm		pop	di
		    asm		popf
		   }
      }

    maskscreen=LoadLBM(maskname,&CurrentLBM);
   }

 if (!noshow)
   {
    _BH=0;
    _DX=0x1800;
    _AH=2;
    geninterrupt(0x10);
    printf("%s - %s",typelist[type],picname);
   }
 else
   {
    cprintf("\r\nGrabbing from %s - %s",typelist[type],picname);
    clreol();
   }

 //
 // MAKE SURE THE SCREEN JUST LOADED IS
 // CORRECT FOR THE GRABBING VIDEOMODE!
 //
 switch(format[0])
 {
  case 'C':
    if (CurrentLBM.planes!=2)
      {
       char erstr[200]="The screen '";
       strcat(erstr,picname);
       strcat(erstr,"' isn't a CGA screen!");
       errout(erstr);
      }
    break;
  case 'E':
    if (CurrentLBM.planes!=4)
      {
       char erstr[200]="The screen '";
       strcat(erstr,picname);
       strcat(erstr,"' isn't an EGA screen!");
       errout(erstr);
      }
    break;
  case 'V':
    if (CurrentLBM.planes!=4)
      {
       char erstr[200]="The screen '";
       strcat(erstr,picname);
       strcat(erstr,"' isn't a VGA screen!");
       errout(erstr);
      }
    break;
 }


 #if 0
 //
 // debug code to blast the screen in "lbmscreen" buffer
 // to EGA screen
 //
 {
  unsigned DSreg,SIreg,i,lwidth,height;
  long size;


  lwidth=CurrentLBM.width/8;
  height=CurrentLBM.height;
  size=lwidth*height;
  outport(GCindex,GCmode);

  for (i=0;i<4;i++)
    {
      outport(SCindex,SCmapmask | (1<<i)*256);
      DSreg=FP_SEG(lbmscreen)+(size*i)/16;
      SIreg=FP_OFF(lbmscreen)+(size*i)&15;

      asm	push	ds
      asm	mov	si,SIreg
      asm	mov	ax,DSreg
      asm	mov	ds,ax
      asm	xor	di,di
      asm	mov	ax,0xa000
      asm	mov	es,ax
      asm	cld
      asm	mov	bx,[height]

      LOOP:

      asm	mov	cx,20
      asm	rep movsw
      asm	sub	si,40
      asm	add	si,[lwidth]

      asm	dec	bx
      asm	jnz	LOOP
      asm	pop	ds
    }
  bioskey(0);
 }
 #endif
}


/////////////////////////////////////////////////////////
//
// See if there's enough Extended memory for FastCaching
// If so, allocate the memory & load in the file to Extended Memory
//
/////////////////////////////////////////////////////////
int CheckXMSamount(char *filename,int *handle1,int *handle2)
{
 long size,clen,coff;

 if (!xms)
   return 0;

 size=filelen(filename);
 if (1024L*XMSTotalFree()<2L*size)
   {
    *handle1=*handle2=0;
    return 0;
   }

 *handle1=XMSAllocate(size);
 *handle2=XMSAllocate(size);

 coff=0;
 do
 {
  clen=0x8000;
  if (size<0x8000)
    clen=size;

  LoadFile(filename,lbmscreen,coff,clen);
  XMSmove(0,(long)lbmscreen,*handle1,coff,clen);
  size-=0x8000;
  coff+=clen;
 } while(size>0);

 return 1;
}


/////////////////////////////////////////////////////////
//
// Free XMS buffers, if any
//
/////////////////////////////////////////////////////////
void FreeXMSbuffs(int *handle1,int *handle2,char *filename,long start,long size)
{
 long clen,coff;

 if (!xms || (!*handle1 && !*handle2))
   return;

 coff=0;
 do
 {
  clen=0x8000;
  if (size<0x8000)
    clen=size;

  XMSmove(*handle2,coff,0,(long)lbmscreen,clen);
  SaveFile(filename,lbmscreen,clen,start+coff);
  size-=0x8000;
  coff+=clen;
 } while(size>0);

 XMSFreeMem(*handle1);
 XMSFreeMem(*handle2);
 *handle1=*handle2=0;
}


/////////////////////////////////////////////////////////
//
// Get file's length
//
/////////////////////////////////////////////////////////
long filelen(char *string)
{
 long size;
 int handle;

 if ((handle=_open(string,O_BINARY))<0)
   return 0;

 size=filelength(handle);
 close(handle);
 return size;
}


/////////////////////////////////////////////////////////
//
// Move status screen to text screen
//
/////////////////////////////////////////////////////////
void DispStatusScreen(void)
{
 _fmemcpy((void far *)MK_FP(0xb800,0),&SCREEN+7,4000);

 window(1,1,80,25);
 gotoxy(43,4);
 cprintf("%s",VERSION);
 gotoxy(46,5);
 cprintf("%cGA",format[0]);
 window(2,8,78,19);
}


/////////////////////////////////////////////////////////
//
// Create .H,.EQU files
//
/////////////////////////////////////////////////////////
void CreateHeaders(void)
{
 char str[50],temp[5];
 long start;
 int chnk=0;


 time(&tblock);

 strcpy(dest,"GFX?_ext.H");
 dest[3]=format[0];
 strncpy(dest+5,ext,3);
 unlink(dest);
 if ((fp=fopen(dest,"wt"))==NULL)
   {
    char msg[80]="Couldn't create '";

    strcat(msg,dest);
    strcat(msg,"'!");
    errout(msg);
   }

 fprintf(fp,"//////////////////////////////////////\n");
 fprintf(fp,"//\n");
 fprintf(fp,"// Graphics .H file for .%s\n",ext);
 fprintf(fp,"// IGRAB-ed on %s",ctime(&tblock));
 fprintf(fp,"//\n");
 fprintf(fp,"//////////////////////////////////////\n\n");

 fprintf(fp,"typedef enum {\n");

 for (i=0;i<Data[PIC].num;i++)
   {
    int end;

    if (ChunkStart[chnk]+Data[ChunkType[chnk]].offset==i+Data[PIC].offset)
      {
       fprintf(fp,"\t\t// Lump Start\n");
       chnk++;
      }

    _fmemcpy((char far *)str,(char far *)PicNames+i*NAMELEN,50);
    strcat(str,"PIC");
    if (!i)
      fprintf(fp,"\t\t%s=%d,\n",str,i+Data[PIC].offset);
    else
    {
     strcat(str,",");
     while(strlen(str)<NAMELEN+5)
       strcat(str," ");
      fprintf(fp,"\t\t%s// %d\n",str,i+Data[PIC].offset);
    }
   }

 fprintf(fp,"\n");

 for (i=0;i<Data[PICM].num;i++)
   {
    int end;

    if (ChunkStart[chnk]+Data[ChunkType[chnk]].offset==i+Data[PICM].offset)
      {
       fprintf(fp,"\t\t// Lump Start\n");
       chnk++;
      }

    _fmemcpy((char far *)str,(char far *)PicMNames+i*NAMELEN,50);
    strcat(str,"PICM");
    if (!i)
      fprintf(fp,"\t\t%s=%d,\n",str,i+Data[PICM].offset);
    else
    {
     strcat(str,",");
     while(strlen(str)<NAMELEN+5)
       strcat(str," ");
      fprintf(fp,"\t\t%s// %d\n",str,i+Data[PICM].offset);
    }
   }

 fprintf(fp,"\n");

 for (i=0;i<Data[SPRITE].num;i++)
   {
    int end;

    if (ChunkStart[chnk]+Data[ChunkType[chnk]].offset==i+Data[SPRITE].offset)
      {
       fprintf(fp,"\t\t// Lump Start\n");
       chnk++;
      }

    _fmemcpy((char far *)str,(char far *)SpriteNames+i*NAMELEN,50);
    strcat(str,"SPR");
    if (!i)
      fprintf(fp,"\t\t%s=%d,\n",str,i+Data[SPRITE].offset);
    else
    {
     strcat(str,",");
     while(strlen(str)<NAMELEN+5)
       strcat(str," ");
      fprintf(fp,"\t\t%s// %d\n",str,i+Data[SPRITE].offset);
    }
   }

 fprintf(fp,"\n");

 for (i=0;i<NumMisc;i++)
   {
    int end;

    _fmemcpy((char far *)str,(char far *)MiscNames+i*NAMELEN,50);
    if (!i)
      fprintf(fp,"\t\t%s=%d,\n",str,i+Data[TILE32M].offset+Data[TILE32M].num);
    else
    {
     strcat(str,",");
     while(strlen(str)<NAMELEN+5)
       strcat(str," ");
      fprintf(fp,"\t\t%s// %d\n",str,i+Data[TILE32M].offset+Data[TILE32M].num);
    }
   }

 fprintf(fp,"\t\tENUMEND\n\t     } graphicnums;\n");
 fprintf(fp,"\n");

 //
 // output the LUMP defines
 //
 fprintf(fp,"//\n// Data LUMPs\n//\n");

 for (i=0;i<whichchunk;i++)
   {
    int end;

    for (j=0;j<_fstrlen((char far *)ChunkNames+i*CHKNAMELEN);j++)
      if (*(ChunkNames+i*CHKNAMELEN+j)=='\r')
	{
	 *(ChunkNames+i*CHKNAMELEN+j)=0;
	 break;
	}

    _fmemcpy((char far *)str,(char far *)ChunkNames+i*CHKNAMELEN,50);
    strcat(str,"_LUMP_START");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"#define %s%d\n",str,ChunkStart[i]+Data[ChunkType[i]].offset);

    _fmemcpy((char far *)str,(char far *)ChunkNames+i*CHKNAMELEN,50);
    strcat(str,"_LUMP_END");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"#define %s%d\n\n",str,ChunkEnd[i]+Data[ChunkType[i]].offset);
   }

 fprintf(fp,"\n");

 //
 // output the #defines for NUM
 //
 fprintf(fp,"//\n// Amount of each data item\n//\n");

 strcpy(str,"#define NUMCHUNKS    ");
 itoa(numall,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMFONT      ");
 itoa(Data[FONT].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMFONTM     ");
 itoa(Data[FONTM].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMPICS      ");
 itoa(Data[PIC].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMPICM      ");
 itoa(Data[PICM].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMSPRITES   ");
 itoa(Data[SPRITE].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE8     ");
 itoa(Data[TILE8].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE8M    ");
 itoa(Data[TILE8M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE16    ");
 itoa(Data[TILE16].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE16M   ");
 itoa(Data[TILE16M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE32    ");
 itoa(Data[TILE32].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMTILE32M   ");
 itoa(Data[TILE32M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define NUMEXTERNS   ");
 itoa(NumMisc,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 //
 // output the #defines for START
 //
 fprintf(fp,"//\n// File offsets for data items\n//\n");

 //
 // SPECIAL DATA CHUNKS
 //
 start=0;
 if (Data[PIC].num)
   {
    strcpy(str,"#define STRUCTPIC    ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }
 if (Data[PICM].num)
   {
    strcpy(str,"#define STRUCTPICM   ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }
 if (Data[SPRITE].num)
   {
    strcpy(str,"#define STRUCTSPRITE ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T8whichbit)
   {
    strcpy(str,"#define T8MBITARRAY  ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T16whichbit)
   {
    strcpy(str,"#define T16MBITARRAY ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T32whichbit)
   {
    strcpy(str,"#define T32MBITARRAY ");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 //
 // GRAPHIC DATA CHUNKS
 //
 if (start)
   fprintf(fp,"\n");

 strcpy(str,"#define STARTFONT    ");
 ltoa(Data[FONT].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTFONTM   ");
 ltoa(Data[FONTM].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTPICS    ");
 ltoa(Data[PIC].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTPICM    ");
 ltoa(Data[PICM].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTSPRITES ");
 ltoa(Data[SPRITE].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE8   ");
 ltoa(Data[TILE8].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE8M  ");
 ltoa(Data[TILE8M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE16  ");
 ltoa(Data[TILE16].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE16M ");
 ltoa(Data[TILE16M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE32  ");
 ltoa(Data[TILE32].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTTILE32M ");
 ltoa(Data[TILE32M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"#define STARTEXTERNS ");
 ltoa(Data[TILE32M].offset+Data[TILE32M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 //
 // finish up .H
 //
 fprintf(fp,"\n");
 fprintf(fp,"//\n");
 fprintf(fp,"// Thank you for using IGRAB!\n");
 fprintf(fp,"//\n");
 fclose(fp);



 //
 // Create .EQU ASM file
 //

 time(&tblock);

 strcpy(dest,"GFX?_ext.EQU");
 dest[3]=format[0];
 strncpy(dest+5,ext,3);
 unlink(dest);
 if ((fp=fopen(dest,"wt"))==NULL)
   {
    char msg[80]="Couldn't create '";

    strcat(msg,dest);
    strcat(msg,"'!");
    errout(msg);
   }

 fprintf(fp,";=====================================\n");
 fprintf(fp,";\n");
 fprintf(fp,"; Graphics .EQU file for .%s\n",ext);
 fprintf(fp,"; IGRAB-ed on %s",ctime(&tblock));
 fprintf(fp,";\n");
 fprintf(fp,";=====================================\n\n");

 for (i=0;i<Data[PIC].num;i++)
   {
    _fmemcpy((char far *)str,(char far *)PicNames+i*NAMELEN,50);
    strcat(str,"PIC");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=	%d\n",str,i+Data[PIC].offset);
   }

 fprintf(fp,"\n");

 for (i=0;i<Data[PICM].num;i++)
   {
    _fmemcpy((char far *)str,(char far *)PicMNames+i*NAMELEN,50);
    strcat(str,"PICM");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=	%d\n",str,i+Data[PICM].offset);
   }

 fprintf(fp,"\n");

 for (i=0;i<Data[SPRITE].num;i++)
   {
    _fmemcpy((char far *)str,(char far *)SpriteNames+i*NAMELEN,50);
    strcat(str,"SPR");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=	%d\n",str,i+Data[SPRITE].offset);
   }

 fprintf(fp,"\n");

 for (i=0;i<NumMisc;i++)
   {
    _fmemcpy((char far *)str,(char far *)MiscNames+i*NAMELEN,50);
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=	%d\n",str,i+Data[TILE32M].offset+Data[TILE32M].num);
   }

 fprintf(fp,"\n");

 for (i=0;i<whichchunk;i++)
   {
    for (j=0;j<_fstrlen((char far *)ChunkNames+i*CHKNAMELEN);j++)
      if (*(ChunkNames+i*CHKNAMELEN+j)=='\r')
	{
	 *(ChunkNames+i*CHKNAMELEN+j)=0;
	 break;
	}

    _fmemcpy((char far *)str,(char far *)ChunkNames+i*CHKNAMELEN,50);
    strcat(str,"_LUMP_START");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=\x9%d\n",str,ChunkStart[i]+Data[ChunkType[i]].offset);

    _fmemcpy((char far *)str,(char far *)ChunkNames+i*CHKNAMELEN,50);
    strcat(str,"_LUMP_END");
    for (end=(48-strlen(str)-9)/8,j=0;j<end;j++)
      strcat(str,"\x9");
    fprintf(fp,"%s=\x9%d\n\n",str,ChunkEnd[i]+Data[ChunkType[i]].offset);
   }

 fprintf(fp,"\n");

 //
 // output the #defines for NUM
 //
 fprintf(fp,";\n; Amount of each data item\n;\n");

 strcpy(str,"NUMCHUNKS\x9=\x9");
 itoa(numall,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMFONT  \x9=\x9");
 itoa(Data[FONT].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMFONTM  \x9=\x9");
 itoa(Data[FONTM].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMPICS  \x9=\x9");
 itoa(Data[PIC].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMPICM  \x9=\x9");
 itoa(Data[PICM].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMSPRITES  \x9=\x9");
 itoa(Data[SPRITE].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE8  \x9=\x9");
 itoa(Data[TILE8].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE8M  \x9=\x9");
 itoa(Data[TILE8M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE16  \x9=\x9");
 itoa(Data[TILE16].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE16M  \x9=\x9");
 itoa(Data[TILE16M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE32  \x9=\x9");
 itoa(Data[TILE32].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMTILE32M  \x9=\x9");
 itoa(Data[TILE32M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"NUMEXTERN  \x9=\x9");
 itoa(NumMisc,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 //
 // output the #defines for START
 //
 fprintf(fp,";\n; File offsets for data items\n;\n");

 //
 // SPECIAL CHUNKS
 //
 start=0;
 if (Data[PIC].num)
   {
    strcpy(str,"STRUCTPIC  \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }
 if (Data[PICM].num)
   {
    strcpy(str,"STRUCTPICM  \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }
 if (Data[SPRITE].num)
   {
    strcpy(str,"STRUCTSPRITE  \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T8whichbit)
   {
    strcpy(str,"T8MBITARRAY   \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T16whichbit)
   {
    strcpy(str,"T16MBITARRAY  \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 if (bit && T32whichbit)
   {
    strcpy(str,"T32MBITARRAY  \x9=\x9");
    itoa(start,temp,10);
    strcat(str,temp);
    fprintf(fp,"%s\n",str);
    start++;
   }

 //
 // GRAPHIC CHUNKS
 //
 if (start)
   fprintf(fp,"\n");

 strcpy(str,"STARTFONT  \x9=\x9");
 ltoa(Data[FONT].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTFONTM  \x9=\x9");
 ltoa(Data[FONTM].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTPICS  \x9=\x9");
 ltoa(Data[PIC].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTPICM  \x9=\x9");
 ltoa(Data[PICM].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTSPRITES  \x9=\x9");
 ltoa(Data[SPRITE].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE8  \x9=\x9");
 ltoa(Data[TILE8].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE8M  \x9=\x9");
 ltoa(Data[TILE8M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE16  \x9=\x9");
 ltoa(Data[TILE16].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE16M  \x9=\x9");
 ltoa(Data[TILE16M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE32  \x9=\x9");
 ltoa(Data[TILE32].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTTILE32M  \x9=\x9");
 ltoa(Data[TILE32M].offset,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 strcpy(str,"STARTEXTERN  \x9=\x9");
 ltoa(Data[TILE32M].offset+Data[TILE32M].num,temp,10);
 strcat(str,temp);
 fprintf(fp,"%s\n",str);

 //
 // finish up .EQU
 //
 fprintf(fp,"\n");
 fprintf(fp,";\n");
 fprintf(fp,"; Thank you for using IGRAB!\n");
 fprintf(fp,";\n");
 fclose(fp);
}


/////////////////////////////////////////////////////////
//
// Compress all the different data types
//
/////////////////////////////////////////////////////////
void CompressData(void)
{
 //
 // Compress the different data types
 //
 oldhtab=wherex();
 oldvtab=wherey();
 gotoxy(oldhtab,oldvtab);

 CompressFonts();
 CompressPics();
 CompressSprites();
 Compress8();
 Compress16();
 Compress32();

 //
 // MAKE SURE WE WRITE OUT A FINAL "OFFSET"
 // FOR LENGTH-FINDING PURPOSES!
 //
 fileoffs[Data[TILE32M].offset+Data[TILE32M].num]=currentlen;
}


/////////////////////////////////////////////////////////
//
// Compress the FONT/Ms
//
/////////////////////////////////////////////////////////
void CompressFonts(void)
{
 if (Data[FONT].num)
   {
    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");

    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    for (i=0;i<Data[FONT].num;i++)
      {
       long newlen,length=FontOffs[i+1]-FontOffs[i];

       gotoxy(44,wherey());
       printf("%d",i+1);

       constlen+=length;
       oldlen+=length;
       fileoffs[Data[FONT].offset+i]=currentlen;
       *(long huge *)databuffer=length;
       LoadFile("FONT.TMP",lbmscreen,FontOffs[i],length);
       newlen=HuffCompress(lbmscreen,length,databuffer+4);
       SaveFile(grname,databuffer,newlen+4,currentlen);
       currentlen+=newlen+4;
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
       constcomp+=newlen;
      }
    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");

 if (Data[FONTM].num)
   {
    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");

    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    for (i=0;i<Data[FONTM].num;i++)
      {
       long newlen,length=FontMOffs[i+1]-FontMOffs[i];

       gotoxy(44,wherey());
       printf("%d",i+1);

       constlen+=length;
       oldlen+=length;
       fileoffs[Data[FONTM].offset+i]=currentlen;
       *(long huge *)databuffer=length;
       LoadFile("FONTM.TMP",lbmscreen,FontMOffs[i],length);
       newlen=HuffCompress(lbmscreen,length,databuffer+4);
       SaveFile(grname,databuffer,newlen+4,currentlen);
       currentlen+=newlen+4;
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
       constcomp+=newlen;
      }
    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Compress the PIC/Ms
//
/////////////////////////////////////////////////////////
void CompressPics(void)
{
 if (Data[PIC].num)
   {
    long newlen,length;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");

    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    //
    // Check for enough Extended memory for this...
    //
    xmsok=CheckXMSamount("PIC.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    for (i=0;i<Data[PIC].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       length=PicOffs[i+1]-PicOffs[i];
       if (length>comp_size)
	 errout("PIC is too big for the allocated buffer (compression)!");
       constlen+=length;
       oldlen+=length;
       fileoffs[Data[PIC].offset+i]=currentlen;
       *(long huge *)databuffer=length;

       if (!xmsok)
	 LoadFile("PIC.TMP",lbmscreen,PicOffs[i],length);
       else
	 XMSmove(xmsSource,PicOffs[i],0,(long)lbmscreen,length);

       //
       // Munge VGA ModeX graphics?
       //
       if (ModeX)
	 VL_MungePic((unsigned char far *)lbmscreen,(PicTable+i)->width,(PicTable+i)->height);

       newlen=HuffCompress(lbmscreen,length,databuffer+4)+4;

       if (!xmsok)
	 SaveFile(grname,databuffer,newlen,currentlen);
       else
	 XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

       currentlen+=newlen;
       constcomp+=newlen;
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }

    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");

 if (Data[PICM].num)
   {
    long newlen,length;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("PICM.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    for (i=0;i<Data[PICM].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       length=PicMOffs[i+1]-PicMOffs[i];
       if (length>comp_size)
	 errout("PICM is too big for the allocated buffer (compression)!");
       constlen+=length;
       oldlen+=length;
       fileoffs[Data[PICM].offset+i]=currentlen;
       *(long huge *)databuffer=length;

       if (!xmsok)
	 LoadFile("PICM.TMP",lbmscreen,PicMOffs[i],length);
       else
	 XMSmove(xmsSource,PicMOffs[i],0,(long)lbmscreen,length);

       //
       // Munge VGA ModeX graphics?
       //
       if (ModeX)
	 VL_MungePic((unsigned char far *)lbmscreen,PicmTable[i].width,PicmTable[i].height);

       newlen=HuffCompress(lbmscreen,length,databuffer+4)+4;

       if (!xmsok)
	 SaveFile(grname,databuffer,newlen,currentlen);
       else
	 XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

       currentlen+=newlen;
       constcomp+=newlen;
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Compress the SPRITES
//
/////////////////////////////////////////////////////////
void CompressSprites(void)
{
 if (Data[SPRITE].num)
   {
    long newlen,length;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("SPRITE.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    for (i=0;i<Data[SPRITE].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       length=SpriteOffs[i+1]-SpriteOffs[i];
       if (length>comp_size)
	 errout("SPRITE is too big for the allocated buffer (compression)!");
       constlen+=length;
       oldlen+=length;
       fileoffs[Data[SPRITE].offset+i]=currentlen;
       *(long huge *)databuffer=length;

       if (!xmsok)
	 LoadFile("SPRITE.TMP",lbmscreen,SpriteOffs[i],length);
       else
	 XMSmove(xmsSource,SpriteOffs[i],0,(long)lbmscreen,length);

       //
       // Munge VGA ModeX graphics?
       //
       if (ModeX)
	 VL_MungePic((unsigned char far *)lbmscreen,SpriteTable[i].width,SpriteTable[i].height);

       newlen=HuffCompress(lbmscreen,length,databuffer+4)+4;

       if (!xmsok)
	 SaveFile(grname,databuffer,newlen,currentlen);
       else
	 XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

       currentlen+=newlen;
       constcomp+=newlen;
      }
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Compress the TILE8s
//
/////////////////////////////////////////////////////////
void Compress8(void)
{
 if (Data[TILE8].num)
   {
    long newlen,length,spcount=0,clen=0;


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    if (!cmpt8)
      {
       int numsparse;

       gotoxy(30,wherey());
       printf("Compressing...");

       for (numsparse=i=0;i<Data[TILE8].num;i++)
	 numsparse+=Sparse[TILE8*totalobjects+i];

       length=Data[TILE8].graphlen[gmode]*Data[TILE8].num-
	      Data[TILE8].graphlen[gmode]*numsparse;
       oldlen+=length;
       constlen=length;
       fileoffs[Data[TILE8].offset]=currentlen;
       LoadFile("TILE8.TMP",lbmscreen,0,length);
       newlen=HuffCompress(lbmscreen,length,databuffer);
       SaveFile(grname,databuffer,newlen,currentlen);
       currentlen+=newlen;
       constcomp=newlen;
      }
    else
      {
       constlen=constcomp=0;
       gotoxy(30,wherey());
       printf("Compressing...");

       length=Data[TILE8].graphlen[gmode];
       for (i=0;i<Data[TILE8].num;i++)
	 {
	  gotoxy(44,wherey());
	  printf("%d",i+1);

	  if (Sparse[TILE8*totalobjects+i])
	    {
	     fileoffs[Data[TILE8].offset+i]=-1;
	     spcount++;
	    }
	  else
	    {
	     oldlen+=length;
	     constlen+=length;
	     fileoffs[Data[TILE8].offset+i]=currentlen;
	     LoadFile("TILE8.TMP",lbmscreen,clen,length);
	     newlen=HuffCompress(lbmscreen,length,databuffer);
	     SaveFile(grname,databuffer,newlen,currentlen);
	     currentlen+=newlen;
	     clen+=length;
	     constcomp+=newlen;
	    }
	  if ((bioskey(1)>>8)==1)
	    {
	     clrscr();
	     errout("Aborted!");
	    }
	 }
       allsparse+=spcount;
      }
    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");

 if (Data[TILE8M].num)
   {
    long clen=0,newlen,length,spcount=0;


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    if (!cmpt8)
      {
       int numsparse;

       gotoxy(30,wherey());
       printf("Compressing...");

       for (numsparse=i=0;i<Data[TILE8M].num;i++)
	 numsparse+=Sparse[TILE8M*totalobjects+i];

       length=Data[TILE8M].graphlen[gmode]*Data[TILE8M].num-
	      Data[TILE8M].graphlen[gmode]*numsparse;
       constlen=length;
       oldlen+=length;
       fileoffs[Data[TILE8M].offset]=currentlen;
       LoadFile("TILE8M.TMP",lbmscreen,0,length);
       newlen=HuffCompress(lbmscreen,length,databuffer);
       SaveFile(grname,databuffer,newlen,currentlen);
       currentlen+=newlen;
       constcomp=newlen;
      }
    else
      {
       constlen=constcomp=0;
       gotoxy(30,wherey());
       printf("Compressing...");

       length=Data[TILE8M].graphlen[gmode];
       for (i=0;i<Data[TILE8M].num;i++)
	 {
	  gotoxy(44,wherey());
	  printf("%d",i+1);

	  if (Sparse[TILE8M*totalobjects+i])
	    {
	     fileoffs[Data[TILE8M].offset+i]=-1;
	     spcount++;
	    }
	  else
	    {
	     constlen+=length;
	     oldlen+=length;
	     fileoffs[Data[TILE8M].offset+i]=currentlen;
	     LoadFile("TILE8M.TMP",lbmscreen,clen,length);
	     newlen=HuffCompress(lbmscreen,length,databuffer);
	     SaveFile(grname,databuffer,newlen,currentlen);
	     currentlen+=newlen;
	     clen+=length;
	     constcomp+=newlen;
	    }
	  if ((bioskey(1)>>8)==1)
	    {
	     clrscr();
	     errout("Aborted!");
	    }
	 }
       allsparse+=spcount;
      }
    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Compress the TILE16s
//
/////////////////////////////////////////////////////////
void Compress16(void)
{
 if (Data[TILE16].num)
   {
    long clen=0,newlen,length,spcount=0;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("TILE16.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    length=Data[TILE16].graphlen[gmode];
    for (i=0;i<Data[TILE16].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       if (Sparse[TILE16*totalobjects+i])
	 {
	  fileoffs[Data[TILE16].offset+i]=-1;
	  spcount++;
	 }
       else
	 {
	  constlen+=length;
	  oldlen+=length;
	  fileoffs[Data[TILE16].offset+i]=currentlen;

	  if (!xmsok)
	    LoadFile("TILE16.TMP",lbmscreen,clen,length);
	  else
	    XMSmove(xmsSource,clen,0,(long)lbmscreen,length);

	  newlen=HuffCompress(lbmscreen,length,databuffer);

	  if (!xmsok)
	    SaveFile(grname,databuffer,newlen,currentlen);
	  else
	    XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

	  currentlen+=newlen;
	  clen+=length;
	  constcomp+=newlen;
	 }
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }

    allsparse+=spcount;
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");

 if (Data[TILE16M].num)
   {
    long clen=0,newlen,length,spcount=0;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("TILE16M.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    length=Data[TILE16M].graphlen[gmode];
    for (i=0;i<Data[TILE16M].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       if (Sparse[TILE16M*totalobjects+i])
	 {
	  fileoffs[Data[TILE16M].offset+i]=-1;
	  spcount++;
	 }
       else
	 {
	  constlen+=length;
	  oldlen+=length;
	  fileoffs[Data[TILE16M].offset+i]=currentlen;

	  if (!xmsok)
	    LoadFile("TILE16M.TMP",lbmscreen,clen,length);
	  else
	    XMSmove(xmsSource,clen,0,(long)lbmscreen,length);

	  newlen=HuffCompress(lbmscreen,length,databuffer);

	  if (!xmsok)
	    SaveFile(grname,databuffer,newlen,currentlen);
	  else
	    XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

	  currentlen+=newlen;
	  clen+=length;
	  constcomp+=newlen;
	 }
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }

    allsparse+=spcount;
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Compress the TILE32s
//
/////////////////////////////////////////////////////////
void Compress32(void)
{
 if (Data[TILE32].num)
   {
    long clen=0,newlen,length,spcount=0;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("TILE32.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    length=Data[TILE32].graphlen[gmode];
    for (i=0;i<Data[TILE32].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       if (Sparse[TILE32*totalobjects+i])
	 {
	  fileoffs[Data[TILE32].offset+i]=-1;
	  spcount++;
	 }
       else
	 {
	  constlen+=length;
	  oldlen+=length;
	  fileoffs[Data[TILE32].offset+i]=currentlen;

	  if (!xmsok)
	    LoadFile("TILE32.TMP",lbmscreen,clen,length);
	  else
	    XMSmove(xmsSource,clen,0,(long)lbmscreen,length);

	  newlen=HuffCompress(lbmscreen,length,databuffer);

	  if (!xmsok)
	    SaveFile(grname,databuffer,newlen,currentlen);
	  else
	    XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

	  currentlen+=newlen;
	  clen+=length;
	  constcomp+=newlen;
	 }
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }

    allsparse+=spcount;
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");

 if (Data[TILE32M].num)
   {
    long clen=0,newlen,length,spcount=0;

    constlen=constcomp=0;
    gotoxy(30,wherey());
    printf("Compressing...");


    //
    // Stick Id Software ID bytes in!!!
    //
    SaveFile(grname,"!ID!",4,currentlen);
    currentlen+=4;

    xmsok=CheckXMSamount("TILE32M.TMP",&xmsSource,&xmsDest);
    if (xmsok)
      {
       printf("       (XMS cache)");
       startlen=currentlen;
      }

    length=Data[TILE32M].graphlen[gmode];
    for (i=0;i<Data[TILE32M].num;i++)
      {
       gotoxy(44,wherey());
       printf("%d",i+1);

       if (Sparse[TILE32M*totalobjects+i])
	 {
	  fileoffs[Data[TILE32M].offset+i]=-1;
	  spcount++;
	 }
       else
	 {
	  constlen+=length;
	  oldlen+=length;
	  fileoffs[Data[TILE32M].offset+i]=currentlen;

	  if (!xmsok)
	    LoadFile("TILE32M.TMP",lbmscreen,clen,length);
	  else
	    XMSmove(xmsSource,clen,0,(long)lbmscreen,length);

	  newlen=HuffCompress(lbmscreen,length,databuffer);

	  if (!xmsok)
	    SaveFile(grname,databuffer,newlen,currentlen);
	  else
	    XMSmove(0,(long)databuffer,xmsDest,currentlen-startlen,newlen);

	  currentlen+=newlen;
	  clen+=length;
	  constcomp+=newlen;
	 }
       if ((bioskey(1)>>8)==1)
	 {
	  clrscr();
	  errout("Aborted!");
	 }
      }

    allsparse+=spcount;
    FreeXMSbuffs(&xmsSource,&xmsDest,grname,startlen,currentlen-startlen);

    gotoxy(30,wherey());
    printf("Compressed %lu to %lu.",constlen,constcomp);
    clreol();
   }
 cprintf("\n\r");
}


/////////////////////////////////////////////////////////
//
// Setup the FinishUp function
//
/////////////////////////////////////////////////////////
void SetupFinish(void)
{
 //
 // HANDLE "END" KEYWORD
 //
 if (lumpactive)
   errout("You didn't ENDLUMP before ENDing your script!");


 if (maskscreen!=NULL)
   farfree((void far *)maskscreen);
 if (lbmscreen!=NULL)
   farfree((void far *)lbmscreen);

 lbmscreen=(char huge *)farmalloc(comp_size);

 if ((fileoffs=(long huge *)farmalloc((totalobjects+1)*4L))==NULL)
   errout("Not enough memory to allocate massive FILEOFF array!");

 if (!begin)
   errout("How can you END when you haven't even BEGUN?");

 //
 // See if Extended memory is present
 //
 xms=0;
 if (!InitXMS())
   xms=1;

 //
 // Compress everything
 //
 if (!noshow)
   {
    settext();
    DispStatusScreen();
   }

 clrscr();
 vtab=wherey();
 cprintf("FONTs    grabbed: %d\n\r",Data[FONT].num);
 cprintf("FONTMs   grabbed: %d\n\r",Data[FONTM].num);
 cprintf("PICs     grabbed: %d\n\r",Data[PIC].num);
 cprintf("PICMs    grabbed: %d\n\r",Data[PICM].num);
 cprintf("SPRITEs  grabbed: %d\n\r",Data[SPRITE].num);
 cprintf("TILE8s   grabbed: %d\n\r",Data[TILE8].num);
 cprintf("TILE8Ms  grabbed: %d\n\r",Data[TILE8M].num);
 cprintf("TILE16s  grabbed: %d\n\r",Data[TILE16].num);
 cprintf("TILE16Ms grabbed: %d\n\r",Data[TILE16M].num);
 cprintf("TILE32s  grabbed: %d\n\r",Data[TILE32].num);
 cprintf("TILE32Ms grabbed: %d\n\r",Data[TILE32M].num);
 cprintf("MISC DATA import: %d",NumMisc);
 gotoxy(2,vtab);

 grname[0]=format[0];
 grname[1]=0;
 strcat(grname,"GAGRAPH.");
 strcat(grname,ext);
}


/////////////////////////////////////////////////////////
//
// Create the Fileoffs array & count remaining databytes
//
/////////////////////////////////////////////////////////
void CreateOffsets(void)
{
 CountBytes((unsigned char huge *)PicTable,Data[PIC].num*sizeof(PicStruct));
 CountBytes((unsigned char huge *)PicmTable,Data[PICM].num*sizeof(PicStruct));
 CountBytes((unsigned char huge *)&SpriteTable,Data[SPRITE].num*sizeof(SprStruct));

 Huffmanize();	// create the dictionary
 //
 // create pointer-table indexes
 //
 start=(Data[PIC].num>0)+
       (Data[PICM].num>0)+
       (Data[SPRITE].num>0)+
       (bit && T8whichbit)+
       (bit && T16whichbit)+
       (bit && T32whichbit);

 Data[FONT].offset=start;
 Data[FONTM].offset=Data[FONT].offset+Data[FONT].num;
 Data[PIC].offset=Data[FONTM].offset+Data[FONTM].num;
 Data[PICM].offset=Data[PIC].offset+Data[PIC].num;
 Data[SPRITE].offset=Data[PICM].offset+Data[PICM].num;
 Data[TILE8].offset=Data[SPRITE].offset+Data[SPRITE].num;

 if (!cmpt8)
   {
    Data[TILE8M].offset=Data[TILE8].offset+1*(Data[TILE8].num>0);// tile8's are special
    Data[TILE16].offset=Data[TILE8M].offset+1*(Data[TILE8M].num>0);
   }
 else
   {
    Data[TILE8M].offset=Data[TILE8].offset+Data[TILE8].num;	// now they're not!
    Data[TILE16].offset=Data[TILE8M].offset+Data[TILE8M].num;
   }

 Data[TILE16M].offset=Data[TILE16].offset+Data[TILE16].num;
 Data[TILE32].offset=Data[TILE16M].offset+Data[TILE16M].num;
 Data[TILE32M].offset=Data[TILE32].offset+Data[TILE32].num;

 if (Data[TILE32M].offset+Data[TILE32M].num>totalobjects)
   {
    char tmpstr[]="ERROR! Not enough space allocated for OFFSETTABLE. Talk to John R.!\nYou need space for ",
	poop[10];

    ltoa(Data[TILE32M].offset+Data[TILE32M].num,poop,10);
    strcat(tmpstr,poop);
    strcat(tmpstr," objects, pal.");
    clrscr();
    errout(tmpstr);
   }

 for (i=0;i<totalobjects+1;i++)
   fileoffs[i]=0;
}


/////////////////////////////////////////////////////////
//
// Compress all the STRUCTS and BIT ARRAYS
//
/////////////////////////////////////////////////////////
void CompressSpecial(void)
{
 start=0;

 //
 // Compress the SPECIAL CHUNKS
 //
 if (Data[PIC].num)
   {
    long newlen,length=Data[PIC].num*sizeof(PicStruct);

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)PicTable,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
 if (Data[PICM].num)
   {
    long newlen,length=Data[PICM].num*sizeof(PicStruct);

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)PicmTable,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
 if (Data[SPRITE].num)
   {
    long newlen,length=Data[SPRITE].num*sizeof(SprStruct);

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)&SpriteTable,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
 if (bit && T8whichbit)
   {
    long newlen,length=(T8whichbit+7)/8;

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)T8bit,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
 if (bit && T16whichbit)
   {
    long newlen,length=(T16whichbit+7)/8;

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)T16bit,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
 if (bit && T32whichbit)
   {
    long newlen,length=(T32whichbit+7)/8;

    fileoffs[start]=currentlen;
    *(long huge *)databuffer=length;
    newlen=HuffCompress((char huge *)T32bit,length,databuffer+4);
    SaveFile(grname,databuffer,newlen+4,currentlen);
    currentlen+=newlen+4;
    start++;
   }
}


/////////////////////////////////////////////////////////
//
// Compress the Miscellaneous Data
//
/////////////////////////////////////////////////////////
void CompressMisc(void)
{
 int start,i;
 long len,newlen,constcomp,constlen;
 char temp[NAMELEN];


 if (!NumMisc)
   return;

 constlen=constcomp=0;
 gotoxy(30,wherey());
 printf("Compressing...");

 start=Data[TILE32M].offset+Data[TILE32M].num;
 for (i=0;i<NumMisc;i++)
   {
    gotoxy(44,wherey());
    printf("%d",i+1);

    _fstrcpy((char far *)temp,(char far *)MiscFNames+i*NAMELEN);
    fileoffs[i+start]=currentlen;
    len=filelen(temp);
    constlen+=len;
    LoadFile(temp,lbmscreen,0,0);
    *(long huge *)databuffer=len;
    newlen=HuffCompress(lbmscreen,len,databuffer+4)+4;
    SaveFile(grname,databuffer,newlen,currentlen);
    constcomp+=newlen;
    currentlen+=newlen;
   }

 gotoxy(30,wherey());
 printf("Compressed %lu to %lu.",constlen,constcomp);
 clreol();

 // MAKE SURE WE STICK THE LAST ONE IN!
 fileoffs[start+NumMisc]=currentlen;
}


/////////////////////////////////////////////////////////
//
// Create the ?GAHEAD,?GADICT,TEDINFO? files
//
/////////////////////////////////////////////////////////
void CreateGraphFiles(void)
{
 //
 // Create "xGAHEAD.EXT"
 //
 strcpy(dest,"?GAHEAD.");
 dest[0]=format[0];
 strcat(dest,ext);
 dest[12]=0;

 //
 // Save "xGAHEAD.EXT"
 //
 strcpy(headname,dest);
 unlink(dest);
 if ((handle=_creat(dest,FA_ARCH))==-1)
   {
    char msg[80]="Couldn't create '";

    strcat(msg,dest);
    strcat(msg,"!");
    errout(msg);
   }

 if (!cmpt8)
   numall=Data[FONT].num+
	  Data[FONTM].num+
	  (Data[TILE8].num>0)+
	  (Data[TILE8M].num>0)+
	  Data[TILE16].num+
	  Data[TILE16M].num+
	  Data[TILE32].num+
	  Data[TILE32M].num+
	  Data[PIC].num+
	  Data[PICM].num+
	  Data[SPRITE].num+
	  (Data[PIC].num>0)+
	  (Data[PICM].num>0)+
	  (Data[SPRITE].num>0)+
	  (bit && T8whichbit)+
	  (bit && T16whichbit)+
	  (bit && T32whichbit)+
	  NumMisc;

 else
   numall=Data[FONT].num+
	  Data[FONTM].num+
	  Data[TILE8].num+
	  Data[TILE8M].num+
	  Data[TILE16].num+
	  Data[TILE16M].num+
	  Data[TILE32].num+
	  Data[TILE32M].num+
	  Data[PIC].num+
	  Data[PICM].num+
	  Data[SPRITE].num+
	  (Data[PIC].num>0)+
	  (Data[PICM].num>0)+
	  (Data[SPRITE].num>0)+
	  (bit && T8whichbit)+
	  (bit && T16whichbit)+
	  (bit && T32whichbit)+
	  NumMisc;

 close(handle);
 //
 // 3-byte Offsets?
 //
 if (!Do4offs)
 {
  unsigned i;
  char huge *newoffs;

  if ((newoffs=farmalloc(3L*(numall+1)))==NULL)
    errout("Not enough memory to create 3-byte OFFSETS!");

  for (i=0;i<numall+1;i++)
    *(long huge *)(newoffs+3L*i)=fileoffs[i];

  SaveFile(dest,(char huge *)newoffs,3L*(numall+1),0);
 }
 else
   SaveFile(dest,(char huge *)fileoffs,4*(numall+1),0);

 //
 // Create EGADICT.ext file
 //
 {
  char name[14]="?GADICT.";

  name[0]=format[0];
  strcat(name,ext);
  SaveFile(name,(char huge *)&nodearray,sizeof(nodearray),0);
 }

 //
 // Create GFXINFO?.ext file
 //
 {
  char name[14]="GFXINFO?.";
  InfoStruct huge *infofile;

  if((infofile=(InfoStruct huge *)farmalloc(sizeof(InfoStruct)))==NULL)
    errout("Not enough memory to create GFXINFO file!");

  _fmemset((void far *)infofile,0,sizeof(InfoStruct));
  infofile->num8=Data[TILE8].num;
  infofile->num8m=Data[TILE8M].num;
  infofile->num16=Data[TILE16].num;
  infofile->num16m=Data[TILE16M].num;
  infofile->num32=Data[TILE32].num;
  infofile->num32m=Data[TILE32M].num;

  infofile->off8=Data[TILE8].offset;
  infofile->off8m=Data[TILE8M].offset;
  infofile->off16=Data[TILE16].offset;
  infofile->off16m=Data[TILE16M].offset;
  infofile->off32=Data[TILE32].offset;
  infofile->off32m=Data[TILE32M].offset;

  infofile->numpics=Data[PIC].num;
  infofile->numpicm=Data[PICM].num;
  infofile->numsprites=Data[SPRITE].num;

  infofile->offpic=Data[PIC].offset;
  infofile->offpicm=Data[PICM].offset;
  infofile->offsprites=Data[SPRITE].offset;

  infofile->numexterns=NumMisc;
  infofile->offexterns=Data[TILE32M].offset+Data[TILE32M].num;

  name[7]=format[0];
  strcat(name,ext);
  SaveFile(name,(char huge *)infofile,sizeof(InfoStruct),0);
 }
}


/////////////////////////////////////////////////////////
//
// Create OBJ files if requested
//
/////////////////////////////////////////////////////////
void CreateOBJs(void)
{
 //
 // GENERATE 2 .OBJ FILES?
 //
 if (genobj)
   {
    char newname[24],pubname[25],farname[25],dname[13];

    //
    // Header
    //
    strcpy(newname,ext);
    strcat(newname,format);
    strcat(newname,"HEAD.OBJ");

    strcpy(pubname,"_");
    strcat(pubname,format);
    strcat(pubname,"GAhead");

    strcpy(farname,format);
    strcat(farname,"GA_grafixheader");

    if (MakeOBJ(headname,newname,pubname,FARDATA,farname)>0)
      errout("Error making Header OBJ file!");


    //
    // Dictionary
    //
    strcpy(dname,format);
    strcat(dname,"GADICT.");
    strcat(dname,ext);

    strcpy(newname,ext);
    strcat(newname,format);
    strcat(newname,"DICT.OBJ");

    strcpy(pubname,"_");
    strcat(pubname,format);
    strcat(pubname,"GAdict");

    if (MakeOBJ(dname,newname,pubname,DATA,"")>0)
      errout("Error making Dictionary OBJ file!");

    gotoxy(2,19);
    printf("OBJ files created.\n");
   }
}


/////////////////////////////////////////////////////////
//
// Update the IGRAB grabwindow
//
/////////////////////////////////////////////////////////
void UpdateWindow(void)
{
 window(1,1,80,25);
 gotoxy(30,21);
 cprintf("%lu",oldlen);
 gotoxy(64,21);
 cprintf("%lu",currentlen);
 gotoxy(25,22);
 cprintf("%d",allsparse);
 gotoxy(59,22);
 cprintf("%d",allsparse*4);

 gotoxy(60,6);
 cprintf("%d",numall);
}



/*
=================
=
= VL_MungePic
=
=================
*/
void VL_MungePic (unsigned char far *source, unsigned width, unsigned height)
{
	unsigned	x,y,plane,size,pwidth;
	unsigned char	far *temp, far *dest, far *srcline;

	size = width*height;

	if (width&3)
		errout ("VL_MungePic: Not divisable by 4!\n");

//
// copy the pic to a temp buffer
//
	temp = (unsigned char far *)farmalloc (size);
	if (!temp)
		errout ("Non enough memory for munge buffer!\n");

	_fmemcpy (temp,source,size);

//
// munge it back into the original buffer
//
	dest = source;
	pwidth = width/4;

	for (plane=0;plane<4;plane++)
	{
		srcline = temp;
		for (y=0;y<height;y++)
		{
			for (x=0;x<pwidth;x++)
				*dest++ = *(srcline+x*4+plane);
			srcline+=width;
		}
	}

	farfree (temp);
}

