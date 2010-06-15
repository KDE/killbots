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

#include "render.h"

#include <kgametheme.h>

#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KPixmapCache>

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

namespace Killbots
{
	class RenderPrivate
	{
	public:
		RenderPrivate()
		  : m_svgRenderer(),
		    m_pixmapCache("killbots-cache"),
		    m_cursors()
		{};

		QSvgRenderer m_svgRenderer;
		KPixmapCache m_pixmapCache;
		QHash<int,QCursor> m_cursors;
		QColor m_textColor;
		qreal m_aspectRatio;
	};

	namespace Render
	{
		K_GLOBAL_STATIC( Killbots::RenderPrivate, rp )
	}
}


bool Killbots::Render::loadTheme( const QString & fileName )
{
	bool result = false;

	KGameTheme newTheme;
	if ( newTheme.load( fileName ) )
	{
		const QDateTime cacheTimeStamp = QDateTime::fromTime_t( rp->m_pixmapCache.timestamp() );
		const QDateTime desktopFileTimeStamp = QFileInfo( newTheme.path() ).lastModified();
		const QDateTime svgFileTimeStamp = QFileInfo( newTheme.graphics() ).lastModified();

		// We check to see if the cache contains a key matching the path to the
		// new theme file.
		QPixmap nullPixmap;
		const bool isNotInCache = !rp->m_pixmapCache.find( newTheme.path(), nullPixmap );
		kDebug( isNotInCache ) << "Theme is not already in cache.";

		const bool desktopFileIsNewer = desktopFileTimeStamp > cacheTimeStamp;
		kDebug( desktopFileIsNewer ) << "Desktop file is newer than cache." << endl
		                             << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate ) << endl
		                             << "Desktop file timestamp is" << desktopFileTimeStamp.toString( Qt::ISODate );

		const bool svgFileIsNewer = svgFileTimeStamp > cacheTimeStamp;
		kDebug( svgFileIsNewer ) << "SVG file is newer than cache." << endl
		                         << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate ) << endl
		                         << "SVG file timestamp is" << svgFileTimeStamp.toString( Qt::ISODate );

		// Discard the cache if the loaded theme doesn't match the one already
		// in the cache, or if either of the theme files have been updated
		// since the cache was created.
		const bool discardCache = isNotInCache || svgFileIsNewer || desktopFileIsNewer;
		if ( discardCache )
		{
			kDebug() << "Discarding cache.";
			rp->m_pixmapCache.discard();
			rp->m_pixmapCache.setTimestamp( QDateTime::currentDateTime().toTime_t() );

			// We store a null pixmap in the cache with the new theme's
			// file path as the key. This allows us to keep track of the
			// theme that is stored in the disk cache between runs.
			rp->m_pixmapCache.insert( newTheme.path(), nullPixmap );
		}

		// Only bother actually loading the SVG if no SVG has been loaded
		// yet or if the cache must be discarded.
		if ( discardCache || !rp->m_svgRenderer.isValid() )
		{
			if ( rp->m_svgRenderer.load( newTheme.graphics() ) )
			{
				// Get the theme's aspect ratio from the .desktop file.
				const QRectF tileRect = rp->m_svgRenderer.boundsOnElement("cell");
				rp->m_aspectRatio = tileRect.width() / tileRect.height();
				rp->m_aspectRatio = qBound<qreal>( 0.3333, rp->m_aspectRatio, 3.0 );

				// Get the theme's text color from the "textcolor" SVG element.
				rp->m_textColor = renderElement( "textcolor", QSize( 3, 3 ) ).toImage().pixel( 1, 1 );

				// Generate cursors.
				for ( int i = 0; i <= 8; ++i )
				{
					const QPixmap pixmap = renderElement( QString("cursor%1").arg( i ), QSize( 42, 42 ) );
					if ( !pixmap.isNull() )
						rp->m_cursors.insert( i, QCursor( pixmap ) );
				}
			}

			result = rp->m_svgRenderer.isValid();
		}
	}

	return result;
}


QPixmap Killbots::Render::renderElement( const QString & elementId, QSize size )
{
	QPixmap result;

	if ( size.isEmpty() )
	{
		kDebug() << "Cannot render" << elementId << "at zero size.";
		return result;
	}

	const QString key = elementId + QString::number( size.width() )
	                    + 'x' + QString::number( size.height() );

	if ( !rp->m_pixmapCache.find( key, result ) )
	{
		kDebug() << "Rendering \"" << elementId << "\" at " << size << " pixels.";

		result = QPixmap( size );
		result.fill( Qt::transparent );

		QPainter p( &result );
		rp->m_svgRenderer.render( &p, elementId );
		p.end();

		rp->m_pixmapCache.insert( key, result );
	}

	return result;
}


QCursor Killbots::Render::cursorFromAction( int direction )
{
	return rp->m_cursors.value( direction, Qt::ArrowCursor );
}


QColor Killbots::Render::textColor()
{
	return rp->m_textColor;
}


qreal Killbots::Render::aspectRatio()
{
	return rp->m_aspectRatio;
}


void Killbots::Render::cleanUp()
{
	rp.destroy();
}

