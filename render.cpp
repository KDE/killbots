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

#include "render.h"

#include <kgamesvgdocument.h>
#include <kgametheme.h>

#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KPixmapCache>
#include <KDE/KSvgRenderer>

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QPainter>


namespace Killbots
{
	class RenderPrivate
	{
	public:
		RenderPrivate()
		  : m_svgRenderer(),
		    m_pixmapCache("killbots-cache"),
		    m_cursors(),
		    m_hasBeenLoaded( false )
		{};

		KSvgRenderer m_svgRenderer;
		KPixmapCache m_pixmapCache;
		QHash<int,QCursor> m_cursors;
		QColor m_textColor;
		qreal m_aspectRatio;
		bool m_hasBeenLoaded;
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
		if ( isNotInCache )
			kDebug() << "Theme is not already in cache.";

		const bool desktopFileIsNewer = desktopFileTimeStamp > cacheTimeStamp;
		if ( desktopFileIsNewer )
		{
			kDebug() << "Desktop file is newer than cache.";
			kDebug() << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate );
			kDebug() << "Desktop file timestamp is" << desktopFileTimeStamp.toString( Qt::ISODate );
		}

		const bool svgFileIsNewer = svgFileTimeStamp > cacheTimeStamp;
		if ( svgFileIsNewer )
		{
			kDebug() << "SVG file is newer than cache.";
			kDebug() << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate );
			kDebug() << "SVG file timestamp is" << svgFileTimeStamp.toString( Qt::ISODate );
		}

		// Discard the cache if the loaded theme doesn't match the one already
		// in the cache, or if either of the theme files have been updated
		// since the cache was created.
		const bool discardCache = isNotInCache || svgFileIsNewer || desktopFileIsNewer;

		// Only bother actually loading the SVG if no SVG has been loaded
		// yet or if the cache must be discarded.
		if ( !rp->m_hasBeenLoaded || discardCache )
		{
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

			result = rp->m_hasBeenLoaded = rp->m_svgRenderer.load( newTheme.graphics() );

			// Get the theme's aspect ratio from the .desktop file.
			const QRectF tileRect = rp->m_svgRenderer.boundsOnElement("cell");
			rp->m_aspectRatio = tileRect.width() / tileRect.height();
			rp->m_aspectRatio = qBound( 0.3333, rp->m_aspectRatio, 3.0 );

			// Get the theme's text color. Use the fill color of the "text" SVG element.
			// If it exists doesn't exist or the fill isn't a valid color, default to black.
			KGameSvgDocument svg;
			svg.load( newTheme.graphics() );
			if ( !svg.elementById("text").isNull() )
				rp->m_textColor = QColor( svg.styleProperty("fill") );
			if ( !rp->m_textColor.isValid() )
				rp->m_textColor = Qt::black;

			// Generate cursors.
			for ( int i = 0; i <= 8; ++i )
			{
				const QPixmap pixmap = renderElement( QString("cursor%1").arg( i ), QSize( 42, 42 ) );
				if ( !pixmap.isNull() )
					rp->m_cursors.insert( i, QCursor( pixmap ) );
			}
		}
	}

	return result;
}


bool Killbots::Render::loadDefaultTheme()
{
	return loadTheme("themes/default.desktop");
}


QPixmap Killbots::Render::renderElement( const QString & elementId, QSize size )
{
	QPixmap result;

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
