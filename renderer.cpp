/*
 *  Copyright 2007-2010  Parker Coates <coates@kde.org>
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

#include "renderer.h"

#include "settings.h"

#include <KGameRenderer>

#include <KDE/KGlobal>

#include <QtGui/QCursor>


K_GLOBAL_STATIC( Killbots::Renderer, r )


Killbots::Renderer * Killbots::Renderer::self()
{
	return r;
}


Killbots::Renderer::Renderer()
  : KGameRenderer( Settings::defaultThemeValue() )
{
	setTheme( Settings::theme() );
}


QCursor Killbots::Renderer::cursorFromAction( int direction )
{
	QString element = QString("cursor%1").arg( direction );
	QPixmap pixmap = spritePixmap( element, QSize( 42, 42 ) );
	return QCursor( pixmap );
}


QColor Killbots::Renderer::textColor()
{
	if ( m_cachedTheme != theme() )
	{
		m_textColor = spritePixmap( "textcolor", QSize( 3, 3 ) ).toImage().pixel( 1, 1 );
		m_cachedTheme = theme();
	}
	return m_textColor;
}


qreal Killbots::Renderer::aspectRatio()
{
	const QRectF tileRect = boundsOnSprite("cell");
	return qBound<qreal>( 0.3333, tileRect.width() / tileRect.height(), 3.0 );
}
