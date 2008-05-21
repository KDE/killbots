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
		Right,
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

		RepeatRight = -( Right + 1 ),
		RepeatUpRight = -( Right + 1 ),
		RepeatUp = -( Right + 1 ),
		RepeatUpLeft = -( Right + 1 ),
		RepeatLeft = -( Right + 1 ),
		RepeatDownLeft = -( Right + 1 ),
		RepeatDown = -( Right + 1 ),
		RepeatDownRight = -( Right + 1 ),
		RepeatHold = -( Right + 1 )
	};

	class Engine : public QObject
	{
		Q_OBJECT

	  public:
		explicit Engine( Scene * scene, QObject * parent = 0 );
		virtual ~Engine();
		const Ruleset * ruleset();

	  signals:
		void newGame( int rows, int columns, bool gameIncludesEnergy );
		void roundComplete();
		void gameOver( int score, int round );
		void boardFull();

		void roundChanged( int round );
		void scoreChanged( int score );
		void enemyCountChanged( int enemyCount );
		void energyChanged( int energy );
		void canAffordSafeTeleport( bool canAfford );

	  public slots:
		void newGame();
		void doAction( HeroAction action );
		void doAction( int action );
		void goToNextPhase();

	  private slots:
		void newRound();
		void cleanUpRound();

	  private:
		enum Phase { CleanUpRound, NewRound, ReadyToStart, MoveRobots, AssessDamageToRobots, MoveFastbots, AssessDamageToFastbots, BoardFull, RoundComplete, FinalPhase, GameOver };

		void moveHero( HeroAction direction );
		void pushJunkheap( Sprite * junkheap, HeroAction direction );
		void teleportHero();
		void teleportHeroSafely();
		void waitOutRound();
		void moveRobots( bool justFastbots = false );
		void assessDamage();

		void animateThenGoToNextPhase( Phase phase );

		void refreshSpriteMap();
		int spriteTypeAt( const QPoint & cell ) const;
		bool cellIsValid( const QPoint & cell ) const;
		bool moveIsValid( const QPoint & cell, HeroAction direction ) const;
		bool canPushJunkheap( Sprite * junkheap, HeroAction direction ) const;
		bool moveIsSafe( const QPoint &  cell, HeroAction direction ) const;
		QPoint vectorFromDirection( int direction ) const;
		void destroySprite( Sprite * sprite );
		bool destroyAllCollidingBots( Sprite * sprite );

		Scene * m_scene;

		Sprite * m_hero;
		QList<Sprite *> m_bots;
		QList<Sprite *> m_junkheaps;

		bool m_busy;
		int m_nextPhase;
		bool m_repeatMove;
		bool m_waitOutRound;
		HeroAction m_lastDirection;

		Ruleset * m_rules;
		int m_round;
		int m_score;
		int m_energy;
		int m_robotCount;
		int m_fastbotCount;

		QMultiHash< QPoint, Sprite * > m_spriteMap;
	};
}

#endif
