/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_COORDINATOR_H
#define KILLBOTS_COORDINATOR_H

#include "actions.h"
#include "sprite.h"

class KGamePopupItem;

#include <QObject>
#include <QTimeLine>

namespace Killbots
{
class Scene;
class Engine;
class NumericDisplayItem;

class Coordinator : public QObject
{
    Q_OBJECT

public: // functions
    explicit Coordinator(QObject *parent = nullptr);
    ~Coordinator() override;

    void setEngine(Engine *engine);
    void setScene(Scene *scene);

    void setAnimationSpeed(int speed);

    void beginNewAnimationStage();
    Sprite *createSprite(SpriteType type, QPoint position);
    void slideSprite(Sprite *sprite, QPoint position);
    void teleportSprite(Sprite *sprite, QPoint position);
    void destroySprite(Sprite *sprite);

public Q_SLOTS:
    void requestNewGame();
    void requestAction(int action);

private: // types
    struct AnimationStage;

private: // functions
    void startNewGame();
    void doAction(HeroAction action);
    void startAnimation();
    void startAnimationStage();
    void animationDone();

    void showUnqueuedMessage(const QString &message, int timeOut = 3000);
    void showQueuedMessage(const QString &message);

private Q_SLOTS:
    void nextAnimationStage();
    void animate(qreal value);

    void updateRound(int round);
    void updateScore(int score);
    void updateEnemyCount(int enemyCount);
    void updateEnergy(int energy);

    void showNewGameMessage();
    void showRoundCompleteMessage();
    void showBoardFullMessage();
    void showGameOverMessage();

private: // data members
    Engine *m_engine;
    Scene *m_scene;

    NumericDisplayItem *m_roundDisplay;
    NumericDisplayItem *m_scoreDisplay;
    NumericDisplayItem *m_enemyCountDisplay;
    NumericDisplayItem *m_energyDisplay;

    KGamePopupItem *m_unqueuedPopup;
    KGamePopupItem *m_queuedPopup;

    QTimeLine m_timeLine;

    QList<AnimationStage> m_stages;

    bool m_busyAnimating;
    bool m_newGameRequested;
    HeroAction m_repeatedAction;
    HeroAction m_queuedAction;
};
}

#endif
