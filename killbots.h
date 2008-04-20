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
		Robot,
		Fastbot
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

		Teleport,
		TeleportSafely,
		TeleportSafelyIfPossible,
		WaitOutRound,

		RepeatRight = -( Right + 1 ),
		RepeatUpRight = -( Right + 1 ),
		RepeatUp = -( Right + 1 ),
		RepeatUpLeft = -( Right + 1 ),
		RepeatLeft = -( Right + 1 ),
		RepeatDownLeft = -( Right + 1 ),
		RepeatDown = -( Right + 1 ),
		RepeatDownRight = -( Right + 1 ),
		RepeatHold = -( Right + 1 )
	};

	inline int sign( int num )
	{
		return (num > 0) ? 1 : (num == 0) ? 0 : -1;
	};
}

#endif
