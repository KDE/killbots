/*
 *  Copyright 2007-2009  Parker Coates <coates@kde.org>
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

#ifndef KILLBOTS_COORDINATOR_H
#define KILLBOTS_COORDINATOR_H

#include "actions.h"
#include "sprite.h"

class KGamePopupItem;

#include <QtCore/QObject>
#include <QtCore/QTimeLine>


namespace Killbots
{
	class Scene;
	class Engine;
	class NumericDisplayItem;

	class Coordinator : public QObject
	{
		Q_OBJECT

	public: // functions
		explicit Coordinator( QObject * parent = 0 );
		virtual ~Coordinator();

		void setEngine( Engine * engine );
		void setScene( Scene * scene );

		void setAnimationSpeed( int speed );

		void beginNewAnimationStage();
		Sprite * createSprite( SpriteType type, QPoint position );
		void slideSprite( Sprite * sprite, QPoint position );
		void teleportSprite( Sprite * sprite, QPoint position );
		void destroySprite( Sprite * sprite );

	public slots:
		void requestNewGame();
		void requestAction( int action );

	private: // types
		struct AnimationStage;

	private: // functions
		void startNewGame();
		void doAction( HeroAction action );
		void startAnimation();
		void startAnimationStage();
		void animationDone();

		void showUnqueuedMessage( const QString & message, int timeOut = 3000 );
		void showQueuedMessage( const QString & message );

	private slots:
		void nextAnimationStage();
		void animate( qreal value );

		void updateRound( int round );
		void updateScore( int score );
		void updateEnemyCount( int enemyCount );
		void updateEnergy( int energy );

		void showNewGameMessage();
		void showRoundCompleteMessage();
		void showBoardFullMessage();
		void showGameOverMessage();

	private: // data members
		Engine * m_engine;
		Scene * m_scene;

		NumericDisplayItem * m_roundDisplay;
		NumericDisplayItem * m_scoreDisplay;
		NumericDisplayItem * m_enemyCountDisplay;
		NumericDisplayItem * m_energyDisplay;

		KGamePopupItem * m_unqueuedPopup;
		KGamePopupItem * m_queuedPopup;

		QTimeLine m_timeLine;

		QList<AnimationStage> m_stages;

		bool m_busyAnimating;
		bool m_newGameRequested;
		HeroAction m_repeatedAction;
		HeroAction m_queuedAction;
	};
}

#endif
