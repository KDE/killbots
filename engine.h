/*
 *  Killbots
 *  Copyright (C) 2006-2009  Parker Coates <parker.coates@gmail.com>
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

#ifndef KILLBOTS_ENGINE_H
#define KILLBOTS_ENGINE_H

#include "ruleset.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
class QPoint;

namespace Killbots
{
	class Scene;
	class Sprite;

	enum HeroAction
	{
		Right = 0,
		UpRight,
		Up,
		UpLeft,
		Left,
		DownLeft,
		Down,
		DownRight,
		Hold,

		Teleport,
		TeleportSafely,
		TeleportSafelyIfPossible,
		WaitOutRound,
		SonicScrewdriver,
		NoAction,

		RepeatRight = -( Right + 1 ),
		RepeatUpRight = -( UpRight + 1 ),
		RepeatUp = -( Up + 1 ),
		RepeatUpLeft = -( UpLeft + 1 ),
		RepeatLeft = -( Left + 1 ),
		RepeatDownLeft = -( DownLeft + 1 ),
		RepeatDown = -( Down + 1 ),
		RepeatDownRight = -( DownRight + 1 ),
		RepeatHold = -( Hold + 1 )
	};

	class Engine : public QObject
	{
		Q_OBJECT

	public: // functions
		explicit Engine( Scene * scene, QObject * parent = 0 );
		virtual ~Engine();
		const Ruleset * ruleset();
		bool gameHasStarted();

	public slots:
		void requestNewGame();
		void requestAction( HeroAction action );
		void requestAction( int action );

	signals:
		void newGame( int rows, int columns, bool gameIncludesEnergy );
		void gameOver( int score, int round );

		void roundChanged( int round );
		void scoreChanged( int score );
		void enemyCountChanged( int enemyCount );
		void energyChanged( int energy );

		void showNewGameMessage();
		void showRoundCompleteMessage();
		void showBoardFullMessage();
		void showGameOverMessage();

		void teleportAllowed( bool allowed );
		void teleportSafelyAllowed( bool allowed );
		void sonicScrewdriverAllowed( bool allowed );
		void waitOutRoundAllowed( bool allowed );

	private: // functions
		void newGame();
		void newRound( bool incrementRound = true );
		void doAction( HeroAction direction );

		bool moveHero( HeroAction direction );
		bool teleportHero();
		bool teleportHeroSafely();
		bool sonicScrewdriver();

		void pushJunkheap( Sprite * junkheap, HeroAction direction );
		void moveRobots( bool justFastbots = false );
		void assessDamage();
		void cleanUpRound();
		void resetBotCounts();

		void refreshSpriteMap();
		int spriteTypeAt( const QPoint & cell ) const;
		bool cellIsValid( const QPoint & cell ) const;
		bool moveIsValid( const QPoint & cell, HeroAction direction ) const;
		bool moveIsSafe( const QPoint &  cell, HeroAction direction ) const;
		bool canPushJunkheap( const Sprite * junkheap, HeroAction direction ) const;
		QPoint offsetFromDirection( int direction ) const;
		void destroySprite( Sprite * sprite, bool calculatePoints = true );
		bool destroyAllCollidingBots( const Sprite * sprite, bool calculatePoints = true );
		void updateScore( int changeInScore );
		void updateEnergy( int changeInEnergy );

	private slots:
		void animationDone();

	private: // data members
		Scene * m_scene;

		Sprite * m_hero;
		QList<Sprite *> m_bots;
		QList<Sprite *> m_junkheaps;

		bool m_busyAnimating;
		bool m_processFastbots;
		bool m_gameOver;
		bool m_newGameRequested;
		HeroAction m_repeatedAction;
		HeroAction m_queuedAction;

		Ruleset * m_rules;
		int m_round;
		int m_score;
		int m_energy;
		qreal m_maxEnergy;
		qreal m_robotCount;
		qreal m_fastbotCount;
		qreal m_junkheapCount;

		QMultiHash< QPoint, Sprite * > m_spriteMap;
	};
}

#endif
