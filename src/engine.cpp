/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "engine.h"

#include "coordinator.h"
#include "settings.h"
#include "sprite.h"

#include "killbots_debug.h"

#include <QRandomGenerator>

#include <array>

uint qHash(const QPoint &point)
{
    return qHash(point.x() * 1000 + point.y());
}

inline int sign(int num)
{
    return (num > 0) ? 1 : (num == 0) ? 0 : -1;
}

Killbots::Engine::Engine(Killbots::Coordinator *scene, QObject *parent)
    : QObject(parent),
      m_coordinator(scene),
      m_hero(nullptr),
      m_rules(nullptr),
      m_round(0),
      m_score(0),
      m_energy(0),
      m_maxEnergy(0.0),
      m_robotCount(0.0),
      m_fastbotCount(0.0),
      m_junkheapCount(0.0),
      m_heroIsDead(false),
      m_waitingOutRound(false),
      m_spriteMap()
{
}

Killbots::Engine::~Engine()
{
    delete m_rules;
}

void Killbots::Engine::setRuleset(const Ruleset *ruleset)
{
    if (ruleset && ruleset != m_rules) {
        delete m_rules;
        m_rules = ruleset;
    }
}

const Killbots::Ruleset *Killbots::Engine::ruleset() const
{
    return m_rules;
}

bool Killbots::Engine::gameHasStarted() const
{
    return m_hero && m_score > 0;
}

bool Killbots::Engine::isRoundComplete() const
{
    return m_bots.isEmpty();
}

bool Killbots::Engine::isHeroDead() const
{
    return m_heroIsDead;
}

bool Killbots::Engine::isBoardFull() const
{
    return m_robotCount + m_fastbotCount + m_junkheapCount
           > m_rules->rows() * m_rules->columns() / 2;
}

bool Killbots::Engine::canSafeTeleport() const
{
    return m_rules->safeTeleportEnabled()
           && m_energy >= m_rules->costOfSafeTeleport();
}

bool Killbots::Engine::canUseVaporizer() const
{
    return m_rules->vaporizerEnabled()
           && m_energy >= m_rules->costOfVaporizer();
}

void Killbots::Engine::startNewGame()
{
    Q_ASSERT(m_rules != nullptr);

    // Don't show the new game message on first start.
    if (m_round != 0) {
        Q_EMIT showNewGameMessage();
    }

    m_heroIsDead = false;

    m_round = 1;
    m_score = 0;
    m_maxEnergy = m_rules->energyEnabled() ? m_rules->maxEnergyAtGameStart() : 0;
    m_energy = m_rules->energyEnabled() ? m_rules->energyAtGameStart() : 0;
    m_robotCount = m_rules->enemiesAtGameStart();
    m_fastbotCount = m_rules->fastEnemiesAtGameStart();
    m_junkheapCount = m_rules->junkheapsAtGameStart();

    Q_EMIT teleportAllowed(true);
    Q_EMIT waitOutRoundAllowed(true);
    Q_EMIT teleportSafelyAllowed(canSafeTeleport());
    Q_EMIT vaporizerAllowed(canUseVaporizer());

    // Code used to generate theme previews
    //newRound( "  r\nhjf", false );

    startNewRound(false);
}

void Killbots::Engine::startNewRound(bool incrementRound, const QString &layout)
{
    cleanUpRound();

    m_waitingOutRound = false;

    m_coordinator->beginNewAnimationStage();

    if (incrementRound) {
        ++m_round;

        if (m_rules->energyEnabled()) {
            m_maxEnergy += m_rules->maxEnergyAddedEachRound();
            updateEnergy(m_rules->energyAddedEachRound());
        }
        m_robotCount += m_rules->enemiesAddedEachRound();
        m_fastbotCount += m_rules->fastEnemiesAddedEachRound();
        m_junkheapCount += m_rules->junkheapsAddedEachRound();
    }

    if (layout.isEmpty()) {
        // Place the hero in the centre of the board.
        const QPoint centre = QPoint(qRound((float)(m_rules->columns() / 2)), qRound((float)(m_rules->rows() / 2)));
        m_hero = m_coordinator->createSprite(Hero, centre);

        // Create and randomly place junkheaps.
        for (int i = m_junkheapCount; i > 0 ; --i) {
            const QPoint point = randomEmptyCell();
            m_junkheaps << m_coordinator->createSprite(Junkheap, point);
            m_spriteMap.insert(point, m_junkheaps.last());
        }

        // Create and randomly place robots.
        for (int i = m_robotCount; i > 0; --i) {
            const QPoint point = randomEmptyCell();
            m_bots << m_coordinator->createSprite(Robot, point);
            m_spriteMap.insert(point, m_bots.last());
        }

        // Create and randomly place fastbots.
        for (int i = m_fastbotCount; i > 0; --i) {
            const QPoint point = randomEmptyCell();
            m_bots << m_coordinator->createSprite(Fastbot, point);
            m_spriteMap.insert(point, m_bots.last());
        }
    } else {
        const QStringList rows = layout.split(QLatin1Char('\n'));
        for (int r = 0; r < rows.size(); ++r) {
            for (int c = 0; c < rows.at(r).size(); ++c) {
                const QChar ch = rows.at(r).at(c);
                const QPoint point(c, r);

                if (ch == QLatin1Char('h') && m_hero == nullptr) {
                    m_hero = m_coordinator->createSprite(Hero, point);
		} else if (ch == QLatin1Char('r')) {
                    m_bots << m_coordinator->createSprite(Robot, point);
		} else if (ch == QLatin1Char('f')) {
                    m_bots << m_coordinator->createSprite(Fastbot, point);
		} else if (ch == QLatin1Char('j')) {
                    m_junkheaps << m_coordinator->createSprite(Junkheap, point);
                }
            }
        }
    }

    Q_EMIT roundChanged(m_round);
    Q_EMIT scoreChanged(m_score);
    Q_EMIT enemyCountChanged(m_bots.size());
    Q_EMIT energyChanged(m_energy);

    refreshSpriteMap();
}

// Returns true if the move was performed, returns false otherwise.
bool Killbots::Engine::moveHero(Killbots::HeroAction direction)
{
    refreshSpriteMap();
    const QPoint newCell = m_hero->gridPos() + offsetFromDirection(direction);
    const bool preventUnsafeMoves = Settings::preventUnsafeMoves() || direction < 0;

    if (moveIsValid(newCell, direction) && (moveIsSafe(newCell, direction) || !preventUnsafeMoves)) {
        if (direction != Hold) {
            m_coordinator->beginNewAnimationStage();

            if (spriteTypeAt(newCell) == Junkheap) {
                pushJunkheap(m_spriteMap.value(newCell), direction);
            }

            m_coordinator->slideSprite(m_hero, newCell);
        }
        return true;
    } else {
        return false;
    }
}

// Always returns true as teleports always succeed.
bool Killbots::Engine::teleportHero()
{
    refreshSpriteMap();
    const QPoint point = randomEmptyCell();
    m_coordinator->beginNewAnimationStage();
    m_coordinator->teleportSprite(m_hero, point);
    return true;
}

// Returns true if a safe cell was found. If no safe cell was found than
// the board must be full.
bool Killbots::Engine::teleportHeroSafely()
{
    refreshSpriteMap();

    // Choose a random cell...
    const QPoint startPoint = QPoint(QRandomGenerator::global()->bounded(m_rules->columns()),
                                     QRandomGenerator::global()->bounded(m_rules->rows()));
    QPoint point = startPoint;

    // ...and step through all the cells on the board looking for a safe cell.
    do {
        if (point.x() < m_rules->columns() - 1) {
            point.rx()++;
        } else {
            point.rx() = 0;
            if (point.y() < m_rules->rows() - 1) {
                point.ry()++;
            } else {
                point.ry() = 0;
            }
        }

        // Looking for an empty and safe cell.
        if (spriteTypeAt(point) == NoSprite && point != m_hero->gridPos() && moveIsSafe(point, Teleport)) {
            break;
        }
    } while (point != startPoint);

    // If we stepped through every cell and found none that were safe, reset the robot counts.
    if (point == startPoint) {
        return false;
    } else {
        m_coordinator->beginNewAnimationStage();
        updateEnergy(-m_rules->costOfSafeTeleport());
        m_coordinator->teleportSprite(m_hero, point);

        return true;
    }
}

// Returns true if any enemies were within range.
bool Killbots::Engine::useVaporizer()
{
    refreshSpriteMap();
    QList<Sprite *> neighbors;
    for (int i = Right; i <= DownRight; ++i) {
        const QPoint neighbor = m_hero->gridPos() + offsetFromDirection(i);
        if (cellIsValid(neighbor) && (spriteTypeAt(neighbor) == Robot || spriteTypeAt(neighbor) == Fastbot)) {
            neighbors << m_spriteMap.value(neighbor);
        }
    }

    if (!neighbors.isEmpty()) {
        m_coordinator->beginNewAnimationStage();
        for (Sprite *sprite : qAsConst(neighbors)) {
            destroySprite(sprite);
        }
        updateEnergy(-m_rules->costOfVaporizer());
        return true;
    } else {
        return false;
    }
}

bool Killbots::Engine::waitOutRound()
{
    m_waitingOutRound = true;
    return true;
}

void Killbots::Engine::moveRobots(bool justFastbots)
{
    m_coordinator->beginNewAnimationStage();

    if (justFastbots) {
        refreshSpriteMap();
        for (Sprite *bot : qAsConst(m_bots)) {
            if (bot->spriteType() == Fastbot) {
                const QPoint offset(sign(m_hero->gridPos().x() - bot->gridPos().x()), sign(m_hero->gridPos().y() - bot->gridPos().y()));
                const QPoint target = bot->gridPos() + offset;
                if (spriteTypeAt(target) != Robot || !m_rules->fastEnemiesArePatient()) {
                    m_coordinator->slideSprite(bot, target);
                }
            }
        }
    } else {
        for (Sprite *bot : qAsConst(m_bots)) {
            const QPoint offset(sign(m_hero->gridPos().x() - bot->gridPos().x()), sign(m_hero->gridPos().y() - bot->gridPos().y()));
            m_coordinator->slideSprite(bot, bot->gridPos() + offset);
        }
    }
}

void Killbots::Engine::assessDamage()
{
    refreshSpriteMap();

    m_coordinator->beginNewAnimationStage();

    if (m_spriteMap.count(m_hero->gridPos()) > 0) {
        m_heroIsDead = true;
    }

    // Check junkheaps for dead robots
    const auto junkheaps = m_junkheaps;
    for (Sprite *junkheap : junkheaps) {
        destroyAllCollidingBots(junkheap, !m_heroIsDead);
    }

    // Check for robot-on-robot violence
    int i = 0;
    while (i < m_bots.size()) {
        Sprite *bot = m_bots[i];
        if (bot->gridPos() != m_hero->gridPos() && destroyAllCollidingBots(bot, !m_heroIsDead)) {
            m_junkheaps << m_coordinator->createSprite(Junkheap, bot->gridPos());
            destroySprite(bot, !m_heroIsDead);
        } else {
            i++;
        }
    }

    if (isRoundComplete()) {
        m_coordinator->beginNewAnimationStage();
        Q_EMIT showRoundCompleteMessage();
    }
}

void Killbots::Engine::resetBotCounts()
{
    m_coordinator->beginNewAnimationStage();
    Q_EMIT showBoardFullMessage();

    m_maxEnergy = m_rules->maxEnergyAtGameStart();
    m_robotCount = m_rules->enemiesAtGameStart();
    m_fastbotCount = m_rules->fastEnemiesAtGameStart();
    m_junkheapCount = m_rules->junkheapsAtGameStart();

    m_coordinator->beginNewAnimationStage();
    startNewRound(false);
}

void Killbots::Engine::endGame()
{
    Q_EMIT showGameOverMessage();
    Q_EMIT teleportAllowed(false);
    Q_EMIT waitOutRoundAllowed(false);
    Q_EMIT teleportSafelyAllowed(false);
    Q_EMIT vaporizerAllowed(false);
    Q_EMIT gameOver(m_score, m_round);
}

// The hero action functions and the assessDamage functions must know the
// contents of each cell. This function updates the hash that maps cells to
// their contents.
void Killbots::Engine::refreshSpriteMap()
{
    m_spriteMap.clear();
    for (Sprite *bot : qAsConst(m_bots)) {
        m_spriteMap.insert(bot->gridPos(), bot);
    }
    for (Sprite *junkheap : qAsConst(m_junkheaps)) {
        m_spriteMap.insert(junkheap->gridPos(), junkheap);
    }
}

// A convenience function to query the type of a sprite any the given cell.
int Killbots::Engine::spriteTypeAt(const QPoint &cell) const
{
    if (m_spriteMap.contains(cell)) {
        return m_spriteMap.value(cell)->spriteType();
    } else {
        return NoSprite;
    }
}

QPoint Killbots::Engine::offsetFromDirection(int direction) const
{
    if (direction < 0) {
        direction = -direction - 1;
    }

    switch (direction) {
    case Right:
        return QPoint(1,  0);
    case UpRight:
        return QPoint(1, -1);
    case Up:
        return QPoint(0, -1);
    case UpLeft:
        return QPoint(-1, -1);
    case Left:
        return QPoint(-1,  0);
    case DownLeft:
        return QPoint(-1,  1);
    case Down:
        return QPoint(0,  1);
    case DownRight:
        return QPoint(1,  1);
    default:
        return QPoint(0,  0);
    };
}

// Returns a random empty cell on the grid. Depends on a fresh spritemap.
QPoint Killbots::Engine::randomEmptyCell() const
{
    QPoint point;
    do {
        point = QPoint(QRandomGenerator::global()->bounded(m_rules->columns()),
                       QRandomGenerator::global()->bounded(m_rules->rows()));
    } while (spriteTypeAt(point) != NoSprite || point == m_hero->gridPos());
    return point;
}

// Returns true if the given cell lies inside the game grid.
bool Killbots::Engine::cellIsValid(const QPoint &cell) const
{
    return (0 <= cell.x()
            && cell.x() < m_rules->columns()
            && 0 <= cell.y()
            && cell.y() < m_rules->rows()
           );
}

bool Killbots::Engine::moveIsValid(const QPoint &cell, HeroAction direction) const
{
    // The short version
    return (cellIsValid(cell)
            && (spriteTypeAt(cell) == NoSprite
                || (spriteTypeAt(cell) == Junkheap
                    && canPushJunkheap(m_spriteMap.value(cell), direction)
                   )
               )
           );

    /*  // The debuggable version
        bool result = true;

        if ( cellIsValid( cell ) )
        {
            if ( spriteTypeAt( cell ) != NoSprite )
            {
                if ( spriteTypeAt( cell ) == Junkheap )
                {
                    if ( !canPushJunkheap( m_spriteMap.value( cell ), direction ) )
                    {
                        result = false;
                        //qCDebug(KILLBOTS_LOG) << "Move is invalid. Cannot push junkheap.";
                    }
                }
                else
                {
                    result = false;
                    //qCDebug(KILLBOTS_LOG) << "Move is invalid. Cell is occupied by an unpushable object.";
                }
            }
        }
        else
        {
            result = false;
            //qCDebug(KILLBOTS_LOG) << "Move is invalid. Cell is lies outside grid.";
        }

        return result;
    */
}

bool Killbots::Engine::moveIsSafe(const QPoint &cell, HeroAction direction) const
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
    const QPoint cellBehindJunkheap = (spriteTypeAt(cell) != Junkheap)
                                      ? QPoint(-1, -1)
                                      : cell + offsetFromDirection(direction);

    // We check the each of the target cells neighbours.
    for (int i = Right; i <= DownRight && result; ++i) {
        const QPoint neighbor = cell + offsetFromDirection(i);

        // If the neighbour is invalid or the cell behind the junkheap, continue to the next neighbour.
        if (!cellIsValid(neighbor) || spriteTypeAt(neighbor) == Junkheap || neighbor == cellBehindJunkheap) {
            continue;
        }

        // If the neighbour contains an enemy, the move is unsafe.
        if (spriteTypeAt(neighbor) == Robot || spriteTypeAt(neighbor) == Fastbot) {
            result = false;
        } else {
            // neighboursNeighbour is the cell behind the neighbour, with respect to the target cell.
            const QPoint neighborsNeighbor = neighbor + offsetFromDirection(i);

            // If we're examining a diagonal neighbour (an odd direction)...
            if (i % 2 == 1) {
                // ...and neighboursNeighbour is a fastbot then the move is unsafe.
                if (spriteTypeAt(neighborsNeighbor) == Fastbot) {
                    result = false;
                }
            }
            // If we're examining an vertical or horizontal neighbour, things are more complicated...
            else {
                // Assemble a list of the cells behind the neighbour.
                const std::array<QPoint, 3> cellsBehindNeighbor {
		    neighborsNeighbor,
                // Add neighboursNeighbour's anticlockwise neighbour.
                // ( i + 2 ) % 8 is the direction a quarter turn anticlockwise from i.
		    neighborsNeighbor + offsetFromDirection((i + 2) % 8),
                // Add neighboursNeighbour's clockwise neighbour.
                // ( i + 6 ) % 8 is the direction a quarter turn clockwise from i.
		    neighborsNeighbor + offsetFromDirection((i + 6) % 8),
		};

                // Then we just count the number of fastbots and robots in the list of cells.
                int fastbotsFound = 0;
                int robotsFound = 0;
                for (const QPoint &cell : cellsBehindNeighbor) {
                    if (spriteTypeAt(cell) == Fastbot) {
                        ++fastbotsFound;
                    } else if (spriteTypeAt(cell) == Robot) {
                        ++robotsFound;
                    }
                }

                // If there is exactly one fastbots and zero robots, the move is unsafe.
                if (fastbotsFound == 1 && robotsFound == 0) {
                    result = false;
                }
            }
        }
    }

    return result;
}

bool Killbots::Engine::canPushJunkheap(const Sprite *junkheap, HeroAction direction) const
{
    Q_ASSERT(junkheap->spriteType() == Junkheap);

    const QPoint nextCell = junkheap->gridPos() + offsetFromDirection(direction);

    if (m_rules->pushableJunkheaps() != Ruleset::None && cellIsValid(nextCell)) {
        if (spriteTypeAt(nextCell) == NoSprite) {
            return true;
        } else if (spriteTypeAt(nextCell) == Junkheap) {
            return m_rules->pushableJunkheaps() == Ruleset::Many && canPushJunkheap(m_spriteMap.value(nextCell), direction);
        } else {
            return m_rules->squaskKillsEnabled();
        }
    } else {
        return false;
    }
}

void Killbots::Engine::pushJunkheap(Sprite *junkheap, HeroAction direction)
{
    const QPoint nextCell = junkheap->gridPos() + offsetFromDirection(direction);
    Sprite *currentOccupant = m_spriteMap.value(nextCell);
    if (currentOccupant) {
        if (currentOccupant->spriteType() == Junkheap) {
            pushJunkheap(currentOccupant, direction);
        } else {
            destroySprite(currentOccupant);
            updateScore(m_rules->squashKillPointBonus());
            updateEnergy(m_rules->squashKillEnergyBonus());
        }
    }

    m_coordinator->slideSprite(junkheap, nextCell);
}

void Killbots::Engine::cleanUpRound()
{
    m_coordinator->beginNewAnimationStage();

    if (m_hero) {
        destroySprite(m_hero);
    }
    m_hero = nullptr;

    const auto bots = m_bots;
    for (Sprite *bot : bots) {
        destroySprite(bot, false);
    }
    Q_ASSERT(m_bots.isEmpty());
    m_bots.clear();

    const auto junkheaps = m_junkheaps;
    for (Sprite *junkheap : junkheaps) {
        destroySprite(junkheap);
    }
    Q_ASSERT(m_junkheaps.isEmpty());
    m_junkheaps.clear();

    m_spriteMap.clear();
}

void Killbots::Engine::destroySprite(Sprite *sprite, bool calculatePoints)
{
    const SpriteType type = sprite->spriteType();

    if (type == Robot || type == Fastbot) {
        if (calculatePoints) {
            if (type == Robot) {
                updateScore(m_rules->pointsPerEnemyKilled());
            } else {
                updateScore(m_rules->pointsPerFastEnemyKilled());
            }

            if (m_waitingOutRound) {
                updateScore(m_rules->waitKillPointBonus());
                updateEnergy(m_rules->waitKillEnergyBonus());
            }
        }
        m_bots.removeOne(sprite);
        Q_EMIT enemyCountChanged(m_bots.size());
    } else if (type == Junkheap) {
        m_junkheaps.removeOne(sprite);
    }

    m_coordinator->destroySprite(sprite);
}

bool Killbots::Engine::destroyAllCollidingBots(const Sprite *sprite, bool calculatePoints)
{
    bool result = false;

    const auto robotsAtPos = m_spriteMap.values(sprite->gridPos());
    for (Sprite *robot : robotsAtPos) {
        if (robot != sprite && (robot->spriteType() == Robot || robot->spriteType() == Fastbot)) {
            destroySprite(robot, calculatePoints);
            result = true;
        }
    }

    return result;
}

void Killbots::Engine::updateScore(int changeInScore)
{
    if (changeInScore != 0) {
        m_score = m_score + changeInScore;
        Q_EMIT scoreChanged(m_score);
    }
}

void Killbots::Engine::updateEnergy(int changeInEnergy)
{
    if (m_rules->energyEnabled() && changeInEnergy != 0) {
        if (changeInEnergy > 0 && m_energy > int(m_maxEnergy)) {
            m_score += changeInEnergy * m_rules->pointsPerEnergyAboveMax();
        } else if (changeInEnergy > 0 && m_energy + changeInEnergy > int(m_maxEnergy)) {
            m_score += (m_energy + changeInEnergy - int(m_maxEnergy)) * m_rules->pointsPerEnergyAboveMax();
            m_energy = int(m_maxEnergy);
        } else {
            m_energy = m_energy + changeInEnergy;
        }

        Q_EMIT energyChanged(m_energy);
        Q_EMIT teleportSafelyAllowed(canSafeTeleport());
        Q_EMIT vaporizerAllowed(canUseVaporizer());
    }
}

QString Killbots::Engine::gridToString() const
{
    QString string;
    for (int r = 0; r < m_rules->rows(); ++r) {
        for (int c = 0; c < m_rules->columns(); ++c) {
            switch (spriteTypeAt(QPoint(c, r))) {
            case Robot:
                string += QLatin1Char('r');
                break;
            case Fastbot:
                string += QLatin1Char('f');
                break;
            case Junkheap:
		string += QLatin1Char('j');
                break;
            default:
		string += QLatin1Char(' ');
                break;
            }
        }
                string += QLatin1Char('\n');
    }
    return string;
}
