////////////////////////////////////////////////////////////
//
// IGRAB - Multi-Graphics-Mode, Multi-Format, Fast BitMap Grabber
// by John Romero (C) 1991 Id Software
//
////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

//
// VARS
//
time_t tblock;
LBMtype CurrentLBM;
PicStruct far *PicTable,far *PicmTable;
SprStruct SpriteTable[MAXSPRITES];
FILE *fp;
grabtype type;
graphtype gmode;
DataStruct Data[11]={{0,{0,0,0,0},0},		// font
		     {0,{0,0,0,0},0},   	// fontm
		     {0,{16,32,64,0},0},	// tile8
		     {0,{32,40,128,0},0},	// tile8m
		     {0,{64,128,256,0},0},	// tile16
		     {0,{128,160,512,0},0},	// tile16m
		     {0,{256,512,1024,0},0},	// tile32
		     {0,{512,640,2048,0},0},	// tile32m
		     {0,{0,0,0,0},0},		// pic
		     {0,{0,0,0,0},0},		// picm
		     {0,{0,0,0,0},0}};		// sprite

extern char typestr[5];

char picname[64],huge *Sparse;

char huge *PicNames,huge *SpriteNames,huge *PicMNames,huge *ChunkNames,
     huge *MiscNames,huge *MiscFNames;

unsigned char format[2],scriptname[64],dest[13],string[80],ext[10],huge *lbmscreen,
     huge *databuffer,huge *maskscreen,ScreenColor,
     typelist[17][10]={"FONT","FONTM","TILE8","TILE8","TILE8M","TILE8M","TILE16","TILE16",
       "TILE16M","TILE16M","TILE32","TILE32","TILE32M","TILE32M","PIC","PICM","SPRITE"},
     huge *T8bit,huge *T16bit,huge *T32bit,path[64],NumMisc;
long offset,size,fsize,tile8off,tile8moff,tile16off,tile16moff,
     tile32off,tile32moff,picoff,picmoff,spriteoff,fontoff,fontmoff,
     FontOffs[MAXFONT],far *PicOffs,SpriteOffs[MAXSPRITES],FontMOffs[MAXFONT],
     far *PicMOffs,bufmax,comp_size;
unsigned begin,j,i,gotstr,frac,temp,keycheck,compress,
	 end,gottiles,globalx,globaly,globalmaxh,nostacking,noshow,shifts,
	 fastgrab,totalobjects,leavetmp,cmpt8,genobj,ChunkStart[MAXSPRITES/8],
	 ChunkEnd[MAXSPRITES/8],whichchunk,ChunkType[MAXSPRITES/8],bit,
	 T8whichbit,T16whichbit,T32whichbit,setbit,lumpactive,SkipToStart,
	 Do4offs,ModeX,PicAmount;

int handle;


////////////////////////////////////////////////////////////
//
// IGRAB start
//
////////////////////////////////////////////////////////////
void main(int argc,char **argv)
{
 //
 // make sure we clear the Huffman compression array
 //

 settext();
 randomize();
 memset(&counts,0,sizeof(counts));
 DeleteTmpFiles();

 clrscr();
 textcolor(15);
 textbackground(1);
 cprintf("%s\r\n",TITLESTR);
 textbackground(0);

 if (argc<2)
   {
    printf("Expected format: IGRAB <format> [options] <filename>\n");
    printf("<format>	= -E for EGA, -V for VGA graphics mode, -C for CGA.\n");
    printf("<options>	= -NS for No Stacking of graphics while displaying\n");
    printf("		= -OFF for turning graphics display off\n");
    printf("		= -F for FastGrab (no keypress per screen)\n");
    printf("		= -NUM=<value> to specify a larger offset array if\n");
    printf("		  you happen to have more than %d objects combined.\n",MAXOFFS);
    printf("		= -L to leave all .TMP files intact after compression\n");
    printf("		= -T8 to compress TILE8/Ms individually\n");
    printf("		= -OBJ to generate a linkable .OBJ header file\n");
    printf("		= -BIT to generate a packed-bit array for all\n");
    printf("		  foreground tiles that don't need a mask.\n");
    printf("		= -PATH=<string> to specify a pathname where\n");
    printf("		  the .LBM screens can be found. Only for loading.\n");
    printf("		= -4 to generate 4-byte chunk offsets, instead of 3.\n");
    printf("		= -HUGE to grab EGA 640x350 graphics\n");
    printf("		= -PICS=<value> to grab more than %d PIC/Ms\n",MAXPICS);
    printf("		= -X to munge VGA PIC/Ms and SPRITEs\n");
    printf("<filename>	= filename of the IGRAB Script File\n");
    exit(1);
   }

 PicAmount=MAXPICS;
 totalobjects=MAXOFFS;
 maskscreen=lbmscreen=NULL;
 bufmax=BUFFERSIZE;
 comp_size=MAXCOMPSIZE;

 /////////////////////////////////////////////////////////
 //
 // CommandLine "parser"
 //
 /////////////////////////////////////////////////////////
 for (i=1;i<argc;i++)
   {
    argv[i]=strupr(argv[i]);
    if (argv[i][0]=='/' || argv[i][0]=='-')
      argv[i]++;

    if ((!strcmp(argv[i],"E")) ||
	(!strcmp(argv[i],"V")) ||
	(!strcmp(argv[i],"C")))
      format[0]=argv[i][0];
    else
    if (!strcmp(argv[i],"NS"))
      nostacking=1;
    else
    if (!strcmp(argv[i],"OFF"))
      noshow=1;
    else
    if (!strcmp(argv[i],"F"))
      fastgrab=1;
    else
    if (!strncmp(argv[i],"NUM=",4))
      totalobjects=atoi(argv[i]+4);
    else
    if (!strncmp(argv[i],"PICS=",5))
      PicAmount=atoi(argv[i]+5);
    else
    if (!strcmp(argv[i],"L"))
      leavetmp=1;
    else
    if (!strcmp(argv[i],"T8"))
      cmpt8=1;
    else
    if (!strcmp(argv[i],"OBJ"))
      genobj=1;
    else
    if (!strcmp(argv[i],"BIT"))
      bit=1;
    else
    if (!strcmp(argv[i],"4"))
      Do4offs=1;
    else
    if (!strncmp(argv[i],"HUGE",4))
      {
       bufmax=BUFFER1SIZE;
       comp_size=MAXCOMP1SIZE;
      }
    else
    if (!strcmp(argv[i],"X"))
      ModeX=1;
    else
    if (!strncmp(argv[i],"PATH=",5))
      {
       strcpy(path,argv[i]+5);
       strcat(path,"\\");
      }
    else
      strcpy(scriptname,argv[i]);
   }


 //
 // DONE PARSING COMMAND-LINE ARGUMENTS
 //
 if (format[0]!='C' && format[0]!='E' && format[0]!='V')
   errout("The graphics type can ONLY be CGA,EGA or VGA!\n");

 switch(format[0])
   {
    case 'C': gmode=CGA; break;
    case 'E': gmode=EGA; break;
    case 'V': gmode=VGA;
   }

 if (gmode!=VGA && ModeX)
   errout("Can't munge non-VGA graphics!\n");

 if (noshow)	// if screen is OFF, just blast thru!
   fastgrab=1;

 //
 // ALLOCATE BIT-ARRAY MEMORY
 //
 if (bit)
   {
    if (cmpt8)
      if ((T8bit=(char huge *)farmalloc(NUMBITARRAY))==NULL)
	errout("Not enough memory for BITARRAY for TILE8's!");
    if ((T16bit=(char huge *)farmalloc(NUMBITARRAY))==NULL)
      errout("Not enough memory for BITARRAY for TILE16's!");
    if ((T32bit=(char huge *)farmalloc(NUMBITARRAY))==NULL)
      errout("Not enough memory for BITARRAY for TILE32's!");

    for (i=0;i<NUMBITARRAY;i++)
      T16bit[i]=T32bit[i]=0;
   }

 if ((fp=fopen(scriptname,"rb"))==NULL)
   {
    char string[80]="Having problems opening ";

    strcat(string,scriptname);
    strcat(string,"!");
    errout(string);
   }

 //
 // Initialize stuff
 //

 PicTable=farmalloc(PicAmount*sizeof(PicStruct));
 PicmTable=farmalloc(PicAmount*sizeof(PicStruct));
 PicOffs=farmalloc(PicAmount*4);
 PicMOffs=farmalloc(PicAmount*4);
 shifts=4;

 //
 // The "8L" takes up "font/m,tile8/m,tile16/m & tile32/m spots
 //
 if ((Sparse=(char huge *)farmalloc(8L*totalobjects))==NULL)
   errout("Not enough memory for SPARSE table allocation!");

 if ((databuffer=(char huge *)farmalloc(bufmax))==NULL)
   errout("Memory allocation for data buffer failed!");

 frac=1;
 keycheck=end=offset=begin=gotstr=0;

 if (!noshow)
   switch(format[0])
     {
      case 'C': videomode(2); break;
      case 'E': videomode(4); break;
      case 'V': videomode(8); break;
     }

 textbackground(1);
 textcolor(14);
 DispStatusScreen();
 window(2,8,78,19);

 PicNames=(char huge *)farmalloc((long)PicAmount*NAMELEN);
 PicMNames=(char huge *)farmalloc((long)PicAmount*NAMELEN);
 SpriteNames=(char huge *)farmalloc((long)MAXSPRITES*NAMELEN);
 ChunkNames=farmalloc((long)MAXSPRITES/8*CHKNAMELEN);
 MiscNames=farmalloc((long)MAXPICS*NAMELEN);
 MiscFNames=farmalloc((long)MAXPICS*FNAMELEN);

 if (!PicNames || !PicMNames || !SpriteNames || !ChunkNames || !MiscNames || !MiscFNames)
   errout("Not enough memory for Name arrays!");

 /////////////////////////////////////////////////////////
 //
 // Parse script file
 //
 /////////////////////////////////////////////////////////

 while(bioskey(1))
   bioskey(0);

 while(1)
   {
    int ready,grabbed;

    //
    // FIRST, SEARCH FOR "START ..."
    //

    ready=0;

    do {
	fpos_t position;

	fgetpos(fp,&position);
	fsetpos(fp,&position);

	if (!gotstr)
	  fgets(string,80,fp);
	else
	  gotstr=0;

	if (!end)
	  for (i=0;i<strlen(string);i++)
	    if (string[i]=='\x1a')
	      errout("You didn't put an END at the end of your script!");

	strupr(string);

	//
	// HANDLE "BEGIN"
	//
	if (!strncmp(string,"BEGIN",5))
	  {
	   begin=1;
	   strcpy(ext,string+6);
	   for (i=0;i<strlen(ext);i++)
	     if (ext[i]=='\r' || ext[i]=='\n')
	       ext[i]=0;
	   continue;
	  }

	//
	// HANDLE "FRAC"
	//

	if (!strncmp(string,"FRAC",4))
	  {
	   frac=atoi(string+5);
	   continue;
	  }

	//
	// HANDLE "SHIFTS"
	//

	if (!strncmp(string,"SHIFTS",6))
	  {
	   shifts=atoi(string+7);
	   continue;
	  }

	//
	// HANDLE "START"
	//

	if (!strncmp(string,"START",5))
	  {
	   if (!begin)
	     errout("You must first BEGIN!");

	   grabbed=0;

	   FindType(string+6);
	   SkipToStart=0;
	   ready++;
	   continue;
	  }

	//
	// HANDLE "END"
	//

	if (!strncmp(string,"END",3))
	  FinishUp();

	//
	// HANDLE "EXTERN"
	//
	// THE "MiscNames" HOLD THE USER NAMES,
	// THE "MiscFNames" HOLD THE FILENAMES OF THE DATA
	//

	if (!strncmp(string,"EXTERN",6))
	  {
	   char *s1,*s2,comp[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			       "abcdefghijklmnopqrstuvwxyz_",
	   comp1[]=",\x9\r\n",*s3,huge *tempdata;
	   long len;


	   s1=strpbrk(string+6,comp);
	   s2=strpbrk(s1,comp1);
	   *s2++=0;
	   s2=strpbrk(s2,comp);
	   s3=strpbrk(s2,comp1);
	   *s3=0;

	   _fstrcpy((char far *)MiscNames+NumMisc*NAMELEN,(char far *)s1);
	   _fstrcpy((char far *)MiscFNames+NumMisc*NAMELEN,(char far *)s2);

	   len=filelen(s2);
	   tempdata=farmalloc(len);
	   if (access(s2,0))
	   {
	    char errst[60]="The external file '";
	    strcat(errst,s2);
	    strcat(errst,"' cannot be found!");
	    errout(errst);
	   }

	   LoadFile(s2,tempdata,0,0);
	   CountBytes((unsigned char huge *)tempdata,len);
	   farfree((void far *)tempdata);

	   NumMisc++;
	  }


       } while(!ready);

    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    //
    // FOUND "START". TIME TO LOAD SCREENS & GRAB STUFF
    //
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    offset=ready=0;
    if (bioskey(1))
      bioskey(0);

    do {
	fpos_t position;

	fgetpos(fp,&position);
	fsetpos(fp,&position);

	fgets(string,80,fp);
	strupr(string);

	/////////////////////////////////////////////
	//
	// HANDLE "EXTERN"
	//
	// THE "MiscNames" HOLD THE USER NAMES,
	// THE "MiscFNames" HOLD THE FILENAMES OF THE DATA
	//
	/////////////////////////////////////////////

	if (!strncmp(string,"EXTERN",6))
	  {
	   char *s1,*s2,comp[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			       "abcdefghijklmnopqrstuvwxyz_",
	   comp1[]=",\x9\r\n",*s3,huge *tempdata;
	   long len;


	   s1=strpbrk(string+6,comp);
	   s2=strpbrk(s1,comp1);
	   *s2++=0;
	   s2=strpbrk(s2,comp);
	   s3=strpbrk(s2,comp1);
	   *s3=0;

	   _fstrcpy((char far *)MiscNames+NumMisc*NAMELEN,(char far *)s1);
	   _fstrcpy((char far *)MiscFNames+NumMisc*NAMELEN,(char far *)s2);

	   len=filelen(s2);
	   tempdata=farmalloc(len);

	   if (access(s2,0))
	   {
	    char errst[60]="The external file '";
	    strcat(errst,s2);
	    strcat(errst,"' cannot be found!");
	    errout(errst);
	   }

	   LoadFile(s2,tempdata,0,0);
	   CountBytes((unsigned char huge *)tempdata,len);
	   farfree((void far *)tempdata);

	   NumMisc++;
	  }

	/////////////////////////////////////////////
	//
	// FRAC keyword
	//
	/////////////////////////////////////////////
	if (!strncmp(string,"FRAC",4))
	  {
	   frac=atoi(string+5);
	   continue;
	  }

	/////////////////////////////////////////////
	//
	// LOAD keyword
	//
	/////////////////////////////////////////////
	if (!strncmp(string,"LOAD",4))
	  {
	   LoadKeyword(string+5,grabbed);
	   globalx=globaly=globalmaxh=0;
	   continue;
	  }

	/////////////////////////////////////////////
	//
	// HANDLE "LUMP"
	//
	/////////////////////////////////////////////

	if (!strncmp(string,"LUMP",4))
	  {
	   int newtype[17]={FONT,FONTM,TILE8,TILE8,TILE8M,TILE8M,
		TILE16,TILE16,TILE16M,TILE16M,TILE32,TILE32,
		TILE32M,TILE32M,PIC,PICM,SPRITE};

	   if (lumpactive)
	     {
	      char erstr[200]="You tried to start a LUMP before ending the current lump ";
	      _fstrcat((char far *)erstr,(char far *)ChunkNames+whichchunk*CHKNAMELEN);
	      strcat(erstr,"!");
	      errout(erstr);
	     }

	   _fmemcpy((char far *)ChunkNames+whichchunk*CHKNAMELEN,(char far *)string+5,CHKNAMELEN);
	   *(ChunkNames+whichchunk*CHKNAMELEN+CHKNAMELEN-1)=0;
	   ChunkType[whichchunk]=newtype[type];
	   ChunkStart[whichchunk]=Data[ChunkType[whichchunk]].num;
	   lumpactive=1;
	   continue;
	  }

	/////////////////////////////////////////////
	//
	// HANDLE "SHIFTS"
	//
	/////////////////////////////////////////////

	if (!strncmp(string,"SHIFTS",6))
	  {
	   shifts=atoi(string+7);
	   continue;
	  }

	/////////////////////////////////////////////
	//
	// HANDLE "ENDLUMP"
	//
	/////////////////////////////////////////////

	if (!strncmp(string,"ENDLUMP",7))
	  {
	   if (!lumpactive)
	     errout("You can't ENDLUMP when one isn't started!");

	   ChunkEnd[whichchunk]=Data[ChunkType[whichchunk]].num-1;
	   whichchunk++;
	   lumpactive=0;
	   continue;
	  }

	/////////////////////////////////////////////
	//
	// GRAB keyword
	//
	/////////////////////////////////////////////
	if (!strncmp(string,"GRAB",4))
	  {
	   sound(1500);
	   switch(type)
	     {
	      case FONTTYPE:
	      case FONTMTYPE:
		GrabFont(type);
		grabbed++;
		break;
	      case TILE8TYPE:
	      case TILE8MTYPE:
	      case ALT8TYPE:
	      case ALT8MTYPE:
	      case TILE16TYPE:
	      case TILE16MTYPE:
	      case ALT16TYPE:
	      case ALT16MTYPE:
	      case TILE32TYPE:
	      case TILE32MTYPE:
	      case ALT32TYPE:
	      case ALT32MTYPE:
		if (!maskscreen &&
		    (type==TILE8MTYPE || type==ALT8MTYPE ||
		     type==TILE16MTYPE || type==ALT16MTYPE ||
		     type==TILE32MTYPE || type==ALT32MTYPE))
		  errout("You haven't loaded a mask screen for TILEM yet!");

		nosound();
		GrabTile(type);
		grabbed++;
		break;
	      case PICTYPE:
	      case PICMTYPE:
		if (!maskscreen && type==PICMTYPE)
		  errout("You haven't loaded a mask screen for PICM yet!");

		GrabPics(type);
		grabbed++;
		break;
	      case SPRITETYPE:
		GrabSprites();
		grabbed++;
		break;
	     }
	   nosound();
	   continue;
	  }


	/////////////////////////////////////////////
	//
	// Check for end of script or start of another datatype
	//
	/////////////////////////////////////////////
	if (!strncmp(string,"START",5) ||
	    !strncmp(string,"END",3))
	  {
	   //
	   // save off data for current type, if any is left
	   //
	   FlushData();
	   if (!strncmp(string,"END",3))
	     end=1;

	   gotstr++;
	   ready++;
	  }

       } while (!ready);

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
       }
    }
   }
}
// END OF MAIN


////////////////////////////////////////////////////////////
//
// Check for out-of-bounds in databuffer
//
////////////////////////////////////////////////////////////
void CheckBuffer(void)
{
 if (offset>bufmax)
   {
    settext();
    printf("ERROR!: The %cGA data buffer is too small!\n",format[0]);
    printf("Contact John Romero for a full refund!\n");
    nosound();
    exit(1);
   }

 //
 // IF MORE THAN 80% OF DATABUFFER IS FULL, FLUSH IT!
 //
 if (offset>4L*(bufmax/5))
   FlushData();
}


////////////////////////////////////////////////////////////
//
// Flush all PIC,PICM or SPRITE data to disk!
//
////////////////////////////////////////////////////////////
void FlushData(void)
{
 if (offset)
   {
    CountBytes(databuffer,offset);
    switch(type)
      {
       case PICTYPE:
	 AddDataToFile("PIC");
	 break;
       case PICMTYPE:
	 AddDataToFile("PICM");
	 break;
       case SPRITETYPE:
	 AddDataToFile("SPRITE");
	 break;
       case TILE8TYPE:
       case ALT8TYPE:
	 AddDataToFile("TILE8");
	 break;
       case TILE8MTYPE:
       case ALT8MTYPE:
	 AddDataToFile("TILE8M");
	 break;
       case TILE16TYPE:
       case ALT16TYPE:
	 AddDataToFile("TILE16");
	 break;
       case TILE16MTYPE:
       case ALT16MTYPE:
	 AddDataToFile("TILE16M");
	 break;
       case TILE32TYPE:
       case ALT32TYPE:
	 AddDataToFile("TILE32");
	 break;
       case TILE32MTYPE:
       case ALT32MTYPE:
	 AddDataToFile("TILE32M");
	 break;

       default:
	 errout("Tried to FLUSHDATA for a type not supported!");
      }
   }
}


////////////////////////////////////////////////////////////
//
// Delete all the temp files we're gonna use
//
////////////////////////////////////////////////////////////
void DeleteTmpFiles(void)
{
 unlink("FONT.TMP");
 unlink("FONTM.TMP");
 unlink("TILE8.TMP");
 unlink("TILE8M.TMP");
 unlink("TILE16.TMP");
 unlink("TILE16M.TMP");
 unlink("TILE32.TMP");
 unlink("TILE32M.TMP");
 unlink("PIC.TMP");
 unlink("PICM.TMP");
 unlink("SPRITE.TMP");
}


////////////////////////////////////////////////////////////
//
// Add data in DATABUFFER to temp file
//
////////////////////////////////////////////////////////////
void AddDataToFile(char *filename)
{
 int handle;
 long olength;
 char newname[20];

 strcpy(newname,filename);
 strcat(newname,".TMP");

 if ((handle=open(newname,O_BINARY))==-1)
   olength=0;
 else
   {
    olength=filelength(handle);
    close(handle);
   }

 SaveFile(newname,databuffer,offset,olength);
 offset=0;
}


////////////////////////////////////////////////////////////
//
// Set text mode
//
////////////////////////////////////////////////////////////
void settext(void)
{
 _AX=3;
 geninterrupt(0x10);
}


////////////////////////////////////////////////////////////
//
// Save a *LARGE* file from a FAR buffer!
// by John Romero (C) 1991 Id Software
//
////////////////////////////////////////////////////////////
void SaveFile(char *filename,char huge *buffer, long size,long offset)
{
 struct dfree disk;
 unsigned int handle,buf1,buf2,offlo,offhi,sizelo,sizehi;

 getdfree(0,&disk);
 if ((long)disk.df_avail*disk.df_bsec*disk.df_sclus<size)
   errout("Not enough disk space!");

 buf1=FP_OFF(buffer);
 buf2=FP_SEG(buffer);
 offlo=offset&0xffff;
 offhi=offset>>16;
 sizelo=size&0xffff;
 sizehi=size>>16;

asm		mov	ax,offlo
asm		or	ax,offhi
asm		jz	CREATEIT

asm		mov	dx,filename
asm		mov	ax,3d02h		// OPEN w/handle (read only)
asm		int	21h
asm		jc	out

asm		mov	handle,ax

asm		mov	bx,handle
asm		mov     dx,offlo
asm		mov	cx,offhi
asm		mov	ax,4200h
asm		int	21h			// SEEK (to file offset)
asm		jc	out

asm		jmp	DOSAVE

CREATEIT:

asm		mov	dx,filename
asm		mov	ax,3c00h		// CREATE w/handle (read only)
asm		xor	cx,cx
asm		int	21h
asm		jc	out

asm		mov	handle,ax

DOSAVE:

asm		cmp	WORD PTR sizehi,0		// larger than 1 segment?
asm		je	L2

L1:

asm		push	ds
asm		mov	bx,handle
asm		mov	cx,8000h
asm		mov	dx,buf1
asm		mov	ax,buf2
asm		mov	ds,ax
asm		mov	ah,40h			// WRITE w/handle
asm		int	21h
asm		pop	ds

asm		add	WORD PTR buf2,800h		// bump ptr up 1/2 segment
asm		sub	WORD PTR sizelo,8000h		// done yet?
asm		sbb	WORD PTR sizehi,0
asm		cmp	WORD PTR sizehi,0
asm		ja	L1
asm		cmp	WORD PTR sizelo,8000h
asm		jae	L1

L2:

asm		push	ds
asm		mov	bx,handle
asm		mov	cx,sizelo
asm		mov	dx,buf1
asm		mov	ax,buf2
asm		mov	ds,ax
asm		mov	ah,40h			// WRITE w/handle
asm		int	21h
asm		pop	ds

out:

asm		mov	bx,handle		// CLOSE w/handle
asm		mov	ah,3eh
asm		int	21h

}

////////////////////////////////////////////////////////////
//
// Load a *LARGE* file into a FAR buffer!
// by John Romero (C) 1991 Id Software
//
////////////////////////////////////////////////////////////
unsigned long LoadFile(char *filename,char huge *buffer,long offset,long size)
{
 unsigned handle,flength1=0,flength2=0,buf1,buf2,len1,len2,
	  rsize1,rsize2,roffset1,roffset2;

 rsize1=size&0xffff;
 rsize2=size>>16;
 roffset1=offset&0xffff;
 roffset2=offset>>16;
 buf1=FP_OFF(buffer);
 buf2=FP_SEG(buffer);

asm		mov	dx,filename
asm		mov	ax,3d00h		// OPEN w/handle (read only)
asm		int	21h
asm		jnc	L_0
asm		jmp	out

L_0:

asm		mov	handle,ax
asm		mov	bx,ax
asm		xor	cx,cx
asm		xor	dx,dx
asm		mov	ax,4202h
asm		int	21h			// SEEK (find file length)
asm		jc	out

asm		mov	flength1,ax
asm		mov	len1,ax
asm		mov	flength2,dx
asm		mov	len2,dx

asm		mov	ax,rsize1		// was a size specified?
asm		or	ax,rsize1
asm		jz	LOADALL

asm		mov	ax,rsize1		// only load size requested
asm		mov	len1,ax
asm		mov	ax,rsize2
asm		mov	len2,ax

LOADALL:

asm		mov	bx,handle
asm		mov     dx,roffset1
asm		mov	cx,roffset2
asm		mov	ax,4200h
asm		int	21h			// SEEK (to file offset)
asm		jc	out

asm		cmp	WORD PTR len2,0			// MULTI-SEGMENTAL?
asm		je      L_2

L_1:

asm		push	ds
asm		mov	bx,handle
asm		mov	cx,8000h		// read 32K chunks
asm		mov	dx,buf1
asm		mov	ax,buf2
asm		mov	ds,ax
asm		mov	ah,3fh			// READ w/handle
asm		int	21h
asm		pop	ds
asm		jc	out

asm		add	WORD PTR buf2,800h
asm		sub	WORD PTR len1,8000h
asm		sbb	WORD PTR len2,0
asm		cmp	WORD PTR len2,0
asm		ja	L_1
asm		cmp	WORD PTR len1,8000h
asm		jae	L_1

L_2:

asm		push	ds
asm		mov	bx,handle
asm		mov	cx,len1
asm		mov	dx,buf1
asm		mov	ax,buf2
asm		mov	ds,ax
asm		mov	ah,3fh			// READ w/handle
asm		int	21h
asm		pop	ds
asm		jmp	exit

out:

asm		mov	WORD PTR flength2,0
asm		mov	WORD PTR flength1,0

exit:

asm		mov	bx,handle		// CLOSE w/handle
asm		mov	ah,3eh
asm		int	21h

return (flength2*0x10000+flength1);

}


///////////////////////////////////////////////////
//
// error routine
//
///////////////////////////////////////////////////
void errout(char *string)
{
 if (!noshow)
   {
    settext();
    DispStatusScreen();
   }

 window(1,1,79,24);
 sound(1500);
 gotoxy(1,24);
 printf("%s",string);
 poke(0,0x41a,peek(0,0x41c));	// clear keyboard
 DeleteTmpFiles();
 nosound();
 exit(1);
}


///////////////////////////////////////////////////
//
// set a graphics mode
//
///////////////////////////////////////////////////
void videomode(int planes)
{
 switch (planes)
   {
    case 2: { _AX=4; geninterrupt(0x10); break; }
    case 4: {
	      _AX=0x0d;
	      geninterrupt(0x10);
	      break;
	    }
    case 8: { _AX=0x13; geninterrupt(0x10); break; }
   }
}


////////////////////////////////////////////////////
//
// Create an OBJ linkable file from any type of datafile
//
// Exit:
//  0 = everything's a-ok!
// -1 = file not found
// -2 = file >64K
//
////////////////////////////////////////////////////
int MakeOBJ(char *filename,char *destfilename,char *public,segtype whichseg,char *farname)
{
 char THEADR[17]={0x80,14,0,12,32,32,32,32,32,32,32,32,32,32,32,32,0},
      COMENT[18]={0x88,0,0,0,0,'M','a','k','e','O','B','J',' ','v','1','.','1',0},
      LNAMES[42]={0x96,0,0,
		  6,'D','G','R','O','U','P',
		  5,'_','D','A','T','A',
		  4,'D','A','T','A',
		  0,
		  5,'_','T','E','X','T',
		  4,'C','O','D','E',
		  8,'F','A','R','_','D','A','T','A'},
      SEGDEF[9]={0x98,7,0,0x48,0,0,2,3,4},	// for .DATA
      SEGDEF1[9]={0x98,7,0,0x48,0,0,5,6,4},	// for .CODE
      SEGDEF2[9]={0x98,7,0,0x60,0,0,8,7,4},	// for .FARDATA
      GRPDEF[7]={0x9a,4,0,1,0xff,1,0x61},
      MODEND[5]={0x8a,2,0,0,0x74};

 unsigned i,j,flag,handle;
 long fsize,offset,loffset,temp,amtleft,amount,offset1;
 char huge *dblock,huge *block;


 //
 // Need to compute the CHECKSUM in the COMENT field
 // (so if the "MakeOBJ..." string is modified, the CHECKSUM
 //  will be correct).
 //
 COMENT[1]=sizeof(COMENT)-3;
 for (flag=i=0;i<sizeof(COMENT);i++)
    flag+=COMENT[i];
 COMENT[sizeof(COMENT)-1]=(flag^0xff)+1;

 if ((handle=open(filename,O_BINARY))==NULL)
   return -1;

 fsize=filelength(handle);
 close(handle);
 if (fsize>0x10000L)		// BIGGER THAN 1 SEG = ERROR!
   return -2;

 block=(char huge *)farmalloc(fsize);
 if (block==NULL)
   errout("No memory to create OBJ!");
 LoadFile(filename,block,0,0);	// LOAD FILE IN
 offset=0;

 dblock=(char huge *)farmalloc(0x10000L);
 if (dblock==NULL)
   errout("No memory to create OBJ!");

 ////////////////////////////////////////////////////
 //
 // INSERT HEADER RECORD
 //
 movedata(_DS,FP_OFF(&THEADR),FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(THEADR));
 movedata(FP_SEG(filename),FP_OFF(filename),
	  FP_SEG(dblock),FP_OFF(dblock)+offset+4,strlen(filename));
 offset+=sizeof(THEADR);


 ////////////////////////////////////////////////////
 //
 // INSERT COMMENT RECORD
 //
 movedata(_DS,FP_OFF(COMENT),FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(COMENT));
 offset+=sizeof(COMENT);


 ////////////////////////////////////////////////////
 //
 // INSERT START OF LIST-OF-NAMES RECORD
 //
 loffset=offset;
 movedata(_DS,FP_OFF(LNAMES),FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(LNAMES));
 offset+=sizeof(LNAMES);

 // If it's a .FARDATA segment, we need to insert the segment name!
 if (whichseg==FARDATA)
   {
    *(dblock+offset)=strlen(farname);
    movedata(FP_SEG(farname),FP_OFF(farname),
	FP_SEG(dblock),FP_OFF(dblock)+offset+1,strlen(farname));
    offset+=strlen(farname)+1;
   }

 // Now, finish the List-Of-Names record by creating
 // the CHECKSUM and LENGTH
 temp=offset;
 offset=offset-loffset-2;
 *(int huge *)(dblock+loffset+1)=offset;
 offset=temp;

 // Now, figure out the CHECKSUM of the record
 for (flag=i=0;i<(offset-loffset);i++)
   flag+=*(dblock+i+loffset);
 *(dblock+offset)=(flag^0xff)+1;
 offset++;


 ////////////////////////////////////////////////////
 //
 // CREATE SEGMENT DEFINITION RECORD
 //
 loffset=offset;
 temp=fsize;
 switch(whichseg)
 {
  case DATA:
    movedata(FP_SEG(&SEGDEF),FP_OFF(&SEGDEF),
	     FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(SEGDEF));
    *(int huge *)(dblock+offset+4)=temp;
    offset+=sizeof(SEGDEF);
    break;
  case CODE:
    movedata(FP_SEG(&SEGDEF1),FP_OFF(&SEGDEF1),
	     FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(SEGDEF1));
    *(int huge *)(dblock+offset+4)=temp;
    offset+=sizeof(SEGDEF1);
    break;
  case FARDATA:
    movedata(FP_SEG(&SEGDEF2),FP_OFF(&SEGDEF2),
	     FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(SEGDEF2));
    *(int huge *)(dblock+offset+4)=temp;
    offset+=sizeof(SEGDEF2);
    break;
 }

 // CHECKSUM
 for (flag=0,i=loffset;i<offset;i++)
   flag+=*(dblock+i);
 *(dblock+offset)=(flag^0xff)+1;
 offset++;


 ////////////////////////////////////////////////////
 //
 // CREATE GROUP DEFINITION RECORD
 //
 switch(whichseg)
 {
  case DATA:
  case CODE:
    movedata(FP_SEG(&GRPDEF),FP_OFF(&GRPDEF),
	     FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(GRPDEF));
    offset+=sizeof(GRPDEF);
 }


 ////////////////////////////////////////////////////
 //
 // CREATE PUBLIC DEFINITION RECORD
 //
 loffset=offset;
 *(dblock+offset)=0x90;		// PUBDEF ID
 offset+=3;			// point to public base, skip length
 *(dblock+offset)=1;		// group index=1
 *(dblock+offset+1)=1;		// segment index=1
 offset+=2;			// point to public name

 temp=0;
 movedata(FP_SEG(public),FP_OFF(public),
	  FP_SEG(dblock),FP_OFF(dblock)+offset+1,strlen(public));
 *(dblock+offset)=strlen(public);
 offset+=strlen(public)+1;
 *(int huge *)(dblock+offset)=0;	// public offset within segment
 offset+=2;
 *(dblock+offset)=0;		// type index
 offset++;

 // LENGTH
 temp=offset-loffset-2;
 *(int huge *)(dblock+loffset+1)=temp;
 offset++;

 // CHECKSUM
 for (flag=0,i=loffset;i<offset;i++)
   flag+=*(dblock+i);
 *(dblock+offset)=(flag^0xff)+1;


 ////////////////////////////////////////////////////
 //
 // DATA RECORD(S). YUCK.
 //

 amtleft=fsize;
 amount=1024;
 for (i=0;i<(fsize+1023)/1024;i++)
   {
    offset1=offset;
    if (amtleft<1024)
      amount=amtleft;
    //
    // RECORD HEADER
    //
    *(dblock+offset)=0xa0;			// LEDATA ID
    *(int huge *)(dblock+offset+1)=amount+4;	// length of record
    offset+=3;
    *(dblock+offset)=1;				// segment index
    *(int huge *)(dblock+offset+1)=i*1024;	// index into segment
    offset+=3;
    //
    // LOAD DATA IN
    //
    LoadFile(filename,(char huge *)dblock+offset,i*1024,amount);
    offset+=amount;
    //
    // CHECKSUM!
    //
    for (flag=0,j=offset1;j<offset;j++)
      flag+=*(dblock+j);
    *(dblock+offset)=(flag^0xff)+1;
    offset++;

    amtleft-=1024;
   }

 ////////////////////////////////////////////////////
 //
 // MODULE END! YES!
 //
 movedata(FP_SEG(&MODEND),FP_OFF(&MODEND),FP_SEG(dblock),FP_OFF(dblock)+offset,sizeof(MODEND));
 offset+=sizeof(MODEND);

 //
 // Save the little puppy out!
 //
 SaveFile(destfilename,(char huge *)dblock,offset,0);
 farfree((void far *)block);
 farfree((void far *)dblock);
 return 0;
}
