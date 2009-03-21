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

#ifndef KILLBOTS_SCENE_H
#define KILLBOTS_SCENE_H

#include "actions.h"
#include "sprite.h"

#include <QtGui/QGraphicsScene>


namespace Killbots
{
	class NumericDisplayItem;
	
	class Scene : public QGraphicsScene
	{
		Q_OBJECT

	public: // functions
		explicit Scene( QObject * parent = 0 );
		virtual ~Scene();

		void addNumericDisplay( NumericDisplayItem * displayItem );
		void setGridSize( int rows, int columns );
		void forgetHero();

		Sprite * createSprite( SpriteType type, QPoint position );
		void animateSprites( const QList<Sprite *> & newSprites,
		                     const QList<Sprite *> & slidingSprites,
		                     const QList<Sprite *> & teleportingSprites,
		                     const QList<Sprite *> & destroyedSprites,
		                     qreal value
		                   ) const;

	public slots:
		void doLayout();

	signals:
		void clicked( int action );

	protected: // functions
		virtual void drawBackground( QPainter * painter, const QRectF & rect );
		virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent );
		virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent );

	private: // functions
		HeroAction getMouseDirection( QPointF cursorPosition ) const;
		bool popupAtPosition( QPointF position ) const;
		void updateSpritePos( Sprite * sprite, QPoint gridPosition ) const;

	private: // data members
		Sprite * m_hero;

		QList<NumericDisplayItem *> m_numericDisplays;

		QSize m_cellSize;
		int m_rows;
		int m_columns;
	};
}

#endif
