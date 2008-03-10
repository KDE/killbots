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

#ifndef KILLBOTS_RENDERER_H
#define KILLBOTS_RENDERER_H

#include <KDE/KPixmapCache>
#include <KDE/KSvgRenderer>

#include  <QtGui/QColor>
class QPixmap;

namespace Killbots
{
	class Renderer
	{
	  public:
		static bool loadTheme( const QString & filename );
		static bool loadDefaultTheme();
		static QPixmap renderElement( const QString & elementId, QSize size );
		static QPixmap renderGrid( int rows, int columns, QSize cellSize );
		static QPixmap renderBackground( QSize size );
		static QColor textColor();
		static qreal aspectRatio();

	  private:
		static Renderer * self();
		Renderer();
		Renderer( const Renderer & );
		Renderer & operator=( const Renderer & );
		~Renderer();

		KSvgRenderer m_svgRenderer;
		KPixmapCache m_pixmapCache;
		QColor m_textColor;
		qreal m_aspectRatio;
		bool m_hasBeenLoaded;
	};

}

#endif
