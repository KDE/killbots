/*
 *  Killbots
 *  Copyright (C) 2006-2009  Parker Coates <parker.coates@gmail.com>
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

#ifndef KILLBOTS_GAMESTATUSDISPLAYITEM_H
#define KILLBOTS_GAMESTATUSDISPLAYITEM_H

#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>

namespace Killbots
{

	class GameStatusDisplayItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT

	public: // functions
		explicit GameStatusDisplayItem( const QString & labelText = QString(), QGraphicsItem * parent = 0 );
		virtual ~GameStatusDisplayItem();

		QString text() const;
		int value() const;
		int digits() const;
		void setFont( const QFont & font );
		QFont font() const;

		virtual QRectF boundingRect() const;
		virtual void paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

	public slots:
		void setText( const QString & text );
		void setValue( int value );
		void setDigits( int digits );
		void setSize( QSize size );
		QSize preferredSize();

	private: // data members
		QString m_label;
		int m_value;

		int m_margin;
		int m_digits;
		QRectF m_boundingRect;

		QFont m_font;
		QFont m_boldFont;
	};

}

#endif
