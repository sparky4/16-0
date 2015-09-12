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
/*
 * soundtest
 */

#include "src/lib/vgmsnd/vgmSnd.h"
#include "src/lib/16_snd.h"
#include "src/lib/16_in.h"

void OPL2_Write(UINT8 reg, UINT8 data);
UINT8 OPL2_ReadStatus(void);

void OPL2_Write(UINT8 reg, UINT8 data)
{
	//ym3812_w(0, 0, reg);
	//ym3812_w(0, 1, data);
	opl2out(reg, data);
	return;
}

UINT8 OPL2_ReadStatus(void)
{
	return 0;
	//return ym3812_r(0, 0);
}

void
main(int argc, char *argv[])
{
	global_game_variables_t gvar;
	VGM_FILE pee;
	player_t player[MaxPlayers];

	InitEngine();
	OpenVGMFile("data\0.vgm", &pee);
	IN_Startup();
	IN_Default(0,&player,ctrl_Joystick);
	PlayMusic(&pee);
	while(!IN_KeyDown(sc_Escape))
	{
		IN_ReadControl(0,&player);
		UpdateSoundEngine();
	}
	StopMusic();
	/*VGM_FILE* tempVgmFile;
	UINT8 vgmChn;
	UINT8 vgmId;

	tempVgmFile = &vgmFiles[vgmId];

	if (vgmChn == 0x7F)
		PlayMusic(tempVgmFile);
	else
		PlaySFX(tempVgmFile, vgmChn);*/
	FreeVGMFile(&pee);
	DeinitEngine();
	IN_Shutdown();
}
