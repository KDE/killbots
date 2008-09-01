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


void Killbots::GameStatusDisplayItem::adjustSize()
{
	QString fullString = m_label + QString( m_digits + 1, '8' );

	QSize size = QFontMetrics( m_boldFont ).boundingRect( fullString ).size();

	size += 2 * QSize( m_margin, m_margin );

	setSize( size );
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
	m_label = text;
	update();
}


QString Killbots::GameStatusDisplayItem::text() const
{
	return m_label;
}


void Killbots::GameStatusDisplayItem::setValue( int value )
{
	m_value = value;
	update();
}


int Killbots::GameStatusDisplayItem::value() const
{
	return m_value;
}


void Killbots::GameStatusDisplayItem::setDigits( int digits )
{
	m_digits = digits;
	adjustSize();
}


int Killbots::GameStatusDisplayItem::digits() const
{
	return m_digits;
}


void Killbots::GameStatusDisplayItem::setFont( const QFont & font )
{
	m_font = font;

	m_boldFont = font;
	m_boldFont.setBold( true );

	m_margin = int( m_boldFont.pixelSize() * 0.6 );
	adjustSize();
}


QFont Killbots::GameStatusDisplayItem::font() const
{
	return m_font;
}


