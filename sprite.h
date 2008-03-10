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

#ifndef KILLBOTS_SPRITE_H
#define KILLBOTS_SPRITE_H

#include "killbots.h"

#include <QtGui/QGraphicsItem>

namespace Killbots
{
	class Sprite : public QGraphicsItem
	{
	  public:
		explicit Sprite();
		virtual ~Sprite();

		QPoint gridPos() const;
		void setGridPos( QPoint position );

		QPoint storedGridPos() const;
		void storeGridPos();

		SpriteType spriteType() const;
		void setSpriteType( SpriteType type );

		virtual QRectF boundingRect() const;
		void setSize( QSize size );

		virtual void paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
		virtual int type() const;
		enum { Type = UserType + 314 };

	  private:
		SpriteType m_type;
		QPoint m_gridPos;
		QPoint m_storedGridPos;
		QSize m_size;
	};
}

#endif
