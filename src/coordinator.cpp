/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "coordinator.h"

#include "engine.h"
#include "numericdisplayitem.h"
#include "ruleset.h"
#include "scene.h"
#include "settings.h"

#include <KGamePopupItem>

#include <KLocalizedString>

#include <QGraphicsView>

#include <cmath>

struct Killbots::Coordinator::AnimationStage {
    AnimationStage()
        : newRound(-1),
          newScore(-1),
          newEnemyCount(-1),
          newEnergy(-1)
    {}

    bool isEmpty() const
    {
        return spritesToCreate.isEmpty()
               && spritesToSlide.isEmpty()
               && spritesToTeleport.isEmpty()
               && spritesToDestroy.isEmpty()
               && message.isEmpty()
               && newRound == -1
               && newScore == -1
               && newEnemyCount == -1
               && newEnergy == -1;
    }

    QList<Sprite *> spritesToCreate;
    QList<Sprite *> spritesToSlide;
    QList<Sprite *> spritesToTeleport;
    QList<Sprite *> spritesToDestroy;
    QString message;
    int oldRound, newRound;
    int oldScore, newScore;
    int oldEnemyCount, newEnemyCount;
    int oldEnergy, newEnergy;
};

Killbots::Coordinator::Coordinator(QObject *parent)
    : QObject(parent),
      m_engine(nullptr),
      m_scene(nullptr),
      m_roundDisplay(nullptr),
      m_scoreDisplay(nullptr),
      m_enemyCountDisplay(nullptr),
      m_energyDisplay(nullptr),
      m_unqueuedPopup(nullptr),
      m_queuedPopup(nullptr),
      m_timeLine(1000, this),
      m_busyAnimating(false),
      m_newGameRequested(false),
      m_queuedAction(NoAction)
{
    m_timeLine.setEasingCurve(QEasingCurve::InOutSine);
    connect(&m_timeLine, &QTimeLine::valueChanged, this, &Coordinator::animate);
    connect(&m_timeLine, &QTimeLine::finished, this, &Coordinator::nextAnimationStage);
}

Killbots::Coordinator::~Coordinator()
{
}

void Killbots::Coordinator::setEngine(Engine *engine)
{
    m_engine = engine;

    connect(m_engine, &Engine::roundChanged, this, &Coordinator::updateRound);
    connect(m_engine, &Engine::scoreChanged, this, &Coordinator::updateScore);
    connect(m_engine, &Engine::enemyCountChanged, this, &Coordinator::updateEnemyCount);
    connect(m_engine, &Engine::energyChanged, this, &Coordinator::updateEnergy);

    connect(m_engine, &Engine::showNewGameMessage, this, &Coordinator::showNewGameMessage);
    connect(m_engine, &Engine::showRoundCompleteMessage, this, &Coordinator::showRoundCompleteMessage);
    connect(m_engine, &Engine::showBoardFullMessage, this, &Coordinator::showBoardFullMessage);
    connect(m_engine, &Engine::showGameOverMessage, this, &Coordinator::showGameOverMessage);
}

void Killbots::Coordinator::setScene(Scene *scene)
{
    m_scene = scene;

    m_roundDisplay = new NumericDisplayItem;
    m_roundDisplay->setLabel(i18nc("@label", "Round:"));
    m_roundDisplay->setDigits(2);
    m_scene->addNumericDisplay(m_roundDisplay);

    m_scoreDisplay = new NumericDisplayItem;
    m_scoreDisplay->setLabel(i18nc("@label", "Score:"));
    m_scoreDisplay->setDigits(5);
    m_scene->addNumericDisplay(m_scoreDisplay);

    m_enemyCountDisplay = new NumericDisplayItem;
    m_enemyCountDisplay->setLabel(i18nc("@label", "Enemies:"));
    m_enemyCountDisplay->setDigits(3);
    m_scene->addNumericDisplay(m_enemyCountDisplay);

    m_energyDisplay = new NumericDisplayItem;
    m_energyDisplay->setLabel(i18nc("@label", "Energy:"));
    m_energyDisplay->setDigits(2);
    m_scene->addNumericDisplay(m_energyDisplay);

    m_unqueuedPopup = new KGamePopupItem;
    m_unqueuedPopup->setMessageOpacity(0.85);
    m_unqueuedPopup->setHideOnMouseClick(true);
    m_scene->addItem(m_unqueuedPopup);

    m_queuedPopup = new KGamePopupItem;
    m_queuedPopup->setMessageOpacity(0.85);
    m_queuedPopup->setHideOnMouseClick(true);
    m_scene->addItem(m_queuedPopup);

    connect(m_queuedPopup, &KGamePopupItem::hidden, this, &Coordinator::nextAnimationStage);
}

void Killbots::Coordinator::setAnimationSpeed(int speed)
{
    // Equation converts speed in the range 0 to 10 to a duration in the range
    // 1 to 0.05 seconds. There's probably a better way to do this.
    m_timeLine.setDuration(int(pow(1.35, -speed) * 1000));
}

void Killbots::Coordinator::requestNewGame()
{
    if (!m_busyAnimating || m_engine->isHeroDead()) {
        startNewGame();
    } else {
        m_newGameRequested = true;
    }
}

void Killbots::Coordinator::startNewGame()
{
    m_newGameRequested = false;
    m_repeatedAction = NoAction;
    m_queuedAction = NoAction;

    const Ruleset *ruleset = m_engine->ruleset();
    if (!ruleset || ruleset->fileName() != Settings::ruleset()) {
        ruleset = Ruleset::load(Settings::ruleset());
        if (!ruleset) {
            Settings::setRuleset(Settings::defaultRulesetValue());
            ruleset = Ruleset::load(Settings::ruleset());
        }
        m_engine->setRuleset(ruleset);
    }

    m_energyDisplay->setVisible(ruleset->energyEnabled());
    m_scene->setGridSize(ruleset->rows(), ruleset->columns());
    m_scene->doLayout();

    m_engine->startNewGame();

    startAnimation();
}

void Killbots::Coordinator::requestAction(int action)
{
    // If we're doing a repeated move, ignore the request and just stop the current movement.
    if (m_repeatedAction != NoAction && m_repeatedAction != WaitOutRound) {
        m_repeatedAction = NoAction;
    } else if (!m_engine->isHeroDead()) {
        if (!m_busyAnimating) {
            doAction(HeroAction(action));
        } else {
            m_queuedAction = HeroAction(action);
        }
    }
}

void Killbots::Coordinator::doAction(HeroAction action)
{
    bool actionSuccessful = false;
    bool boardFull = false;

    if (action <= Hold) {
        actionSuccessful = m_engine->moveHero(action);
        m_repeatedAction = action < 0 && actionSuccessful
                           ? action
                           : NoAction;
    } else if ((action == TeleportSafely || action == TeleportSafelyIfPossible)
               && m_engine->canSafeTeleport()
              ) {
        actionSuccessful = m_engine->teleportHeroSafely();
        boardFull = !actionSuccessful;
    } else if (action == Teleport || action == TeleportSafelyIfPossible) {
        actionSuccessful = m_engine->teleportHero();
    } else if (action == Vaporizer && m_engine->canUseVaporizer()) {
        actionSuccessful = m_engine->useVaporizer();
    } else if (action == WaitOutRound) {
        actionSuccessful = m_engine->waitOutRound();
        m_repeatedAction = WaitOutRound;
    }

    if (actionSuccessful) {
        if (action != Vaporizer) {
            m_engine->moveRobots();
        }
        m_engine->assessDamage();
        if (!m_engine->isRoundComplete() && action != Vaporizer) {
            m_engine->moveRobots(true);
            m_engine->assessDamage();
        }
        startAnimation();
    } else if (boardFull) {
        m_engine->resetBotCounts();
        startAnimation();
    }
}

void Killbots::Coordinator::animationDone()
{
    m_busyAnimating = false;

    if (m_newGameRequested) {
        startNewGame();
    } else if (m_engine->isHeroDead()) {
        m_scene->forgetHero();
        m_engine->endGame();
    } else if (m_engine->isRoundComplete()) {
        m_repeatedAction = NoAction;
        m_queuedAction = NoAction;
        m_engine->startNewRound();
        if (m_engine->isBoardFull()) {
            m_engine->resetBotCounts();
        }
        startAnimation();
    } else if (m_repeatedAction != NoAction) {
        doAction(m_repeatedAction);
    } else if (m_queuedAction != NoAction) {
        doAction(m_queuedAction);
        m_queuedAction = NoAction;
    }
}

void Killbots::Coordinator::startAnimation()
{
    m_busyAnimating = true;
    startAnimationStage();
}

void Killbots::Coordinator::startAnimationStage()
{
    const QString &message = m_stages.first().message;

    if (m_timeLine.duration() < 60 && message.isEmpty()) {
        animate(1.0);
        nextAnimationStage();
    } else {
        if (!message.isEmpty()) {
            showQueuedMessage(message);
        }

        m_timeLine.start();
    }
}

void Killbots::Coordinator::animate(qreal value)
{
    AnimationStage &stage = m_stages.first();

    if (stage.newRound != -1) {
        m_roundDisplay->setValue(int(stage.oldRound + value * (stage.newRound - stage.oldRound)));
    }

    if (stage.newScore != -1) {
        m_scoreDisplay->setValue(int(stage.oldScore + value * (stage.newScore - stage.oldScore)));
    }

    if (stage.newEnemyCount != -1) {
        m_enemyCountDisplay->setValue(int(stage.oldEnemyCount + value * (stage.newEnemyCount - stage.oldEnemyCount)));
    }

    if (stage.newEnergy != -1) {
        m_energyDisplay->setValue(int(stage.oldEnergy + value * (stage.newEnergy - stage.oldEnergy)));
    }

    m_scene->animateSprites(stage.spritesToCreate,
                            stage.spritesToSlide,
                            stage.spritesToTeleport,
                            stage.spritesToDestroy,
                            value
                           );
}

void Killbots::Coordinator::nextAnimationStage()
{
    // Wait for both the timeline and the popup to finish before moving to the next stage.
    if (m_timeLine.state() != QTimeLine::Running && !m_queuedPopup->isVisible()) {
        m_stages.removeFirst();

        if (!m_stages.isEmpty()) {
            startAnimationStage();
        } else {
            animationDone();
        }
    }
}

void Killbots::Coordinator::beginNewAnimationStage()
{
    if (m_stages.isEmpty()) {
        AnimationStage newStage;
        newStage.oldRound = m_roundDisplay->value();
        newStage.oldScore = m_scoreDisplay->value();
        newStage.oldEnemyCount = m_enemyCountDisplay->value();
        newStage.oldEnergy = m_energyDisplay->value();
        m_stages << newStage;
    } else if (!m_stages.last().isEmpty()) {
        AnimationStage newStage;
        const AnimationStage &lastStage = m_stages.last();
        newStage.oldRound = lastStage.newRound == -1 ? lastStage.oldRound : lastStage.newRound;
        newStage.oldScore = lastStage.newScore == -1 ? lastStage.oldScore : lastStage.newScore;
        newStage.oldEnemyCount = lastStage.newEnemyCount == -1 ? lastStage.oldEnemyCount : lastStage.newEnemyCount;
        newStage.oldEnergy = lastStage.newEnergy == -1 ? lastStage.oldEnergy : lastStage.newEnergy;
        m_stages << newStage;
    }
}

Killbots::Sprite *Killbots::Coordinator::createSprite(SpriteType type, QPoint position)
{
    Sprite *sprite = m_scene->createSprite(type, position);
    m_stages.last().spritesToCreate << sprite;
    return sprite;
}

void Killbots::Coordinator::slideSprite(Sprite *sprite, QPoint position)
{
    sprite->enqueueGridPos(position);
    m_stages.last().spritesToSlide << sprite;
}

void Killbots::Coordinator::teleportSprite(Sprite *sprite, QPoint position)
{
    sprite->enqueueGridPos(position);
    m_stages.last().spritesToTeleport << sprite;
}

void Killbots::Coordinator::destroySprite(Sprite *sprite)
{
    if (sprite->spriteType() == Hero) {
        m_scene->forgetHero();
    }
    m_stages.last().spritesToDestroy << sprite;
}

void Killbots::Coordinator::updateRound(int round)
{
    m_stages.last().newRound = round;
}

void Killbots::Coordinator::updateScore(int score)
{
    m_stages.last().newScore = score;
}

void Killbots::Coordinator::updateEnemyCount(int enemyCount)
{
    m_stages.last().newEnemyCount = enemyCount;
}

void Killbots::Coordinator::updateEnergy(int energy)
{
    m_stages.last().newEnergy = energy;
}

void Killbots::Coordinator::showQueuedMessage(const QString &message)
{
    if (m_unqueuedPopup->isVisible()) {
        m_unqueuedPopup->hide();
    }
    KGamePopupItem::Position corner = m_scene->views().first()->layoutDirection() == Qt::LeftToRight ? KGamePopupItem::TopRight : KGamePopupItem::TopLeft;
    m_queuedPopup->setMessageTimeout(3000);
    m_queuedPopup->showMessage(message, corner, KGamePopupItem::ReplacePrevious);
}

void Killbots::Coordinator::showUnqueuedMessage(const QString &message, int timeout)
{
    if (!m_queuedPopup->isVisible()) {
        KGamePopupItem::Position corner = m_scene->views().first()->layoutDirection() == Qt::LeftToRight ? KGamePopupItem::TopRight : KGamePopupItem::TopLeft;
        m_unqueuedPopup->setMessageTimeout(timeout);
        m_unqueuedPopup->showMessage(message, corner, KGamePopupItem::ReplacePrevious);
    }
}

void Killbots::Coordinator::showRoundCompleteMessage()
{
    m_stages.last().message = i18nc("@info", "Round complete.");
}

void Killbots::Coordinator::showBoardFullMessage()
{
    m_stages.last().message = i18nc("@info", "Board is full.\nResetting enemy counts.");
}

void Killbots::Coordinator::showNewGameMessage()
{
    showUnqueuedMessage(i18nc("@info", "New game."));
}

void Killbots::Coordinator::showGameOverMessage()
{
    showUnqueuedMessage(i18nc("@info", "Game over."), 15000);
}

#include "moc_coordinator.cpp"
