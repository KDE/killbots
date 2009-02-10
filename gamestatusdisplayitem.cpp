/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@gmail.com>
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

#include "gamestatusdisplayitem.h"

#include "render.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>


Killbots::GameStatusDisplayItem::GameStatusDisplayItem( const QString & labelText, QGraphicsItem * parent )
  : QGraphicsItem( parent ),
    m_label( labelText ),
    m_value( 0 ),
    m_digits( 3 )
{
	setFont( QFont() );
	setSize( preferredSize() );
}


Killbots::GameStatusDisplayItem::~GameStatusDisplayItem()
{
}


void Killbots::GameStatusDisplayItem::setSize( QSize size )
{
	prepareGeometryChange();
	m_boundingRect = QRectF( QPointF(), size );
	update();
}


QSize Killbots::GameStatusDisplayItem::preferredSize()
{
	QSize labelSize = QFontMetrics( m_font ).boundingRect( m_label ).size();
	QSize digitsSize = QFontMetrics( m_boldFont ).boundingRect( QString( m_digits + 1, '8' ) ).size();

	return QSize( labelSize.width() + digitsSize.width() + 2 * m_margin,
	              qMax( labelSize.height(), digitsSize.height() ) + 2 * m_margin
	            );
}


QRectF Killbots::GameStatusDisplayItem::boundingRect() const
{
	return m_boundingRect;
}


void Killbots::GameStatusDisplayItem::paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED( option )
	Q_UNUSED( widget )

	p->save();

	p->drawPixmap( boundingRect().topLeft(), Render::renderElement( "status", boundingRect().size().toSize() ) );

	QRectF textRect = boundingRect().adjusted( m_margin, m_margin, -m_margin, -m_margin );
	p->setPen( Render::textColor() );

	p->setFont( m_font );
	p->drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, m_label );

	p->setFont( m_boldFont );
	p->drawText( textRect, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, QString::number( m_value ) );

	p->restore();
}


void Killbots::GameStatusDisplayItem::setText( const QString & text )
{
	if ( text != m_label )
	{
		m_label = text;
		update();
	}
}


QString Killbots::GameStatusDisplayItem::text() const
{
	return m_label;
}


void Killbots::GameStatusDisplayItem::setValue( int value )
{
	if ( value != m_value )
	{
		m_value = value;
		update();
	}
}


int Killbots::GameStatusDisplayItem::value() const
{
	return m_value;
}


void Killbots::GameStatusDisplayItem::setDigits( int digits )
{
	if ( digits != m_digits )
	{
		m_digits = digits;
	}
}


int Killbots::GameStatusDisplayItem::digits() const
{
	return m_digits;
}


void Killbots::GameStatusDisplayItem::setFont( const QFont & font )
{
	if ( font != m_font )
	{
		m_font = font;

		m_boldFont = font;
		m_boldFont.setBold( true );

		m_margin = int( m_boldFont.pixelSize() * 0.6 );
	}
}


QFont Killbots::GameStatusDisplayItem::font() const
{
	return m_font;
}


