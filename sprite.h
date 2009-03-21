/*
 *  Copyright 2006-2009  Parker Coates <parker.coates@gmail.com>
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

#include <QtGui/QGraphicsPixmapItem>

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

	class Sprite : public QGraphicsPixmapItem
	{
	public: // types
		enum {
			Type = UserType + 314
		};

	public: // functions
		explicit Sprite();
		virtual ~Sprite();

		QPoint gridPos() const;
		void setGridPos( QPoint position );

		QPoint storedGridPos() const;
		void storeGridPos();

		SpriteType spriteType() const;
		void setSpriteType( SpriteType type );

		void setSize( QSize size );

		virtual int type() const;

	private: // data members
		SpriteType m_type;
		QPoint m_gridPos;
		QPoint m_storedGridPos;
	};
}

#endif
