/*
 *  Killbots
 *  Copyright (C) 2006-2008  Parker Coates <parker.coates@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses>
 *  or write to the Free Software Foundation, Inc., 51 Franklin Street,
 *  Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KILLBOTS_H
#define KILLBOTS_H

#include <QtCore/QPointF>
#include <QtCore/QSizeF>

namespace Killbots
{
	enum SpriteType
	{
		NoSprite,
		Hero,
		Junkheap,
		Enemy,
		FastEnemy
	};


	enum HeroAction
	{
		Right,
		UpRight,
		Up,
		UpLeft,
		Left,
		DownLeft,
		Down,
		DownRight,
		Hold,
		NumberOfDirections = Hold + 1,
		RepeatRight,
		RepeatUpRight,
		RepeatUp,
		RepeatUpLeft,
		RepeatLeft,
		RepeatDownLeft,
		RepeatDown,
		RepeatDownRight,
		RepeatHold,
		NumberOfRepeatDirections = RepeatHold + 1,
		Teleport,
		TeleportSafely,
		TeleportSafelyIfPossible,
		WaitOutRound
	};


	enum ClickAction
	{
		NothingClick,
		StepClick,
		RepeatedStepClick,
		TeleportClick,
		SafeTeleportClick,
		SafeIfPossibleClick,
		WaitOutRoundClick,
		NumberOfClickActions
	};


	inline int sign( int num )
	{
		return (num > 0) ? 1 : (num == 0) ? 0 : -1;
	};
}

#endif
