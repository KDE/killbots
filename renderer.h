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

#ifndef KILLBOTS_RENDERER_H
#define KILLBOTS_RENDERER_H

#include <KGameRenderer>

#include <QtCore/QHash>


namespace Killbots
{
	class Renderer : public KGameRenderer
	{
	public:
		static Renderer * self();

		Renderer();

		QCursor cursorFromAction( int direction );
		QColor textColor();
		qreal aspectRatio();

	public slots:
		// NOTE: This function is not actually polymorphic as KGR::setTheme()
		// isn't a virtual. I'm not sure if there is a better way to do this,
		// but at least it works for the time being.
		void setTheme( const QString & theme );

	private:
		QHash<int,QCursor> m_cursors;
		QColor m_textColor;
		qreal m_aspectRatio;
	};
}

#endif
