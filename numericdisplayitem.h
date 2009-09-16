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

#ifndef KILLBOTS_NUMERICDISPLAYITEM_H
#define KILLBOTS_NUMERICDISPLAYITEM_H

#include <QtGui/QFont>
#include <QtGui/QGraphicsItem>

namespace Killbots
{

	class NumericDisplayItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT
#if QT_VERSION >= 0x040600			
                Q_INTERFACES(QGraphicsItem)
#endif		
	public: // functions
		explicit NumericDisplayItem( const QString & label = QString(), QGraphicsItem * parent = 0 );
		virtual ~NumericDisplayItem();

		int value() const;
		QString label() const;
		int digits() const;
		QFont font() const;
		QSize preferredSize();

		virtual QRectF boundingRect() const;
		virtual void paint( QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

	public slots:
		void setValue( int value );
		void setLabel( const QString & label );
		void setDigits( int digits );
		void setFont( const QFont & font );
		void setSize( QSize size );

	private: // data members
		QString m_label;
		int m_value;
		int m_digits;

		QRectF m_boundingRect;
		int m_margin;

		QFont m_font;
		QFont m_boldFont;
	};
}

#endif
