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

#include "sprite.h"

#include "render.h"

#include <QtGui/QPainter>


Killbots::Sprite::Sprite()
  : QGraphicsPixmapItem()
{
	setShapeMode( QGraphicsPixmapItem::BoundingRectShape );
}


Killbots::Sprite::~Sprite()
{
}


QPoint Killbots::Sprite::gridPos() const
{
	return m_gridPos;
}


void Killbots::Sprite::setGridPos( QPoint position )
{
	m_gridPos = position;
}


QPoint Killbots::Sprite::storedGridPos() const
{
	return m_storedGridPos;
}


void Killbots::Sprite::storeGridPos()
{
	m_storedGridPos = m_gridPos;
}


Killbots::SpriteType Killbots::Sprite::spriteType() const
{
	return m_type;
}


void Killbots::Sprite::setSpriteType( Killbots::SpriteType type )
{
	m_type = type;
}


void Killbots::Sprite::setSize( QSize size )
{
	static QHash<SpriteType, QString> elementHash;
	if ( elementHash.isEmpty() )
	{
		elementHash.insert( Hero, "hero" );
		elementHash.insert( Robot, "enemy" );
		elementHash.insert( Fastbot, "fastenemy" );
		elementHash.insert( Junkheap, "junkheap" );
	}

	setOffset( -0.5 * QPointF( size.width(), size.height() ) );
	setPixmap( Render::renderElement( elementHash.value( m_type ) , size ) );
}


int Killbots::Sprite::type() const
{
	return Type;
}
