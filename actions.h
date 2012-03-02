/*
 *  Copyright 2006-2009  Parker Coates <coates@kde.org>
 *
 *  This file is part of Killbots.
 *
 *  Killbots is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Killbots is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Killbots. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KILLBOTS_ACTIONS_H
#define KILLBOTS_ACTIONS_H

namespace Killbots
{
	enum HeroAction
	{
		Right = 0,
		UpRight,
		Up,
		UpLeft,
		Left,
		DownLeft,
		Down,
		DownRight,
		Hold,

		Teleport,
		TeleportSafely,
		TeleportSafelyIfPossible,
		WaitOutRound,
		Vaporizer,
		NoAction,

		RepeatRight = -( Right + 1 ),
		RepeatUpRight = -( UpRight + 1 ),
		RepeatUp = -( Up + 1 ),
		RepeatUpLeft = -( UpLeft + 1 ),
		RepeatLeft = -( Left + 1 ),
		RepeatDownLeft = -( DownLeft + 1 ),
		RepeatDown = -( Down + 1 ),
		RepeatDownRight = -( DownRight + 1 ),
		RepeatHold = -( Hold + 1 )
	};
}

#endif
