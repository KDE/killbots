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
  : KGameRenderer( Settings::defaultThemeValue() ),
    m_aspectRatio( 0 )
{
	setTheme( Settings::theme() );
}


QCursor Killbots::Renderer::cursorFromAction( int direction )
{
	if ( m_cursors.isEmpty() )
	{
		for ( int i = 0; i <= 8; ++i )
		{
			const QPixmap pixmap = spritePixmap( QString("cursor%1").arg( i ), QSize( 42, 42 ) );
			if ( !pixmap.isNull() )
				m_cursors.insert( i, QCursor( pixmap ) );
		}
	}

	return m_cursors.value( direction, Qt::ArrowCursor );
}


QColor Killbots::Renderer::textColor()
{
	if ( !m_textColor.isValid() )
		m_textColor = spritePixmap( "textcolor", QSize( 3, 3 ) ).toImage().pixel( 1, 1 );

	return m_textColor;
}


qreal Killbots::Renderer::aspectRatio()
{
	if ( !m_aspectRatio )
	{
		const QRectF tileRect = boundsOnSprite("cell");
		m_aspectRatio = qBound<qreal>( 0.3333, tileRect.width() / tileRect.height(), 3.0 );
	}

	return m_aspectRatio;
}


void Killbots::Renderer::setTheme( const QString & theme )
{
	m_aspectRatio = 0;
	m_textColor = QColor();
	m_cursors.clear();

	KGameRenderer::setTheme( theme );
}
