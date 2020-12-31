/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_ENGINE_H
#define KILLBOTS_ENGINE_H

#include "actions.h"
#include "ruleset.h"

#include <QObject>
#include <QHash>
class QPoint;

namespace Killbots
{
class Coordinator;
class Sprite;

class Engine : public QObject
{
    Q_OBJECT

public: // functions
    explicit Engine(Coordinator *scene, QObject *parent = nullptr);
    virtual ~Engine();

    void setRuleset(const Ruleset *ruleset);
    const Ruleset *ruleset() const;

    bool gameHasStarted() const;
    bool isRoundComplete() const;
    bool isHeroDead() const;
    bool isBoardFull() const;
    bool canSafeTeleport() const;
    bool canUseVaporizer() const;

    void startNewGame();
    void startNewRound(bool incrementRound = true, const QString &layout = QString());

    bool moveHero(Killbots::HeroAction direction);
    bool teleportHero();
    bool teleportHeroSafely();
    bool useVaporizer();
    bool waitOutRound();

    void moveRobots(bool justFastbots = false);
    void assessDamage();
    void resetBotCounts();
    void endGame();

Q_SIGNALS:
    void roundChanged(int round);
    void scoreChanged(int score);
    void enemyCountChanged(int enemyCount);
    void energyChanged(int energy);
    void gameOver(int score, int round);

    void showNewGameMessage();
    void showRoundCompleteMessage();
    void showBoardFullMessage();
    void showGameOverMessage();

    void teleportAllowed(bool allowed);
    void teleportSafelyAllowed(bool allowed);
    void vaporizerAllowed(bool allowed);
    void waitOutRoundAllowed(bool allowed);

private: // functions
    void refreshSpriteMap();
    int spriteTypeAt(const QPoint &cell) const;
    QPoint offsetFromDirection(int direction) const;
    QPoint randomEmptyCell() const;

    bool cellIsValid(const QPoint &cell) const;
    bool moveIsValid(const QPoint &cell, HeroAction direction) const;
    bool moveIsSafe(const QPoint   &cell, HeroAction direction) const;
    bool canPushJunkheap(const Sprite *junkheap, HeroAction direction) const;

    void pushJunkheap(Sprite *junkheap, HeroAction direction);
    void cleanUpRound();
    void destroySprite(Sprite *sprite, bool calculatePoints = true);
    bool destroyAllCollidingBots(const Sprite *sprite, bool calculatePoints = true);
    void updateScore(int changeInScore);
    void updateEnergy(int changeInEnergy);

    QString gridToString() const;

private: // data members
    Coordinator *m_coordinator;

    Sprite *m_hero;
    QList<Sprite *> m_bots;
    QList<Sprite *> m_junkheaps;

    const Ruleset *m_rules;
    int m_round;
    int m_score;
    int m_energy;
    qreal m_maxEnergy;
    qreal m_robotCount;
    qreal m_fastbotCount;
    qreal m_junkheapCount;

    bool m_heroIsDead;
    bool m_waitingOutRound;

    QMultiHash<QPoint, Sprite *> m_spriteMap;

    friend class EngineTest;
};
}

#endif
