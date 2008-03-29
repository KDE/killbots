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
	return qHash( point.x() * 1000 + point.y() );
}


Killbots::Engine::Engine( Scene * scene, QObject * parent )
  : QObject( parent ),
	m_scene( scene ),
	m_hero( 0 ),
	m_busy( false ),
	m_repeatMove( false ),
	m_spriteMap()
{
}


Killbots::Engine::~Engine()
{
}


void Killbots::Engine::newGame()
{
	if ( !m_rules.load( Settings::ruleset() ) )
		m_rules.loadDefault();

	emit newGame( & m_rules );

	cleanUpRound();

	m_round = 0;
	m_score = 0;
	m_energy = m_rules.energyAtGameStart();
	m_robotCount = m_rules.robotsAtGameStart() - m_rules.robotsAddedEachRound();
	m_fastbotCount = m_rules.fastbotsAtGameStart() - m_rules.fastbotsAddedEachRound();
}


void Killbots::Engine::newRound()
{
	m_busy = true;
	m_repeatMove = false;
	m_waitOutRound = false;

	m_robots.clear();
	m_fastbots.clear();
	m_junkheaps.clear();
	m_spriteMap.clear();

	m_robotCount += m_rules.robotsAddedEachRound();
	m_fastbotCount += m_rules.fastbotsAddedEachRound();

	// If board is over half full, reset the counts.
	if ( m_robotCount + m_fastbotCount > m_rules.rows() * m_rules.columns() / 2 )
	{
		m_robotCount = m_rules.robotsAtGameStart() - m_rules.robotsAddedEachRound();
		m_fastbotCount = m_rules.fastbotsAtGameStart() - m_rules.fastbotsAddedEachRound();

		m_nextPhase = NewRound;
		emit boardFull();
	}
	else
	{
		// Place the hero in the centre of the board.
		QPoint centre = QPoint( qRound( m_rules.columns() / 2 ), qRound( m_rules.rows() / 2 ) );
		m_hero = m_scene->createSprite( Hero, centre );
		m_spriteMap.insert( centre, m_hero );

		// Create and randomly place junkheaps.
		for ( int i = 0; i < m_rules.junkheapsAtGameStart(); i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_junkheaps << m_scene->createSprite( Junkheap, point );
			m_spriteMap.insert( point, m_junkheaps.last() );
		}

		// Create and randomly place robots.
		for ( int i = 0; i < m_robotCount; i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_robots << m_scene->createSprite( Robot, point );
			m_spriteMap.insert( point, m_robots.last() );
		}

		// Create and randomly place fastbots.
		for ( int i = 0; i < m_fastbotCount; i++ )
		{
			QPoint point;
			do
				point = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
			while ( m_spriteMap.contains( point ) );

			m_fastbots << m_scene->createSprite( Fastbot, point );
			m_spriteMap.insert( point, m_fastbots.last() );
		}

		// Code used to generate theme previews
// 		m_hero = m_scene->createSprite( Hero, QPoint( 0, 1 ) );
// 		m_junkheaps << m_scene->createSprite( Junkheap, QPoint( 1, 1 ) );
// 		m_robots << m_scene->createSprite( Robot, QPoint( 2, 0 ) );
// 		m_fastbots << m_scene->createSprite( Fastbot, QPoint( 2, 1 ) );

		emit roundChanged( ++m_round );
		emit scoreChanged( m_score );
		emit enemyCountChanged( m_robots.size() + m_fastbots.size() );
		emit energyChanged( m_energy );
		emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

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
	else if ( m_nextPhase == MoveRobots )
		moveRobots();
	else if ( m_nextPhase == AssessDamageToRobots )
		assessDamage();
	else if ( m_nextPhase == MoveFastbots )
		moveRobots( true );
	else if ( m_nextPhase == AssessDamageToFastbots )
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
					Sprite * occupant = m_spriteMap.value( cellBehindJunkheap );
					if ( occupant )
					{
						destroySprite( occupant );
						m_score += m_rules.squashKillPointBonus();
						m_energy = qMin( m_energy + m_rules.squashKillEnergyBonus(), m_rules.maxEnergyAtGameStart() );
						emit energyChanged( m_energy );
						emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );
					}

					m_scene->slideSprite( m_spriteMap.value( newCell ), cellBehindJunkheap );
				}

				m_scene->slideSprite( m_hero, newCell );
			}

			m_busy = true;
			animateThenGoToNextPhase( MoveRobots );
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
		animateThenGoToNextPhase( MoveRobots );
	}
}


void Killbots::Engine::teleportHeroSafely()
{
	if ( !m_busy && m_energy >= m_rules.costOfSafeTeleport() )
	{
		refreshSpriteMap();

		m_repeatMove = false;

		// Choose a random cell...
		QPoint startPoint = QPoint( KRandom::random() % m_rules.columns(), KRandom::random() % m_rules.rows() );
		QPoint point = startPoint;

		// ...and step through all the cells on the board looking for a safe cell.
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

			// Looking for an empty and safe cell.
			if ( spriteTypeAt( point ) == NoSprite && point != m_hero->gridPos() && moveIsSafe( point ) )
			{
				emit energyChanged( m_energy -= m_rules.costOfSafeTeleport() );
				emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

				m_scene->teleportSprite( m_hero, point );
				m_busy = true;
				animateThenGoToNextPhase( MoveRobots );
				break;
			}
		}
		while ( point != startPoint );

		// If we stepped through every cell and found none that were safe, reset the robot counts.
		if ( point == startPoint )
		{
			m_robotCount = m_rules.robotsAtGameStart() - m_rules.robotsAddedEachRound();
			m_fastbotCount = m_rules.fastbotsAtGameStart() - m_rules.fastbotsAddedEachRound();

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
		animateThenGoToNextPhase( MoveRobots );
	}
}


void Killbots::Engine::moveRobots( bool justFastbots )
{
	QList<Sprite *> bots = m_fastbots;
	if ( ! justFastbots )
		bots += m_robots;

	foreach( Sprite * bot, bots )
	{
		QPoint vector( sign( m_hero->gridPos().x() - bot->gridPos().x() ),
		              sign( m_hero->gridPos().y() - bot->gridPos().y() ) );

		m_scene->slideSprite( bot, bot->gridPos() + vector );
	}

	if ( ! justFastbots )
		animateThenGoToNextPhase( AssessDamageToRobots );
	else
		animateThenGoToNextPhase( AssessDamageToFastbots );
}


void Killbots::Engine::assessDamage( bool justFastbots )
{
	bool heroIsDead = false;

	foreach ( Sprite * bot, m_robots + m_fastbots )
		if ( bot->gridPos() == m_hero->gridPos() )
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
		if ( ! justFastbots )
			for ( int i = 0; i < m_robots.size(); /*nothing*/ )
				if ( destroyAllCollidingSprites( m_robots[i] ) )
				{
					m_junkheaps << m_scene->createSprite( Junkheap, m_robots[i]->gridPos() );
					destroySprite( m_robots.takeAt(i) );
				}
				else
					i++;

		for ( int i = 0; i < m_fastbots.size(); /*nothing*/ )
			if ( destroyAllCollidingSprites( m_fastbots[i] ) )
			{
				m_junkheaps << m_scene->createSprite( Junkheap, m_fastbots[i]->gridPos() );
				destroySprite( m_fastbots.takeAt(i) );
			}
			else
				i++;

		emit scoreChanged( m_score );
		emit enemyCountChanged( m_robots.size() + m_fastbots.size() );
		emit energyChanged( m_energy );
		emit canAffordSafeTeleport( m_energy >= m_rules.costOfSafeTeleport() );

		if ( m_robots.isEmpty() && m_fastbots.isEmpty() )
			animateThenGoToNextPhase( RoundComplete );
		else if ( justFastbots || m_fastbots.isEmpty() )
			animateThenGoToNextPhase( FinalPhase );
		else
			animateThenGoToNextPhase( MoveFastbots );
	}
}


void Killbots::Engine::cleanUpRound()
{
	if ( m_hero )
	{
		destroySprite( m_hero );
		m_hero = 0;
	}

	foreach( Sprite * sprite, m_robots + m_fastbots + m_junkheaps )
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
	                   && m_rules.junkheapsArePushable()
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
			if ( spriteTypeAt( cell ) == Junkheap && m_rules.junkheapsArePushable() && direction < Hold )
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
	F = The position of a fastbot.
	* = A cell that we don't particularly care about in this diagram.

	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+
	| * |   |   | F | * |
	+---+---+---+---+---+
	| * |   | H |   | * |    If any of the neighbouring cells contain a robot or fastbot, the move is unsafe.
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
	+---+---+---+---+---+    Fastbots can attack from two cells away, making it trickier to determine whether they
	|   |N01|   |   |E01|    pose a threat. First we have to understand the attack vector of a fastbot. A fastbot
	+---+---+---+---+---+    attacking from a "corner" cell such as C01 will pass through a diagonal neighbour like
	|   |   | H |N02|E02|    like N01. Any fastbot attacking from an "edge" cell like E01, E02 or E03 will have to
	+---+---+---+---+---+    pass through a horizontal or vertical neighbour like N02. This mean that when checking
	|   |   |   |   |E03|    a diagonal neighbour we only need to check the one cell "behind" it for fastbots, but
	+---+---+---+---+---+    when checking a horizontal or vertical neighbour we need to check the three cells
	|   |   |   |   |   |    "behind" it for fastbots.
	+---+---+---+---+---+

	+---+---+---+---+---+
	|   |   |   |   | * |
	+---+---+---+---+---+
	| * |   |   | J |   |
	+---+---+---+---+---+    Back to junkheaps. If a neighbouring cell contains a junkheap, we don't need to check
	| * | J | H |   |   |    the cells behind it for fastbots because if there were any there, they'd just collide
	+---+---+---+---+---+    with the junkheap anyway.
	| * |   |   |   |   |
	+---+---+---+---+---+
	|   |   |   |   |   |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | * | * | * | F |
	+---+---+---+---+---+
	| * | * | * |   | * |
	+---+---+---+---+---+
	| * | * | H | * | * |    "Corner" fastbot threats are easy enough to detect. If a diagonal neighbour is empty
	+---+---+---+---+---+    and the cell behind it contains a fastbot, the move is unsafe.
	| * | * | * | * | * |
	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | * | * | * | * |
	+---+---+---+---+---+
	| R | * | * | * | * |
	+---+---+---+---+---+    "Edge" fastbots threats are much harder to detect because any fastbots on an edge might
	| F |   | H | * | * |    collide with robots or other fastbots on their way to the neighbouring cell. For example,
	+---+---+---+---+---+    the hero in this diagram is perfectly safe because all the fastbots will be destroyed
	|   | * |   | * | * |    before they can become dangerous.
	+---+---+---+---+---+
	| * | F |   | F | * |
	+---+---+---+---+---+

	+---+---+---+---+---+
	| * | F |   |   | * |
	+---+---+---+---+---+
	| * | * |   | * |   |
	+---+---+---+---+---+    With a bit of thought, it's easy to see that an "edge" fastbot is only a threat if there
	| * | * | H |   |   |    is exactly one fastbot and zero robots on that edge.
	+---+---+---+---+---+
	| * | * |   | * | F |    When you put all of the above facts together you (hopefully) get the following algorithm.
	+---+---+---+---+---+
	| * |   | F |   | * |
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
		if ( spriteTypeAt( neighbour ) == Robot || spriteTypeAt( neighbour ) == Fastbot )
			result = false;
		// Only bother checking beyond immediate neighbours if there are fastbots around.
		else if ( ! m_fastbots.isEmpty() )
		{
			// neighboursNeighbour is the cell behind the neighbour, with respect to the target cell.
			QPoint neighboursNeighbour = neighbour + vectorFromDirection( static_cast<HeroAction>( i ) );

			// If we're examining a diagonal neighbour (an odd direction)...
			if ( i % 2 == 1 )
			{
				// ...and neighboursNeighbour is a fastbot then the move is unsafe.
				if ( spriteTypeAt( neighboursNeighbour ) == Fastbot )
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

				// Then we just count the number of fastbots and robots in the list of cells.
				int fastbotsFound = 0;
				int robotsFound = 0;
				foreach( QPoint cell, cellsBehindNeighbour )
					if ( spriteTypeAt( cell ) == Fastbot )
						++fastbotsFound;
					else if ( spriteTypeAt( cell ) == Robot )
						++robotsFound;

				// If there is exactly one fastbots and zero robots, the move is unsafe.
				if ( fastbotsFound == 1 && robotsFound == 0 )
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
	foreach( Sprite * sprite, m_robots + m_fastbots + m_junkheaps )
		m_spriteMap.insert( sprite->gridPos(), sprite );
}


void Killbots::Engine::destroySprite( Sprite * sprite )
{
	switch( sprite->spriteType() )
	{
	  case Hero:
		m_hero = 0;
		break;

	  case Robot:
		if ( m_waitOutRound )
		{
			m_score += m_rules.waitKillPointBonus();
			m_energy = qMin( m_energy + m_rules.waitKillEnergyBonus(), m_rules.maxEnergyAtGameStart() );
		}
		m_score += m_rules.pointsPerRobotKilled();
		m_robots.removeOne( sprite );
		break;

	  case Fastbot:
		if ( m_waitOutRound )
		{
			m_score += m_rules.waitKillPointBonus();
			m_energy = qMin( m_energy + m_rules.waitKillEnergyBonus(), m_rules.maxEnergyAtGameStart() );
		}
		m_score += m_rules.pointsPerFastbotKilled();
		m_fastbots.removeOne( sprite );
		break;

	  case Junkheap:
		m_junkheaps.removeOne( sprite );
		break;

	  default:
		break;
	}

	m_scene->destroySprite( sprite );
}


bool Killbots::Engine::destroyAllCollidingSprites( Sprite * sprite )
{
	bool result = false;

	int i = 0;
	while ( i < m_robots.size() )
	{
		if ( m_robots[i] != sprite && m_robots[i]->gridPos() == sprite->gridPos() )
		{
			destroySprite( m_robots.takeAt(i) );
			result = true;
		}
		else
			i++;
	}

	i = 0;
	while ( i < m_fastbots.size() )
	{
		if ( m_fastbots[i] != sprite && m_fastbots[i]->gridPos() == sprite->gridPos() )
		{
			destroySprite( m_fastbots.takeAt(i) );
			result = true;
		}
		else
			i++;
	}

	return result;
}
