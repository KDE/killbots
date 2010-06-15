/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@kdemail.net>
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

#include "numericdisplayitem.h"

#include "render.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>


Killbots::NumericDisplayItem::NumericDisplayItem( const QString & label, QGraphicsItem * parent )
  : QGraphicsObject( parent ),
    m_label( label ),
    m_value( 0 ),
    m_digits( 3 )
{
	setFont( QFont() );
	setSize( preferredSize() );
}


Killbots::NumericDisplayItem::~NumericDisplayItem()
{
}


int Killbots::NumericDisplayItem::value() const
{
	return m_value;
}


QString Killbots::NumericDisplayItem::label() const
{
	return m_label;
}


int Killbots::NumericDisplayItem::digits() const
{
	return m_digits;
}


QFont Killbots::NumericDisplayItem::font() const
{
	return m_font;
}


QSize Killbots::NumericDisplayItem::preferredSize()
{
	QSize labelSize = QFontMetrics( m_font ).boundingRect( m_label ).size();
	QSize digitsSize = QFontMetrics( m_boldFont ).boundingRect( QString( m_digits + 1, '8' ) ).size();

	return QSize( labelSize.width() + digitsSize.width() + 2 * m_margin,
	              qMax( labelSize.height(), digitsSize.height() ) + 2 * m_margin
	            );
}


QRectF Killbots::NumericDisplayItem::boundingRect() const
{
	return m_boundingRect;
}


void Killbots::NumericDisplayItem::paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget )
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


void Killbots::NumericDisplayItem::setValue( int value )
{
	if ( value != m_value )
	{
		m_value = value;
		update();
	}
}


void Killbots::NumericDisplayItem::setLabel( const QString & label )
{
	if ( label != m_label )
	{
		m_label = label;
		update();
	}
}


void Killbots::NumericDisplayItem::setDigits( int digits )
{
	if ( digits != m_digits )
	{
		m_digits = digits;
	}
}


void Killbots::NumericDisplayItem::setFont( const QFont & font )
{
	m_font = font;
	m_boldFont = m_font;
	m_boldFont.setBold( true );

	m_margin = int( QFontMetrics( m_boldFont ).height() * 0.6 );
}


void Killbots::NumericDisplayItem::setSize( QSize size )
{
	prepareGeometryChange();
	m_boundingRect = QRectF( QPointF(), size );
	update();
}


