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

#ifndef KILLBOTS_SCENE_H
#define KILLBOTS_SCENE_H

#include "engine.h"
#include "sprite.h"

class KGamePopupItem;

#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsScene>


namespace Killbots
{
	class GameStatusDisplayItem;
	
	class Scene : public QGraphicsScene
	{
		Q_OBJECT

	public: // functions
		explicit Scene( QObject * parent = 0 );
		virtual ~Scene();

		void setAnimationSpeed( int speed );
		void showUnqueuedMessage( const QString & message, int timeOut = 3000 );

		void showNewGameMessage();
		void showRoundCompleteMessage();
		void showBoardFullMessage();
		void showGameOverMessage();

		void beginNewAnimationStage();
		Sprite * createSprite( SpriteType type, QPoint position );
		void slideSprite( Sprite * sprite, QPoint position );
		void teleportSprite( Sprite * sprite, QPoint position );
		void destroySprite( Sprite * sprite );
		void showQueuedMessage( const QString & message );
		void updateRound( int round );
		void updateScore( int score );
		void updateEnemyCount( int enemyCount );
		void updateEnergy( int energy );
		void startAnimation();

	public slots:
		void doLayout();
		void onNewGame( int rows, int columns, bool gameIncludesEnergy );

	signals:
		void animationStageDone();
		void animationDone();
		void clicked( int action );

	protected: // functions
		virtual void drawBackground( QPainter * painter, const QRectF & rect );
		virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent );
		virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent );

	private: // types
		struct AnimationStage;

	private: // functions
		void startAnimationStage();
		void updateSpritePos( Sprite * sprite ) const;
		HeroAction getMouseDirection( QGraphicsSceneMouseEvent * event );

	private slots:
		void nextAnimationStage();
		void animate( qreal value );

	private: // data members
		Sprite * m_hero;
		QList<AnimationStage> m_stages;

		QTimeLine m_timeLine;

		KGamePopupItem * m_immediatePopup;
		KGamePopupItem * m_queuedPopup;

		GameStatusDisplayItem * m_roundDisplay;
		GameStatusDisplayItem * m_scoreDisplay;
		GameStatusDisplayItem * m_enemyCountDisplay;
		GameStatusDisplayItem * m_energyDisplay;
		QSize m_displaySize;

		QSize m_cellSize;
		int m_rows;
		int m_columns;
	};
}

#endif
