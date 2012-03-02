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

#ifndef KILLBOTS_SPRITE_H
#define KILLBOTS_SPRITE_H

#include <KGameRenderedItem>

namespace Killbots
{
	enum SpriteType
	{
		NoSprite,
		Junkheap,
		Hero,
		Robot,
		Fastbot
	};

	class Sprite : public KGameRenderedItem
	{
	public: // types
		enum
		{
			Type = UserType + 1
		};

	public: // functions
		explicit Sprite();
		virtual ~Sprite();

		SpriteType spriteType() const;
		void setSpriteType( SpriteType type );

		void enqueueGridPos( QPoint position );
		QPoint currentGridPos() const;
		QPoint nextGridPos() const;
		QPoint gridPos() const;
		void advanceGridPosQueue();

		virtual int type() const;

	protected:
		virtual void receivePixmap( const QPixmap & pixmap );

	private: // data members
		SpriteType m_type;
		QList<QPoint> m_gridPositions;
	};
}

#endif
