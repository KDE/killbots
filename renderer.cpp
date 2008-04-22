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

#include "renderer.h"

#include "killbots.h"
#include "settings.h"

#include <KDE/KGameTheme>

#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>


Killbots::Renderer * Killbots::Renderer::self()
{
	static Renderer instance;
	return &instance;
}


Killbots::Renderer::Renderer()
  : m_svgRenderer(),
	m_pixmapCache("killbots-cache"),
	m_hasBeenLoaded( false )
{
}


Killbots::Renderer::~Renderer()
{
}


bool Killbots::Renderer::loadTheme( const QString & fileName )
{
	bool result = false;

	KGameTheme newTheme;
	if ( newTheme.load( fileName ) )
	{
		QDateTime cacheTimeStamp = QDateTime::fromTime_t( self()->m_pixmapCache.timestamp() );
		QDateTime desktopFileTimeStamp = QFileInfo( newTheme.path() ).lastModified();
		QDateTime svgFileTimeStamp = QFileInfo( newTheme.graphics() ).lastModified();

		// We check to see if the cache contains a key matching the path to the
		// new theme file.
		QPixmap nullPixmap;
		bool isNotInCache = ! self()->m_pixmapCache.find( newTheme.path(), nullPixmap );
		if ( isNotInCache )
			kDebug() << "Theme is not already in cache.";

		bool desktopFileIsNewer = desktopFileTimeStamp > cacheTimeStamp;
		if ( desktopFileIsNewer )
		{
			kDebug() << "Desktop file is newer than cache.";
			kDebug() << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate );
			kDebug() << "Desktop file timestamp is" << desktopFileTimeStamp.toString( Qt::ISODate );
		}

		bool svgFileIsNewer = svgFileTimeStamp > cacheTimeStamp;
		if ( svgFileIsNewer )
		{
			kDebug() << "SVG file is newer than cache.";
			kDebug() << "Cache timestamp is" << cacheTimeStamp.toString( Qt::ISODate );
			kDebug() << "SVG file timestamp is" << svgFileTimeStamp.toString( Qt::ISODate );
		}

		// Discard the cache if the loaded theme doesn't match the one already
		// in the cache, or if either of the theme files have been updated
		// since the cache was created.
		bool discardCache = isNotInCache || svgFileIsNewer || desktopFileIsNewer;

		// Only bother actually loading the SVG if no SVG has been loaded
		// yet or if the cache must be discarded.
		if ( ! self()->m_hasBeenLoaded || discardCache )
		{
			if ( discardCache )
			{
				kDebug() << "Discarding cache.";
				self()->m_pixmapCache.discard();
				self()->m_pixmapCache.setTimestamp( QDateTime::currentDateTime().toTime_t() );

				// We store a null pixmap in the cache with the new theme's
				// file path as the key. This allows us to keep track of the
				// theme that is stored in the disk cache between runs.
				self()->m_pixmapCache.insert( newTheme.path(), nullPixmap );
			}

			result = self()->m_hasBeenLoaded = self()->m_svgRenderer.load( newTheme.graphics() );

			// Get the theme's aspect ratio from the .desktop file, defaulting to 1.0.
			// self()->m_aspectRatio = newTheme.themeProperty( "CellAspectRatio" ).toDouble();
			QRectF tileRect = self()->m_svgRenderer.boundsOnElement( "tile" );
			self()->m_aspectRatio = tileRect.width() / tileRect.height();
			if ( self()->m_aspectRatio <= 0.3333 || self()->m_aspectRatio >= 3.0 )
				self()->m_aspectRatio = 1.0;

			// Get the theme's text color from the .desktop file, defaulting to black.
			self()->m_textColor = QColor( newTheme.themeProperty( "TextColor" ) );
			if ( !self()->m_textColor.isValid() )
				self()->m_textColor = Qt::black;


		}
	}

	return result;
}


bool Killbots::Renderer::loadDefaultTheme()
{
	return loadTheme( "themes/default.desktop" );
}


QPixmap Killbots::Renderer::renderElement( const QString & elementId, QSize size )
{
	QPixmap result;

	QString key = elementId + QString::number( size.width() ) + 'x' + QString::number( size.height() );

	if ( ! self()->m_pixmapCache.find( key, result ) )
	{
		kDebug() << "Rendering \"" << elementId << "\" at " << size << " pixels.";

		result = QPixmap( size );
		result.fill( Qt::transparent );

		QPainter p( &result );
		self()->m_svgRenderer.render( &p, elementId );
		p.end();

		self()->m_pixmapCache.insert( key, result );
	}

	return result;
}


QPixmap Killbots::Renderer::renderGrid( int columns, int rows, QSize cellSize )
{
	QPixmap result;

	QString key = "grid" + QString::number(columns) + 'x' + QString::number(rows) + 'x' + QString::number(cellSize.width()) + 'x' + QString::number(cellSize.height());

	if ( !self()->m_pixmapCache.find( key, result ) )
	{
		kDebug() << "Rendering \"grid\" at " << rows << " rows and " << columns << " columns.";

		result = QPixmap( columns * cellSize.width(), rows * cellSize.height() );
		result.fill( Qt::transparent );

		QPainter p( &result );
		p.drawTiledPixmap( result.rect(), self()->renderElement("tile", cellSize ) );
		p.end();

		self()->m_pixmapCache.insert( key, result );
	}
	return result;
}


QPixmap Killbots::Renderer::renderBackground( QSize size )
{
	QPixmap result;

	QString key = "background" + QString::number( size.width() ) + 'x' + QString::number( size.height() );

	if ( ! self()->m_pixmapCache.find( key, result ) )
	{
		kDebug() << "Rendering \"background\" at " << size << " pixels.";

		result = QPixmap( size );
		result.fill( Qt::transparent );

		QPainter p( &result );
		self()->m_svgRenderer.render( &p, "background" );
		p.end();

		self()->m_pixmapCache.insert( key, result );
	}

	return result;
}


QColor Killbots::Renderer::textColor()
{
	return self()->m_textColor;
}


qreal Killbots::Renderer::aspectRatio()
{
	return self()->m_aspectRatio;
}
