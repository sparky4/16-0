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
//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1w
//	By Jason Blochowiak
//	Open Watcom port by sparky4
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "src/lib/16_in.h"
#pragma	hdrstop

#ifndef	__16_SD__
static word far* clockw= (word far*) 0x046C; /* 18.2hz clock */
longword	TimeCount;
#endif

#define	KeyInt		9	// The keyboard ISR number

//
// mouse constants
//
#define	MReset		0
#define	MButtons	3
#define	MDelta		11

#define	MouseInt	0x33
#define	Mouse(x)	_AX = x,geninterrupt(MouseInt)

//
// joystick constants
//
#define	JoyScaleMax		32768
#define	JoyScaleShift	8
#define	MaxJoyValue		5000

/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
boolean			MousePresent;
boolean			JoysPresent[MaxJoys];
boolean			JoyPadPresent;


// 	Global variables
		boolean		Keyboard[NumCodes];
		boolean		Paused;
		char		LastASCII;
		ScanCode	LastScan;
		KeyboardDef	KbdDefs[MaxKbds] = {0x1d,0x38,0x47,0x48,0x49,0x4b,0x4d,0x4f,0x50,0x51};

		JoystickDef	JoyDefs[MaxJoys];
		ControlType	Controls[MaxPlayers];

//old definition
//KbdDefs = {0x1d,0x38,/*0x47,*/0x48,/*0x49,*/0x4b,0x4d,/*0x4f,*/0x50/*,0x51*/};

		dword		MouseDownCount;

#if DEMO0
		Demo		DemoMode = demo_Off;
#endif
		byte _seg	*DemoBuffer;
		word		DemoOffset,DemoSize;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/

#ifdef __cplusplus		/* Function must be declared C style */
extern "C" {
#endif

	boolean		Keyboard[NumCodes];
	boolean		Paused;
	char		LastASCII;
	ScanCode	LastScan;

static	byte        far ASCIINames[] =		// Unshifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',0  ,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					},
					far ShiftNames[] =		// Shifted ASCII for scan codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'!','@','#','$','%','^','&','*','(',')','_','+',8  ,9  ,	// 0
	'Q','W','E','R','T','Y','U','I','O','P','{','}',13 ,0  ,'A','S',	// 1
	'D','F','G','H','J','K','L',':',34 ,'~',0  ,'|','Z','X','C','V',	// 2
	'B','N','M','<','>','?',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					far SpecialNames[] =	// ASCII for 0xe0 prefixed codes
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
	0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
					},
					*ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					far ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,0x00
					},
					*ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
	"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
	"Down","Left","Right",""
					};

static	boolean		IN_Started;
static	boolean		CapsLock;
static	ScanCode	CurCode,LastCode;

static	Direction	DirTable[] =		// Quick lookup for total direction
					{
						dir_NorthWest,	dir_North,	dir_NorthEast,
						dir_West,		dir_None,	dir_East,
						dir_SouthWest,	dir_South,	dir_SouthEast
					};
#ifdef __cplusplus
}
#endif

static	void			(*INL_KeyHook)(void);
static	void interrupt	(*OldKeyVect)(void);
static	char			*ParmStrings[] = {"nojoys","nomouse",nil};
static	void			INL_StartKbd(void);

//	Internal routines

///////////////////////////////////////////////////////////////////////////
//
//	INL_KeyService() - Handles a keyboard interrupt (key up/down)
//
///////////////////////////////////////////////////////////////////////////
/*static */void interrupt
INL_KeyService(void)
{
static	boolean	special;
		byte	k,c,
				temp;

	k = inportb(0x60);	// Get the scan code

	// Tell the XT keyboard controller to clear the key
	outportb(0x61,(temp = inportb(0x61)) | 0x80);
	outportb(0x61,temp);

	if (k == 0xe0)		// Special key prefix
		special = true;
	else if (k == 0xe1)	// Handle Pause key
		Paused = true;
	else
	{
		if (k & 0x80)	// Break code
		{
			k &= 0x7f;

// DEBUG - handle special keys: ctl-alt-delete, print scrn

			Keyboard[k] = false;
		}
		else			// Make code
		{
			LastCode = CurCode;
			CurCode = LastScan = k;
			Keyboard[k] = true;

			if (special)
				c = SpecialNames[k];
			else
			{
				if (k == sc_CapsLock)
				{
					CapsLock ^= true;
					// DEBUG - make caps lock light work
				}

				if (Keyboard[sc_LShift] || Keyboard[sc_RShift])	// If shifted
				{
					c = ShiftNames[k];
					if ((c >= 'A') && (c <= 'Z') && CapsLock)
						c += 'a' - 'A';
				}
				else
				{
					c = ASCIINames[k];
					if ((c >= 'a') && (c <= 'z') && CapsLock)
						c -= 'a' - 'A';
				}
			}
			if (c)
				LastASCII = c;
		}

		special = false;
	}

	if (INL_KeyHook && !special)
		INL_KeyHook();
#ifdef __DEBUG_InputMgr__
	if(dbg_testkeyin > 0) printf("%c	%u	[0x%x %u]	%u\n", c, c, k, k, Keyboard[k]);
#endif
	outportb(0x20,0x20);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseDelta() - Gets the amount that the mouse has moved from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static void
INL_GetMouseDelta(int *x,int *y)
{
	Mouse(MDelta);
	*x = _CX;
	*y = _DX;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetMouseButtons() - Gets the status of the mouse buttons from the
//		mouse driver
//
///////////////////////////////////////////////////////////////////////////
static word
INL_GetMouseButtons(void)
{
	word	buttons;

	Mouse(MButtons);
	buttons = _BX;
	return(buttons);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyAbs() - Reads the absolute position of the specified joystick
//
///////////////////////////////////////////////////////////////////////////
void
IN_GetJoyAbs(word joy,word *xp,word *yp)
{
	byte	xb,yb,
			xs,ys;
	word	x,y;

	x = y = 0;
	xs = joy? 2 : 0;		// Find shift value for x axis
	xb = 1 << xs;			// Use shift value to get x bit mask
	ys = joy? 3 : 1;		// Do the same for y axis
	yb = 1 << ys;

// Read the absolute joystick values
	__asm {
		pushf				// Save some registers
		push	si
		push	di
		cli					// Make sure an interrupt doesn't screw the timings


		mov		dx,0x201
		in		al,dx
		out		dx,al		// Clear the resistors

		mov		ah,[xb]		// Get masks into registers
		mov		ch,[yb]

		xor		si,si		// Clear count registers
		xor		di,di
		xor		bh,bh		// Clear high byte of bx for later

		push	bp			// Don't mess up stack frame
		mov		bp,MaxJoyValue
#ifdef __BORLANDC__
	}
#endif
loo:
#ifdef __BORLANDC__
	__asm {
#endif
		in		al,dx		// Get bits indicating whether all are finished

		dec		bp			// Check bounding register
		jz		done		// We have a silly value - abort

		mov		bl,al		// Duplicate the bits
		and		bl,ah		// Mask off useless bits (in [xb])
		add		si,bx		// Possibly increment count register
		mov		cl,bl		// Save for testing later

		mov		bl,al
		and		bl,ch		// [yb]
		add		di,bx

		add		cl,bl
		jnz		loo		// If both bits were 0, drop out
#ifdef __BORLANDC__
	}
#endif
done:
#ifdef __BORLANDC__
	__asm {
#endif
		pop		bp

		mov		cl,[xs]		// Get the number of bits to shift
		shr		si,cl		//  and shift the count that many times

		mov		cl,[ys]
		shr		di,cl

		mov		[x],si		// Store the values into the variables
		mov		[y],di

		pop		di
		pop		si
		popf				// Restore the registers
	}

	*xp = x;
	*yp = y;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void INL_GetJoyDelta(word joy,int *dx,int *dy/*,boolean adaptive*/)
{
	word		x,y;
//	longword	time;	unused
	JoystickDef	*def;
static	longword	lasttime;
#ifndef	__16_SD__
TimeCount = *clockw;
#endif

	IN_GetJoyAbs(joy,&x,&y);
	def = JoyDefs + joy;

	if (x < def->threshMinX)
	{
		if (x < def->joyMinX)
			x = def->joyMinX;

		x = -(x - def->threshMinX);
		x *= def->joyMultXL;
		x >>= JoyScaleShift;
		*dx = (x > 127)? -127 : -x;
	}
	else if (x > def->threshMaxX)
	{
		if (x > def->joyMaxX)
			x = def->joyMaxX;

		x = x - def->threshMaxX;
		x *= def->joyMultXH;
		x >>= JoyScaleShift;
		*dx = (x > 127)? 127 : x;
	}
	else
		*dx = 0;

	if (y < def->threshMinY)
	{
		if (y < def->joyMinY)
			y = def->joyMinY;

		y = -(y - def->threshMinY);
		y *= def->joyMultYL;
		y >>= JoyScaleShift;
		*dy = (y > 127)? -127 : -y;
	}
	else if (y > def->threshMaxY)
	{
		if (y > def->joyMaxY)
			y = def->joyMaxY;

		y = y - def->threshMaxY;
		y *= def->joyMultYH;
		y >>= JoyScaleShift;
		*dy = (y > 127)? 127 : y;
	}
	else
		*dy = 0;
//	if (adaptive)
//	{
//		time = (TimeCount - lasttime) / 2;
//		if (time)
//		{
//			if (time > 8)
//				time = 8;
//			*dx *= time;
//			*dy *= time;
//		}
//	}
	lasttime = TimeCount;
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_GetJoyButtons() - Returns the button status of the specified
//		joystick
//
///////////////////////////////////////////////////////////////////////////
static word
INL_GetJoyButtons(word joy)
{
register	word	result;

	result = inportb(0x201);	// Get all the joystick buttons
	result >>= joy? 6 : 4;	// Shift into bits 0-1
	result &= 3;				// Mask off the useless bits
	result ^= 3;
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyButtonsDB() - Returns the de-bounced button status of the
//		specified joystick
//
///////////////////////////////////////////////////////////////////////////
word
IN_GetJoyButtonsDB(word joy)
{
	longword	lasttime;
	word		result1,result2;
#ifndef __16_SD__
TimeCount = *clockw;
#endif

	do
	{
		result1 = INL_GetJoyButtons(joy);
		lasttime = TimeCount;
		while (TimeCount == lasttime)
		{};
		result2 = INL_GetJoyButtons(joy);
	} while (result1 != result2);
	return(result1);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartKbd() - Sets up my keyboard stuff for use
//
///////////////////////////////////////////////////////////////////////////
static void
INL_StartKbd(void)
{
	INL_KeyHook = NULL;	// Clear key hook

	IN_ClearKeysDown();

	OldKeyVect = getvect(KeyInt);//IN_KbdLED();
	setvect(KeyInt,INL_KeyService);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutKbd() - Restores keyboard control to the BIOS
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutKbd(void)
{
	pokeb(0x40,0x17,peekb(0x40,0x17) & 0xfaf0);	// Clear ctrl/alt/shift flags

	setvect(KeyInt,OldKeyVect);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartMouse() - Detects and sets up the mouse
//
///////////////////////////////////////////////////////////////////////////
static boolean
INL_StartMouse(void)
{
#if 0
	if (getvect(MouseInt))
	{
		Mouse(MReset);
		if (_AX == 0xffff)
			return(true);
	}
	return(false);
#else
	byte far *vector;


	if ((vector=MK_FP(peek(0,0x33*4+2),peek(0,0x33*4)))==NULL)
	return false;

	if (*vector == 207)
		return false;

	Mouse(MReset);
	return true;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutMouse() - Cleans up after the mouse
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutMouse(void)
{
}

//
//	INL_SetJoyScale() - Sets up scaling values for the specified joystick
//
static void
INL_SetJoyScale(word joy)
{
	JoystickDef	*def;

	def = &JoyDefs[joy];
	def->joyMultXL = JoyScaleMax / (def->threshMinX - def->joyMinX);
	def->joyMultXH = JoyScaleMax / (def->joyMaxX - def->threshMaxX);
	def->joyMultYL = JoyScaleMax / (def->threshMinY - def->joyMinY);
	def->joyMultYH = JoyScaleMax / (def->joyMaxY - def->threshMaxY);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetupJoy() - Sets up thresholding values and calls INL_SetJoyScale()
//		to set up scaling values
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetupJoy(word joy,word minx,word maxx,word miny,word maxy)
{
	word		d,r;
	JoystickDef	*def;

	def = &JoyDefs[joy];

	def->joyMinX = minx;
	def->joyMaxX = maxx;
	r = maxx - minx;
	d = r / 3;
	def->threshMinX = ((r / 2) - d) + minx;
	def->threshMaxX = ((r / 2) + d) + minx;

	def->joyMinY = miny;
	def->joyMaxY = maxy;
	r = maxy - miny;
	d = r / 3;
	def->threshMinY = ((r / 2) - d) + miny;
	def->threshMaxY = ((r / 2) + d) + miny;

	INL_SetJoyScale(joy);
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_StartJoy() - Detects & auto-configures the specified joystick
//					The auto-config assumes the joystick is centered
//
///////////////////////////////////////////////////////////////////////////
static boolean
INL_StartJoy(word joy)
{
	word		x,y;

	IN_GetJoyAbs(joy,&x,&y);

	if
	(
		((x == 0) || (x > MaxJoyValue - 10))
	||	((y == 0) || (y > MaxJoyValue - 10))
	)
		return(false);
	else
	{
		IN_SetupJoy(joy,0,x * 2,0,y * 2);
		return(true);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_ShutJoy() - Cleans up the joystick stuff
//
///////////////////////////////////////////////////////////////////////////
static void
INL_ShutJoy(word joy)
{
	JoysPresent[joy] = false;
}

//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	boolean	checkjoys,checkmouse;
	word	i;

	if (IN_Started)
		return;

	checkjoys = true;
	checkmouse = true;
	for (i = 1;i < _argc;i++)
	{
		switch (US_CheckParm(_argv[i],ParmStrings))
		{
		case 0:
			checkjoys = false;
			break;
		case 1:
			checkmouse = false;
			break;
		}
	}

	INL_StartKbd();
	MousePresent = checkmouse? INL_StartMouse() : false;

	for (i = 0;i < MaxJoys;i++)
		JoysPresent[i] = checkjoys? INL_StartJoy(i) : false;

	IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Default() - Sets up default conditions for the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Default(boolean gotit,ControlType in)
{
	if
	(
		(!gotit)
	|| 	((in == ctrl_Joystick1) && !JoysPresent[0])
	|| 	((in == ctrl_Joystick2) && !JoysPresent[1])
	|| 	((in == ctrl_Mouse) && !MousePresent)
	)
		in = ctrl_Keyboard1;
/*	KbdDefs[0].button0 = 0x1c;
	KbdDefs[0].button1 = 0x38;
	KbdDefs[0].upleft = 0x47;
	KbdDefs[0].up = 0x48;
	KbdDefs[0].upright = 0x49;
	KbdDefs[0].left = 0x4b;
	KbdDefs[0].right = 0x4d;
	KbdDefs[0].downleft = 0x4f;
	KbdDefs[0].down = 0x50;
	KbdDefs[0].downright = 0x51;*/
	IN_SetControlType(0,in);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Shutdown(void)
{
	word	i;

	if (!IN_Started)
		return;

	INL_ShutMouse();
	for (i = 0;i < MaxJoys;i++)
		INL_ShutJoy(i);
	INL_ShutKbd();

	IN_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetKeyHook() - Sets the routine that gets called by INL_KeyService()
//			everytime a real make/break code gets hit
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetKeyHook(void (*hook)())
{
	INL_KeyHook = hook;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	//int	i;

	LastScan = sc_None;
	LastASCII = key_None;
	memset (Keyboard,0,sizeof(Keyboard));
}

///////////////////////////////////////////////////////////////////////////
//
//	INL_AdjustCursor() - Internal routine of common code from IN_ReadCursor()
//
///////////////////////////////////////////////////////////////////////////
static void
INL_AdjustCursor(CursorInfo *info,word buttons,int dx,int dy)
{
	if (buttons & (1 << 0))
		info->button0 = true;
	if (buttons & (1 << 1))
		info->button1 = true;

	info->x += dx;
	info->y += dy;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadCursor() - Reads the input devices and fills in the cursor info
//		struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadCursor(CursorInfo *info)
{
	word	i,
			buttons;
	int		dx,dy;

	info->x = info->y = 0;
	info->button0 = info->button1 = false;

	if (MousePresent)
	{
		buttons = INL_GetMouseButtons();
		INL_GetMouseDelta(&dx,&dy);
		INL_AdjustCursor(info,buttons,dx,dy);
	}

	for (i = 0;i < MaxJoys;i++)
	{
		if (!JoysPresent[i])
			continue;

		buttons = INL_GetJoyButtons(i);
		INL_GetJoyDelta(i,&dx,&dy/*,true*/);
		dx /= 64;
		dy /= 64;
		INL_AdjustCursor(info,buttons,dx,dy);
	}
}

//if else for gfxtesting and direction
#define DIRECTIONIFELSE	(info->dir == 4)
//#define NDIRECTIONIFELSE	(info->dir != 4)

///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(int player,ControlInfo *info)
{
			boolean		realdelta;
#if DEMO0
			byte		dbyte;
#endif
			word		buttons;
			int			dx,dy;
			Motion		mx,my;
			ControlType	type;
//			sword conpee;
//			byte dir=DirTable[2];
register	KeyboardDef	*def;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

#if DEMO0
	if (DemoMode == demo_Playback)
	{
		dbyte = DemoBuffer[DemoOffset + 1];
		my = (dbyte & 3) - 1;
		mx = ((dbyte >> 2) & 3) - 1;
		buttons = (dbyte >> 4) & 3;

		if (!(--DemoBuffer[DemoOffset]))
		{
			DemoOffset += 2;
			if (DemoOffset >= DemoSize)
				DemoMode = demo_PlayDone;
		}

		realdelta = false;
	}
	else if (DemoMode == demo_PlayDone)
		Quit ("Demo playback exceeded");
	else
	{
#endif
		switch (type = Controls[player])
		{
		case ctrl_Keyboard:
			def = &KbdDefs;

			if (Keyboard[def->upleft])
				mx = motion_Left,my = motion_Up;
			else if (Keyboard[def->upright])
				mx = motion_Right,my = motion_Up;
			else if (Keyboard[def->downleft])
				mx = motion_Left,my = motion_Down;
			else if (Keyboard[def->downright])
				mx = motion_Right,my = motion_Down;
//TODO: make this into a function that the joystick AND keyboard can use wwww
			if(DIRECTIONIFELSE)//(info->dir == 2)
			{
			if(!Keyboard[def->left] && !Keyboard[def->right]){
				if((Keyboard[def->up] && !Keyboard[def->down]))
					my = motion_Up;
				if((Keyboard[def->down] && !Keyboard[def->up]))
					my = motion_Down;
			}else if(!Keyboard[def->up] && !Keyboard[def->down]){
				if((Keyboard[def->left] && !Keyboard[def->right]))
					mx = motion_Left;
				if((Keyboard[def->right] && !Keyboard[def->left]))
					mx = motion_Right;
			}else{	//2 keys pressed
					switch (info->dir)
					{
						case 0:
						case 7:
							if((Keyboard[def->left] && !Keyboard[def->right])){ mx = motion_Left; }
							else if((Keyboard[def->right] && !Keyboard[def->left])){ mx = motion_Right; }
						break;
						case 2:
						case 5:
							if((Keyboard[def->up] && !Keyboard[def->down])){ my = motion_Up; }
							else if((Keyboard[def->down] && !Keyboard[def->up])){ my = motion_Down; }
						break;
						default:
						break;
					}
#ifdef __DEBUG_InputMgr__
					//if(dbg_testcontrolnoisy > 0) printf("dir=%d		dx=%d	dy=%d	mx=%d	my=%d", info->x, info->y, info->xaxis, info->yaxis);//{ printf("dir=%c ", dirchar(dir)); }//printf("pdir=%c	", dirchar(player->pdir)); }
#endif
				}
			}
			//input from player
			if (Keyboard[def->button0])
				buttons += 1 << 0;
			if (Keyboard[def->button1])
				buttons += 1 << 1;
			realdelta = false;
			break;
		case ctrl_Joystick1:
		case ctrl_Joystick2:
			INL_GetJoyDelta(type - ctrl_Joystick,&dx,&dy/*,false*/);
			buttons = INL_GetJoyButtons(type - ctrl_Joystick);
			realdelta = true;
			break;
		case ctrl_Mouse:
			INL_GetMouseDelta(&dx,&dy);
			buttons = INL_GetMouseButtons();
			realdelta = true;
			break;
		}
#ifdef DEMO0
	}
#endif

	if (realdelta)
	{
		mx = (dx < 0)? motion_Left : ((dx > 0)? motion_Right : motion_None);
		my = (dy < 0)? motion_Up : ((dy > 0)? motion_Down : motion_None);
	}
	else
	{
		dx = mx * 127;
		dy = my * 127;
	}

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = buttons & (1 << 0);
	info->button1 = buttons & (1 << 1);
	info->button2 = buttons & (1 << 2);
	info->button3 = buttons & (1 << 3);
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
/*	conpee=(((my + 1) * 2) + (mx + 1))-1;
	info->dir = DirTable[conpee];

	if(DirTable[conpee]!=2)	player->pdir=DirTable[conpee];
	if(player->enti.q==1 &&( dir!=2 || (mx!=motion_None || my!=motion_None)))
	{
		if(dir==2) player->enti.d = info->dir;
		else player->enti.d = DirTable[dir];
	}
*/
#if DEMO0
	if (DemoMode == demo_Record)
	{
		// Pack the control info into a byte
		dbyte = (buttons << 4) | ((mx + 1) << 2) | (my + 1);

		if
		(
			(DemoBuffer[DemoOffset + 1] == dbyte)
		&&	(DemoBuffer[DemoOffset] < 255)
		)
			(DemoBuffer[DemoOffset])++;
		else
		{
			if (DemoOffset || DemoBuffer[DemoOffset])
				DemoOffset += 2;

			if (DemoOffset >= DemoSize)
				Quit ("Demo buffer overflow");

			DemoBuffer[DemoOffset] = 1;
			DemoBuffer[DemoOffset + 1] = dbyte;
		}
	}
#endif
#ifdef __DEBUG_InputMgr__
if(dbg_joymousedelta)
{
	{
static		int			old_dx,old_dy;
static		word		old_buttons=0;
		if(dx!=old_dx || dy!=old_dy) printf("dx,dy	[%d,%d]	%d,%d\n", dx, dy, mx, my);
		if(dx!=old_dx)	old_dx=dx;
		if(dy!=old_dy)	old_dy=dy;
		if(old_buttons!=buttons)
		{
			printf("	buttons={%u,%u,%u,%u}\n", info->button0, info->button1, info->button2, info->button3);
			old_buttons=buttons;
		}
	}
}

if(dbg_testcontrolnoisy > 0)
if(info->dir!=4/*(Keyboard[def->up] || Keyboard[def->down] || Keyboard[def->left] || Keyboard[def->right])*//* || player->enti.q>1*/)
{
	//printf("b1=%u b2=%u b3=%u b4=%u	", info->button0, info->button1, info->button2, info->button3);
	//printf("q=%d ", player->enti.q);
	//printf("cpee=%c ", dirchar(conpee));
	// printf("pdir=%c d=%c dir=%c ", dirchar(player->pdir), dirchar(player->enti.d), dirchar(info->dir));
	printf("info->dir=%c	", dirchar(info->dir));
	printf("dx=%d	dy=%d	mx=%d	my=%d", info->x, info->y, info->xaxis, info->yaxis);
	//else if(!realdelta) printf("%c%d %c%d %c%d %c%d", dirchar(0), Keyboard[def->up], dirchar(4), Keyboard[def->down], dirchar(1), Keyboard[def->left], dirchar(3), Keyboard[def->right]);
	printf("\n");
}
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_SetControlType() - Sets the control type to be used by the specified
//		player
//
///////////////////////////////////////////////////////////////////////////
void
IN_SetControlType(int player,ControlType type)
{
	// DEBUG - check that requested type is present?
	Controls[player] = type;
}

#if DEMO0
///////////////////////////////////////////////////////////////////////////
//
//	IN_StartDemoRecord() - Starts the demo recording, using a buffer the
//		size passed. Returns if the buffer allocation was successful
//
///////////////////////////////////////////////////////////////////////////
boolean
IN_StartDemoRecord(word bufsize)
{
	if (!bufsize)
		return(false);

	MM_GetPtr((memptr *)&DemoBuffer,bufsize);
	DemoMode = demo_Record;
	DemoSize = bufsize & ~1;
	DemoOffset = 0;
	DemoBuffer[0] = DemoBuffer[1] = 0;

	return(true);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_StartDemoPlayback() - Plays back the demo pointed to of the given size
//
///////////////////////////////////////////////////////////////////////////
void
IN_StartDemoPlayback(byte /*__segment*/ *buffer,word bufsize)
{
	DemoBuffer = buffer;
	DemoMode = demo_Playback;
	DemoSize = bufsize & ~1;
	DemoOffset = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_StopDemo() - Turns off demo mode
//
///////////////////////////////////////////////////////////////////////////
void
IN_StopDemo(void)
{
	if ((DemoMode == demo_Record) && DemoOffset)
		DemoOffset += 2;

	DemoMode = demo_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_FreeDemoBuffer() - Frees the demo buffer, if it's been allocated
//
///////////////////////////////////////////////////////////////////////////
void
IN_FreeDemoBuffer(void)
{
	if (DemoBuffer)
		MM_FreePtr((memptr *)&DemoBuffer);
}
#endif


///////////////////////////////////////////////////////////////////////////
//
//	IN_GetScanName() - Returns a string containing the name of the
//		specified scan code
//
///////////////////////////////////////////////////////////////////////////
byte *
IN_GetScanName(ScanCode scan)
{
	byte		**p;
	ScanCode	far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
ScanCode
IN_WaitForKey(void)
{
	ScanCode	result;

	while (!(result = LastScan))
		;
	LastScan = 0;
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForASCII() - Waits for an ASCII char, then clears LastASCII and
//		returns the ASCII value
//
///////////////////////////////////////////////////////////////////////////
char
IN_WaitForASCII(void)
{
	char		result;

	while (!(result = LastASCII))
		;
	LastASCII = '\0';
	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

boolean	btnstate[8];

void IN_StartAck(void)
{
	unsigned	i,buttons;

//
// get initial state of everything
//
	IN_ClearKeysDown();
	memset (btnstate,0,sizeof(btnstate));

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if (buttons&1)
			btnstate[i] = true;
}


boolean IN_CheckAck (void)
{
	unsigned	i,buttons;

//
// see if something has been pressed
//
	if(!IN_Started)
		getch();
	else
	{
		if (LastScan)
			return true;
	}

	buttons = IN_JoyButtons () << 4;
	if (MousePresent)
		buttons |= IN_MouseButtons ();

	for (i=0;i<8;i++,buttons>>=1)
		if ( buttons&1 )
		{
			if (!btnstate[i])
				return true;
		}
		else
			btnstate[i]=false;

	return false;
}


void IN_Ack (void)
{
	IN_StartAck ();

	while (!IN_CheckAck ())
	;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_IsUserInput() - Returns true if a key has been pressed or a button
//		is down
//
///////////////////////////////////////////////////////////////////////////
boolean
IN_IsUserInput(void)
{
	boolean	result;
	word	i;

	result = LastScan;

	if (MousePresent)
		if (INL_GetMouseButtons())
			result = true;

	for (i = 0;i < MaxJoys;i++)
		if (JoysPresent[i])
			if (INL_GetJoyButtons(i))
				result = true;

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
{
	longword	lasttime;
#ifndef	__16_SD__
TimeCount = *clockw;
#endif

	lasttime = TimeCount;
	IN_StartAck ();
	do
	{
		if (IN_CheckAck())
			return true;
	} while (TimeCount - lasttime < delay);
	return(false);
}

//===========================================================================

/*
===================
=
= IN_MouseButtons
=
===================
*/

byte	IN_MouseButtons (void)
{
	if (MousePresent)
	{
		Mouse(MButtons);
		return _BX;
	}
	else
		return 0;
}


/*
===================
=
= IN_JoyButtons
=
===================
*/

word	IN_JoyButtons (void)
{
	word joybits;

	joybits = inportb(0x201);	// Get all the joystick buttons
	joybits >>= 4;				// only the high bits are useful
	joybits ^= 15;				// return with 1=pressed

	return joybits;
}

boolean IN_KeyDown(byte code)
{
#ifdef __DEBUG_InputMgr__
//	if(Keyboard[code])
//		printf("IN_KeyDown(%c):	%u\n", code, Keyboard[code]);
	if(!dbg_nointest)
#endif
	return Keyboard[code];
#ifdef __DEBUG_InputMgr__
	else
		if(dbg_nointest && kbhit())
			return 1;
		else
			return 0;
#endif
}

void IN_ClearKey(byte code)
{
	Keyboard[code] = false;
	if(code == LastScan)
		LastScan = sc_None;
	}

boolean IN_qb(byte kee)
{
#ifdef __DEBUG_InputMgr__
	if(dbg_testkeyin) if(Keyboard[kee]) printf("IN_qb():	%u\n", Keyboard[kee]);
#endif
	if(Keyboard[kee]==true) return 1;
	else return 0;
}

ScanCode IN_GetLastScan()
{
	return LastScan;
}

ScanCode IN_GetCurCode()
{
	return CurCode;
}

void IN_KbdLED()
{
	byte far *lock_key;

	// turn off num-lock via BIOS
	lock_key = MK_FP(0x040, 0x017); // Pointing to the address of the bios shift state keys
	*lock_key&=(~(16 | 32 | 64)); // toggle off the locks by changing the values of the 4th, 5th, and 6th bits of the address byte of 0040:0017
	OldKeyVect();	// call BIOS keyhandler to change keyboard lights
}
