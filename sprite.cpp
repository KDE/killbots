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

#include "sprite.h"

#include "killbots.h"
#include "render.h"

#include <QtGui/QPainter>


Killbots::Sprite::Sprite()
  : QGraphicsItem()
{
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


QRectF Killbots::Sprite::boundingRect() const
{
	return QRectF( -0.5 * QPointF( m_size.width(), m_size.height() ), m_size );
}


void Killbots::Sprite::setSize( QSize size )
{
	prepareGeometryChange();
	m_size = size;
}


void Killbots::Sprite::paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED( option )
	Q_UNUSED( widget )

	QString elementName;
	if ( m_type == NoSprite )
		elementName = "tile";
	else if ( m_type == Hero )
		elementName = "hero";
	else if ( m_type == Robot )
		elementName = "enemy";
	else if ( m_type == Fastbot )
		elementName = "fastenemy";
	else if ( m_type == Junkheap )
		elementName = "junkheap";

	p->drawPixmap( boundingRect().topLeft(), Render::renderElement( elementName , m_size ) );
}

int Killbots::Sprite::type() const
{
	return Type;
}
