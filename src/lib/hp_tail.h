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

#ifndef	__HP_TAIL__
#define	__HP_TAIL__

#include "src/lib/16_head.h"
//#include "src/lib/16text.h"
#include "src/lib/16_pm.h"
#include "src/lib/16_mm.h"
#include "src/lib/16_ca.h"
#include "src/lib/16_in.h"
//#include "src/lib/16_sd.h"
#include "src/lib/16_dbg.h"
#include "src/lib/16_vl.h"


void	Shutdown16(void),
	Startup16(void),
	StartupCAMMPM (void),
	ShutdownCAMMPM (void);

void DebugMemory_(boolean q);
void MU_IntroScreen(global_game_variables_t *gvar);
boolean FizzleFade (unsigned source, unsigned dest, unsigned width, unsigned height, unsigned frames, boolean abortable, global_game_variables_t *gvar);

#endif /* __HP_TAIL__ */
