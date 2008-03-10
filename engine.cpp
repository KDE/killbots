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

#include "engine.h"

#include "killbots.h"
#include "scene.h"
#include "settings.h"
#include "sprite.h"

#include <KDE/KDebug>
#include <KDE/KRandom>

uint qHash( const QPoint & point )
{
	return qHash( point.x() ) ^ qHash( point.y() );
}


Killbots::Engine::Engine( Scene * scene, QObject * parent )
  : QObject( parent ),
	m_scene( scene ),
	m_hero( 0 ),
	m_busy( false ),
	m_repeatMove( false )
{
	connect( m_scene, SIGNAL(clicked(int)), this, SLOT(intToHeroAction(int)) );
	connect( m_scene, SIGNAL(animationDone()), this, SLOT(goToNextPhase()), Qt::QueuedConnection );
}


Killbots::Engine::~Engine()
{
}


void Killbots::Engine::newGame()
{
	if ( !m_rules.load( Settings::ruleset() ) )
		m_rules.loadDefault();

	emit newGame( & m_rules );

	m_round = 0;
	emit scoreChanged( m_score = 0 );
	emit energyChanged( m_energy = m_rules.initialEnergy() );
	emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

	m_enemyCount = m_rules.initialEnemyCount() - m_rules.enemiesAddedEachRound();
	m_fastEnemyCount = m_rules.initialFastEnemyCount() - m_rules.fastEnemiesAddedEachRound();

	cleanUpRound();
}


void Killbots::Engine::newRound()
{
	m_busy = true;
	m_repeatMove = false;
	m_waitOutRound = false;

	m_enemies.clear();
	m_fastEnemies.clear();
	m_junkheaps.clear();
	m_spriteMap.clear();

	m_enemyCount += m_rules.enemiesAddedEachRound();
	m_fastEnemyCount += m_rules.fastEnemiesAddedEachRound();

	// If board is over half full, reset the counts.
	if ( m_enemyCount + m_fastEnemyCount > m_rules.rows() * m_rules.columns() / 2 )
	{
		m_enemyCount = m_rules.initialEnemyCount() - m_rules.enemiesAddedEachRound();
		m_fastEnemyCount = m_rules.initialFastEnemyCount() - m_rules.fastEnemiesAddedEachRound();

		m_nextPhase = NewRound;
		emit boardFull();
	}
	else
	{
		QPoint centre = QPoint( qRound( m_rules.columns() / 2 ), qRound( m_rules.rows() / 2 ) );
		m_hero = m_scene->createSprite( Hero, centre );
		m_spriteMap[ centre ] = m_hero;

		for ( int i = 0; i < m_rules.initialJunkheapCount(); i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_junkheaps << m_scene->createSprite( Junkheap, point );
			m_spriteMap[ point ] = m_junkheaps.last();
		}


		for ( int i = 0; i < m_enemyCount; i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_enemies << m_scene->createSprite( Enemy, point );
			m_spriteMap[ point ] = m_enemies.last();
		}

		for ( int i = 0; i < m_fastEnemyCount; i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_fastEnemies << m_scene->createSprite( FastEnemy, point );
			m_spriteMap[ point ] = m_fastEnemies.last();
		}

		// Code used to generate theme previews
// 		m_hero = m_scene->createSprite( Hero, QPoint( 0, 1 ) );
// 		m_junkheaps << m_scene->createSprite( Junkheap, QPoint( 1, 1 ) );
// 		m_enemies << m_scene->createSprite( Enemy, QPoint( 2, 0 ) );
// 		m_fastEnemies << m_scene->createSprite( FastEnemy, QPoint( 2, 1 ) );

		++m_round;
		emit roundChanged( m_round );
		emit enemyCountChanged( m_enemies.size() + m_fastEnemies.size() );

		animateThenGoToNextPhase( ReadyToStart );
	}
}


void Killbots::Engine::animateThenGoToNextPhase( Phase phase )
{
	m_nextPhase = phase;

	m_scene->startAnimation();
}


void Killbots::Engine::goToNextPhase()
{
	if ( m_nextPhase == CleanUpRound )
		cleanUpRound();
	else if ( m_nextPhase == NewRound )
		newRound();
	else if ( m_nextPhase == ReadyToStart )
		m_busy = false;
	else if ( m_nextPhase == MoveEnemies )
		moveEnemies();
	else if ( m_nextPhase == AssessDamageToEnemies )
		assessDamage();
	else if ( m_nextPhase == MoveFastEnemies )
		moveEnemies( true );
	else if ( m_nextPhase == AssessDamageToFastEnemies )
		assessDamage( true );
	else if ( m_nextPhase == RoundComplete )
	{
		m_nextPhase = CleanUpRound;
		emit roundComplete();
	}
	else if ( m_nextPhase == FinalPhase )
	{
		m_busy = false;

		if ( m_waitOutRound )
			waitOutRound();
		else if ( m_repeatMove )
			moveHero( m_lastDirection );
	}
}


void Killbots::Engine::intToHeroAction( int action )
{
	if ( ! m_busy )
	{
		if ( action < NumberOfRepeatDirections )
			moveHero( static_cast<HeroAction>( action ) );
		else if ( action == Teleport )
			teleportHero();
		else if ( action == TeleportSafely )
			teleportHeroSafely();
		else if ( action == TeleportSafelyIfPossible )
			if ( m_energy >= m_rules.costOfSafeTeleport() )
				teleportHeroSafely();
			else
				teleportHero();
		else if ( action == WaitOutRound )
			waitOutRound();
	}
}


void Killbots::Engine::moveHero( HeroAction direction )
{
	if ( ! m_busy && direction < NumberOfRepeatDirections )
	{
		refreshSpriteMap();

		QPoint newCell = m_hero->gridPos() + vectorFromDirection( direction );

		if ( moveIsValid( newCell, direction )
		     && ( ( Settings::allowUnsafeMoves()
		            && ! m_repeatMove )
		          || moveIsSafe( newCell, direction )
		        )
		   )
		{
			m_lastDirection = direction;
			m_repeatMove = ( direction >= NumberOfDirections );

			if ( direction != Hold )
			{
				if ( spriteTypeAt( newCell ) == Junkheap )
				{
					QPoint cellBehindJunkheap = newCell + vectorFromDirection( direction );
					Sprite * occupant = m_spriteMap[ cellBehindJunkheap ];
					if ( occupant )
					{
						destroySprite( occupant );
						m_score += m_rules.pointsPerSquashKill();
						m_energy = qMin( m_energy + m_rules.energyPerSquashKill(), m_rules.maximumEnergy() );
						emit energyChanged( m_energy );
						emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );
					}

					m_scene->slideSprite( m_spriteMap[ newCell ], cellBehindJunkheap );
				}

				m_scene->slideSprite( m_hero, newCell );
			}

			m_busy = true;
			animateThenGoToNextPhase( MoveEnemies );
		}
		else
			m_busy = false;
	}
}


void Killbots::Engine::teleportHero()
{
	if ( ! m_busy )
	{
		refreshSpriteMap();

		m_repeatMove = false;

		QPoint point;
		do
			point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
		while ( spriteTypeAt( point ) != NoSprite || point == m_hero->gridPos() );

		m_scene->teleportSprite( m_hero, point );
		m_busy = true;
		animateThenGoToNextPhase( MoveEnemies );
	}
}


void Killbots::Engine::teleportHeroSafely()
{
	if ( !m_busy && m_energy >= m_rules.costOfSafeTeleport() )
	{
		refreshSpriteMap();

		m_repeatMove = false;

		QPoint startPoint = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
		QPoint point = startPoint;

		do
		{
			if ( point.x() < m_rules.columns() - 1 )
				point.rx()++;
			else
			{
				point.rx() = 0;
				if ( point.y() < m_rules.rows() - 1 )
					point.ry()++;
				else
					point.ry() = 0;
			}

			if ( spriteTypeAt( point ) == NoSprite && point != m_hero->gridPos() && moveIsSafe( point ) )
			{
				emit energyChanged( m_energy -= m_rules.costOfSafeTeleport() );
				emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

				m_scene->teleportSprite( m_hero, point );
				m_busy = true;
				animateThenGoToNextPhase( MoveEnemies );
				break;
			}
		}
		while ( point != startPoint );

		if ( point == startPoint )
		{
			m_enemyCount = m_rules.initialEnemyCount() - m_rules.enemiesAddedEachRound();
			m_fastEnemyCount = m_rules.initialFastEnemyCount() - m_rules.fastEnemiesAddedEachRound();

			--m_round;
			m_nextPhase = CleanUpRound;
			emit boardFull();
		}
	}
}


void Killbots::Engine::waitOutRound()
{
	if ( ! m_busy )
	{
		m_busy = true;
		m_waitOutRound = true;
		animateThenGoToNextPhase( MoveEnemies );
	}
}


void Killbots::Engine::moveEnemies( bool justFastEnemies )
{
	QList<Sprite *> enemies = m_fastEnemies;
	if ( ! justFastEnemies )
		enemies += m_enemies;

	foreach( Sprite * enemy, enemies )
	{
		QPoint vector( sign( m_hero->gridPos().x() - enemy->gridPos().x() ),
		              sign( m_hero->gridPos().y() - enemy->gridPos().y() ) );

		m_scene->slideSprite( enemy, enemy->gridPos() + vector );
	}

	if ( ! justFastEnemies )
		animateThenGoToNextPhase( AssessDamageToEnemies );
	else
		animateThenGoToNextPhase( AssessDamageToFastEnemies );
}


void Killbots::Engine::assessDamage( bool justFastEnemies )
{
	bool heroIsDead = false;

	foreach ( Sprite * enemy, m_enemies + m_fastEnemies )
		if ( enemy->gridPos() == m_hero->gridPos() )
			heroIsDead = true;

	if ( heroIsDead )
	{
		m_nextPhase = GameOver;
		emit gameOver( m_rules.name(), m_score, m_round );
	}
	else
	{
		// Check junkheaps for dead robots
		foreach ( Sprite * junkheap, m_junkheaps )
			destroyAllCollidingSprites( junkheap );

		// Check for robot-on-robot violence
		if ( ! justFastEnemies )
			for ( int i = 0; i < m_enemies.size(); /*nothing*/ )
				if ( destroyAllCollidingSprites( m_enemies[i] ) )
				{
					m_junkheaps << m_scene->createSprite( Junkheap, m_enemies[i]->gridPos() );
					destroySprite( m_enemies.takeAt(i) );
				}
				else
					i++;

		for ( int i = 0; i < m_fastEnemies.size(); /*nothing*/ )
			if ( destroyAllCollidingSprites( m_fastEnemies[i] ) )
			{
				m_junkheaps << m_scene->createSprite( Junkheap, m_fastEnemies[i]->gridPos() );
				destroySprite( m_fastEnemies.takeAt(i) );
			}
			else
				i++;

		emit scoreChanged( m_score );
		emit enemyCountChanged( m_enemies.size() + m_fastEnemies.size() );
		emit energyChanged( m_energy );
		emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

		if ( m_enemies.isEmpty() && m_fastEnemies.isEmpty() )
			animateThenGoToNextPhase( RoundComplete );
		else if ( justFastEnemies || m_fastEnemies.isEmpty() )
			animateThenGoToNextPhase( FinalPhase );
		else
			animateThenGoToNextPhase( MoveFastEnemies );
	}
}


void Killbots::Engine::cleanUpRound()
{
	if ( m_hero )
	{
		destroySprite( m_hero );
		m_hero = 0;
	}

	foreach( Sprite * sprite, m_enemies + m_fastEnemies + m_junkheaps )
		destroySprite( sprite );

	animateThenGoToNextPhase( NewRound );
}


int Killbots::Engine::spriteTypeAt( const QPoint & cell ) const
{
	Sprite * occupant = m_spriteMap.value( cell );
	if ( occupant )
		return occupant->spriteType();
	else
		return NoSprite;
}


bool Killbots::Engine::isBusy() const
{
	return m_busy;
}


bool Killbots::Engine::cellIsValid( const QPoint & cell ) const
{
	return 0 <= cell.x() && cell.x() < m_rules.columns()
	    && 0 <= cell.y() && cell.y() < m_rules.rows();
}

bool Killbots::Engine::moveIsValid( const QPoint & cell, HeroAction direction ) const
{
	QPoint cellBehindCell = cell + vectorFromDirection( direction );

/*	// The short version
	return ( cellIsValid( cell )
	         && ( spriteTypeAt( cell ) == ""
	              || ( spriteTypeAt( cell ) == "junkheap"
	                   && direction < Hold
	                   && cellIsValid( cellBehindCell )
	                   && spriteTypeAt( cellBehindCell ) != "junkheap"
	                 )
	            )
	       );
*/

	// The debuggable version
	bool result = true;

	if ( cellIsValid( cell ) )
	{
		if ( spriteTypeAt( cell ) != NoSprite )
		{
			if ( spriteTypeAt( cell ) == Junkheap && direction < Hold )
			{
				if ( cellIsValid( cellBehindCell ) )
				{
					if ( spriteTypeAt( cellBehindCell ) == Junkheap )
					{
						result = false;
						kDebug() << "Move is invalid. Cell behind junkheap is occupied by another junkheap.";
					}
				}
				else
				{
					result = false;
					kDebug() << "Move is invalid. Cell behind junkheap is invalid.";
				}
			}
			else
			{
				result = false;
				kDebug() << "Move is invalid. Cell is occupied by something other than a junkheap.";
			}
		}
	}
	else
	{
		result = false;
		kDebug() << "Move is invalid. Cell is invalid.";
	}

	return result;
}


bool Killbots::Engine::moveIsSafe( const QPoint & cell, HeroAction direction ) const
{
	/*
	Warning: This algorithm might break your head. The following diagrams and descriptions try to help.

	Note: This algorithm assumes that the proposed move has already been checked for validity.

	Legend
	H = The position of the hero after the proposed move (the cell who's safeness we're trying to determine).
	J = The position of a junkheap after the proposed move, whether moved by the hero or sitting there already.
	R = The position of a robot.
	S = The position of a superbot.
	* = A cell that we don't particularly care about in this diagram.

	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+
	| * |   |   | S | * |
	+---+---+---+---+---+
	| * |   | H |   | * |    If any of the neighbouring cells contain a robot or superbot, the move is unsafe.
	+---+---+---+---+---+
	| * |   | R |   | * |
	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+

	+---+---+---+---+---+
	|   |   |   |   |   |
	+---+---+---+---+---+
	|   |   |   |   |   |
	+---+---+---+---+---+
	|   | *<==J<==H |   |    If the proposed move involved pushing a junkheap, we can ignore the cell that the junkheap
	+---+---+---+---+---+    will end up in, because if there were an enemy there, it would be crushed.
	|   |   |   |   |   |
	+---+---+---+---+---+
	|   |   |   |   |   |
	+---+---+---+---+---+

	+---+---+---+---+---+
	|C01|   |   |   |   |
	+---+---+---+---+---+    Superbots can attack from two cells away, making it trickier to determine whether they
	|   |N01|   |   |E01|    pose a threat. First we have to understand the attack vector of a superbot. A superbot
	+---+---+---+---+---+    attacking from a "corner" cell such as C01 will pass through a diagonal neighbour like
	|   |   | H |N02|E02|    like N01. Any superbot attacking from an "edge" cell like E01, E02 or E03 will have to
	+---+---+---+---+---+    pass through a horizontal or vertical neighbour like N02. This mean that when checking
	|   |   |   |   |E03|    a diagonal neighbour we only need to check the one cell "behind" it for superbots, but
	+---+---+---+---+---+    when checking a horizontal or vertical neighbour we need to check the three cells
	|   |   |   |   |   |    "behind" it for superbots.
	+---+---+---+---+---+

	+---+---+---+---+---+
	|   |   |   |   | * |
	+---+---+---+---+---+
	| * |   |   | J |   |
	+---+---+---+---+---+    Back to junkheaps. If a neighbouring cell contains a junkheap, we don't need to check
	| * | J | H |   |   |    the cells behind it for superbots because if there were any there, they'd just collide
	+---+---+---+---+---+    with the junkheap anyway.
	| * |   |   |   |   |
	+---+---+---+---+---+
	|   |   |   |   |   |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | * | * | * | S |
	+---+---+---+---+---+
	| * | * | * |   | * |
	+---+---+---+---+---+
	| * | * | H | * | * |    "Corner" superbot threats are easy enough to detect. If a diagonal neighbour is empty
	+---+---+---+---+---+    and the cell behind it contains a superbot, the move is unsafe.
	| * | * | * | * | * |
	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+
	| R | * | * | * | * |
	+---+---+---+---+---+    "Edge" superbots threats are much harder to detect because any superbots on an edge might
	| S |   | H | * | * |    collide with robots or other superbots on their way to the neighbouring cell. For example,
	+---+---+---+---+---+    the hero in this diagram is perfectly safe because all the superbots will be destroyed
	|   | * |   | * | * |    before they can become dangerous.
	+---+---+---+---+---+
	| * | S |   | S | * |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | S |   |   | * |
	+---+---+---+---+---+
	| * | * |   | * |   |
	+---+---+---+---+---+    With a bit of thought, it's easy to see that an "edge" superbot is only a threat if there
	| * | * | H |   |   |    is exactly one superbot and zero robots on that edge.
	+---+---+---+---+---+
	| * | * |   | * | S |    When you put all of the above facts together you (hopefully) get the following algorithm.
	+---+---+---+---+---+
	| * |   | S |   | * |
	+---+---+---+---+---+

	*/

	// The move is assumed safe until proven unsafe.
	bool result = true;

	// If we're pushing a junkheap, store the cell that the junkheap will end up in. Otherwise store an invalid cell.
	QPoint cellBehindJunkheap = ( spriteTypeAt( cell ) != Junkheap ) ? QPoint( -1, -1 ) : cell + vectorFromDirection( direction );

	// We check the each of the target cells neighbours.
	for ( int i = 0; i < 8 && result; i++ )
	{
		QPoint neighbour = cell + vectorFromDirection( static_cast<HeroAction>( i ) );

		// If the neighbour is invalid or the cell behind the junkheap, continue to the next neighbour.
		if ( ! cellIsValid( neighbour ) || spriteTypeAt( neighbour ) == Junkheap || neighbour == cellBehindJunkheap )
			continue;

		// If the neighbour contains an enemy, the move is unsafe.
		if ( spriteTypeAt( neighbour ) == Enemy || spriteTypeAt( neighbour ) == FastEnemy )
			result = false;
		// Only bother checking beyond immediate neighbours if there are superbots around.
		else if ( ! m_fastEnemies.isEmpty() )
		{
			// neighboursNeighbour is the cell behind the neighbour, with respect to the target cell.
			QPoint neighboursNeighbour = neighbour + vectorFromDirection( static_cast<HeroAction>( i ) );

			// If we're examining a diagonal neighbour (an odd direction)...
			if ( i % 2 == 1 )
			{
				// ...and neighboursNeighbour is a superbot then the move is unsafe.
				if ( spriteTypeAt( neighboursNeighbour ) == FastEnemy )
					result = false;
			}
			// If we're examining an vertical or horizontal neighbour, things are more complicated...
			else
			{
				// Assemble a list of the cells behind the neighbour.
				QList<QPoint> cellsBehindNeighbour;
				cellsBehindNeighbour << neighboursNeighbour;
				// Add neighboursNeighbour's anticlockwise neighbour. ( i + 2 ) % 8 is the direction a quarter turn anticlockwise from i.
				cellsBehindNeighbour << neighboursNeighbour + vectorFromDirection( static_cast<HeroAction>( ( i + 2 ) % 8 ) );
				// Add neighboursNeighbour's clockwise neighbour. ( i + 6 ) % 8 is the direction a quarter turn clockwise from i.
				cellsBehindNeighbour << neighboursNeighbour + vectorFromDirection( static_cast<HeroAction>( ( i + 6 ) % 8 ) );

				// Then we just count the number of superbots and robots in the list of cells.
				int fastEnemiesFound = 0;
				int enemiesFound = 0;
				foreach( QPoint cell, cellsBehindNeighbour )
					if ( spriteTypeAt( cell ) == FastEnemy )
						++fastEnemiesFound;
					else if ( spriteTypeAt( cell ) == Enemy )
						++enemiesFound;

				// If there is exactly one superbot and zero robots, the move is unsafe.
				if ( fastEnemiesFound == 1 && enemiesFound == 0 )
					result = false;
			}
		}
	}

	return result;
}


QPoint Killbots::Engine::vectorFromDirection( HeroAction direction ) const
{
	if ( direction >= NumberOfDirections )
		direction = static_cast<HeroAction>( direction - NumberOfDirections );

	switch( direction )
	{
	  case Right:     return QPoint(  1,  0 );
	  case UpRight:   return QPoint(  1, -1 );
	  case Up:        return QPoint(  0, -1 );
	  case UpLeft:    return QPoint( -1, -1 );
	  case Left:      return QPoint( -1,  0 );
	  case DownLeft:  return QPoint( -1,  1 );
	  case Down:      return QPoint(  0,  1 );
	  case DownRight: return QPoint(  1,  1 );
	  default:        return QPoint(  0,  0 );
	};
}


void Killbots::Engine::refreshSpriteMap()
{
	m_spriteMap.clear();
	foreach( Sprite * sprite, m_enemies + m_fastEnemies + m_junkheaps )
		m_spriteMap.insert( sprite->gridPos(), sprite );
}


void Killbots::Engine::destroySprite( Sprite * sprite )
{
	switch( sprite->spriteType() )
	{
	  case Hero:
		m_hero = 0;
		break;

	  case Enemy:
		if ( m_waitOutRound )
		{
			m_score += m_rules.pointsPerWaitKill();
			m_energy = qMin( m_energy + m_rules.energyPerWaitKill(), m_rules.maximumEnergy() );
		}
		m_score += m_rules.pointsPerRobotKill();
		m_enemies.removeAll( sprite );
		break;

	  case FastEnemy:
		if ( m_waitOutRound )
		{
			m_score += m_rules.pointsPerWaitKill();
			m_energy = qMin( m_energy + m_rules.energyPerWaitKill(), m_rules.maximumEnergy() );
		}
		m_score += m_rules.pointsPerSuperbotKill();
		m_fastEnemies.removeAll( sprite );
		break;

	  case Junkheap:
		m_junkheaps.removeAll( sprite );
		break;

	  default:
		break;
	}

	m_scene->destroySprite( sprite );
}


bool Killbots::Engine::destroyAllCollidingSprites( Sprite * sprite )
{
	bool result = false;

	for ( int i = 0; i < m_enemies.size(); /*nothing*/ )
		if ( m_enemies[i] != sprite && m_enemies[i]->gridPos() == sprite->gridPos() )
		{
			destroySprite( m_enemies.takeAt(i) );
			result = true;
		}
		else
			i++;

	for ( int i = 0; i < m_fastEnemies.size(); /*nothing*/ )
		if ( m_fastEnemies[i] != sprite && m_fastEnemies[i]->gridPos() == sprite->gridPos() )
		{
			destroySprite( m_fastEnemies.takeAt(i) );
			result = true;
		}
		else
			i++;

	return result;
}
