/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scene.h"

#include "numericdisplayitem.h"
#include "renderer.h"
#include "settings.h"

#include <KGamePopupItem>

#include "killbots_debug.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>

#include <cmath>

Killbots::Scene::Scene(QObject *parent)
    : QGraphicsScene(parent),
      m_hero(nullptr),
      m_rows(0),
      m_columns(0)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
}

Killbots::Scene::~Scene()
{
}

void Killbots::Scene::addNumericDisplay(NumericDisplayItem *displayItem)
{
    addItem(displayItem);
    displayItem->setPos(-1000000, 0);
    m_numericDisplays << displayItem;
}

void Killbots::Scene::setGridSize(int rows, int columns)
{
    if (m_rows != rows || m_columns != columns) {
        m_rows = rows;
        m_columns = columns;
    }
}

void Killbots::Scene::forgetHero()
{
    m_hero = nullptr;
}

Killbots::Sprite *Killbots::Scene::createSprite(SpriteType type, QPoint position)
{
    Sprite *sprite = new Sprite();
    sprite->setSpriteType(type);
    sprite->setRenderSize(m_cellSize);
    sprite->enqueueGridPos(position);
    updateSpritePos(sprite, position);
    sprite->setTransform(QTransform::fromScale(0, 0), true);
    // A bit of a hack, but we use the sprite type for stacking order.
    sprite->setZValue(type);

    addItem(sprite);

    if (type == Hero) {
        m_hero = sprite;
    }

    return sprite;
}

void Killbots::Scene::animateSprites(const QList<Sprite *> &newSprites,
                                     const QList<Sprite *> &slidingSprites,
                                     const QList<Sprite *> &teleportingSprites,
                                     const QList<Sprite *> &destroyedSprites,
                                     qreal value
                                    ) const
{
    static bool halfDone = false;

    if (value == 0.0) {
        halfDone = false;
    } else if (value < 1.0) {
        for (Sprite *sprite : newSprites) {
            sprite->resetTransform();
            sprite->setTransform(QTransform::fromScale(value, value), true);
        }

        for (Sprite *sprite : slidingSprites) {
            QPointF posInGridCoordinates = value * QPointF(sprite->nextGridPos() - sprite->currentGridPos()) + sprite->currentGridPos();
            sprite->setPos(QPointF(posInGridCoordinates.x() * m_cellSize.width(), posInGridCoordinates.y() * m_cellSize.height()));
        }

        qreal scaleFactor = value < 0.5
                            ? 1.0 - 2 * value
                            : 2 * value - 1.0;

        if (!halfDone && value >= 0.5) {
            halfDone = true;
            for (Sprite *sprite : teleportingSprites) {
                updateSpritePos(sprite, sprite->nextGridPos());
            }
        }

        for (Sprite *sprite : teleportingSprites) {
            sprite->resetTransform();
            sprite->setTransform(QTransform::fromScale(scaleFactor, scaleFactor), true);
        }

        for (Sprite *sprite : destroyedSprites) {
            sprite->resetTransform();
            sprite->setTransform(QTransform::fromScale(1 - value, 1 - value), true);
            sprite->setTransform(QTransform().rotate(value * 180), true);
        }
    } else {
        for (auto& sprites : {newSprites, slidingSprites, teleportingSprites}) {
            for (Sprite *sprite : sprites) {
                sprite->resetTransform();
                sprite->advanceGridPosQueue();
                updateSpritePos(sprite, sprite->currentGridPos());
            }
        }

        qDeleteAll(destroyedSprites);
    }
}

void Killbots::Scene::doLayout()
{
    QSize size = views().first()->size();

    // If no game has been started
    if (m_rows == 0 || m_columns == 0) {
        setSceneRect(QRectF(QPointF(0, 0), size));
        return;
    }

    //qCDebug(KILLBOTS_LOG) << "Laying out scene at" << size;

    // Make certain layout properties proportional to the scene height,
    // but clamp them between reasonable values. There's probably room for more
    // tweaking here.
    const int baseDimension = qMin(size.width(), size.height()) / 35;
    const int spacing = qBound(5, baseDimension, 15);
    const int newFontPixelSize = qBound(QFontInfo(QFont()).pixelSize(), baseDimension, 25);
    const qreal aspectRatio = Renderer::self()->aspectRatio();

    QSize displaySize;
    // If the font size has changed, resize all the displays (visible or not).
    if (m_numericDisplays.first()->font().pixelSize() != newFontPixelSize) {
        QFont font;
        font.setPixelSize(newFontPixelSize);

        for (NumericDisplayItem *display : std::as_const(m_numericDisplays)) {
            display->setFont(font);
            displaySize = displaySize.expandedTo(display->preferredSize());
        }
        for (NumericDisplayItem *display : std::as_const(m_numericDisplays)) {
            display->setRenderSize(displaySize);
        }
    } else {
        displaySize = m_numericDisplays.first()->boundingRect().size().toSize();
    }

    // The rest of the function deals only with a list of visible displays.
    QList<NumericDisplayItem *> visibleDisplays;
    for (NumericDisplayItem *display : std::as_const(m_numericDisplays)) {
        if (display->isVisible()) {
            visibleDisplays << display;
        }
    }

    // Determine the total width required to arrange the displays horizontally.
    const int widthOfDisplaysOnTop = visibleDisplays.size() * displaySize.width()
                                     + (visibleDisplays.size() - 1) * spacing;

    // The displays can either be placed centered, across the top of the
    // scene or top-aligned, down the side of the scene. We first calculate
    // what the cell size would be for both options.
    int availableWidth = size.width() - 3 * spacing - displaySize.width();
    int availableHeight = size.height() - 2 * spacing;
    const qreal cellWidthSide = (availableWidth / m_columns < availableHeight / m_rows * aspectRatio)
                                ? availableWidth / m_columns
                                : availableHeight / m_rows * aspectRatio;

    availableWidth = size.width() - 2 * spacing;
    availableHeight = size.height() - 3 * spacing - displaySize.height();
    const qreal cellWidthTop = (availableWidth / m_columns < availableHeight / m_rows * aspectRatio)
                               ? availableWidth / m_columns
                               : availableHeight / m_rows * aspectRatio;

    // If placing the displays on top would result in larger cells, we take
    // that option, but only if the displays would actually fit.
    const bool displaysOnTop = (cellWidthTop > cellWidthSide && size.width() > widthOfDisplaysOnTop);
    const qreal newCellWidth = displaysOnTop ? cellWidthTop : cellWidthSide;
    m_cellSize = QSize(qRound(newCellWidth), qRound(newCellWidth / aspectRatio));

    const auto items = this->items();
    for (QGraphicsItem *item : items) {
        Sprite *sprite = qgraphicsitem_cast<Sprite *>(item);
        if (sprite) {
            sprite->setRenderSize(m_cellSize);
            updateSpritePos(sprite, sprite->currentGridPos());
        }
    }

    if (displaysOnTop) {
        // Set the sceneRect to center the grid if possible, but ensure the display items are visible
        const qreal sceneRectXPos = -(size.width() - m_cellSize.width() * (m_columns - 1)) / 2.0;
        const qreal centeredYPos = - (size.height() - m_cellSize.height() * (m_rows - 1)) / 2.0;
        const qreal indentedYPos = - (m_cellSize.height() / 2.0 + 2 * spacing + displaySize.height());
        const qreal sceneRectYPos = qMin(centeredYPos, indentedYPos);

        // Position the display items centered at the top of the scene
        const qreal displayYPos = (sceneRectYPos - (displaySize.height() + m_cellSize.height() / 2.0)) / 2;

        int xPos = sceneRectXPos + (size.width() - widthOfDisplaysOnTop) / 2.0;
        for (NumericDisplayItem *display : std::as_const(visibleDisplays)) {
            display->setPos(xPos, displayYPos);
            xPos += displaySize.width() + spacing;
        }

        setSceneRect(QRectF(sceneRectXPos, sceneRectYPos, size.width(), size.height()));
    } else {
        qreal sceneRectXPos;
        const qreal centeredXPos = - (size.width() - m_cellSize.width() * (m_columns - 1)) / 2.0;
        const qreal sceneRectYPos = -(size.height() - m_cellSize.height() * (m_rows - 1)) / 2.0;
        qreal displayXPos;

        // If the application layout is LTR, place the displays on left,
        // otherwise, place them on the right.
        if (views().first()->layoutDirection() == Qt::LeftToRight) {
            // Set the sceneRect to center the grid if possible, but ensure the display items are visible
            const qreal indentedXPos = - (m_cellSize.width() / 2.0 + 2 * spacing + displaySize.width());
            sceneRectXPos = qMin(centeredXPos, indentedXPos);

            // Position the display items to the left of the grid
            displayXPos = - (spacing + displaySize.width() + m_cellSize.width() / 2);
        } else {
            // Set the sceneRect to center the grid if possible, but ensure the display items are visible
            const qreal indentedXPos = (m_cellSize.width() * m_columns + 1 * spacing + displaySize.width()) - size.width();
            sceneRectXPos = qMax(centeredXPos, indentedXPos);

            // Position the display items to the right of the grid
            displayXPos = m_cellSize.width() * (m_columns - 0.5) + spacing;
        }

        int yPos = -m_cellSize.height() / 2;
        for (NumericDisplayItem *display : std::as_const(visibleDisplays)) {
            display->setPos(displayXPos, yPos);
            yPos += displaySize.height() + spacing;
        }

        setSceneRect(QRectF(sceneRectXPos, sceneRectYPos, size.width(), size.height()));
    }

    // Update the scene background
    QPainter p;
    QRect gridRect(-sceneRect().x() - m_cellSize.width() / 2,
                   -sceneRect().y() - m_cellSize.height() / 2,
                   m_columns * m_cellSize.width(),
                   m_rows * m_cellSize.height()
                  );

    QPixmap unrotated = Renderer::self()->spritePixmap(QStringLiteral("background"), size);
    p.begin(&unrotated);
    p.drawTiledPixmap(gridRect, Renderer::self()->spritePixmap(QStringLiteral("cell"), m_cellSize));
    p.end();

    // The background brush begins painting at 0,0 but our sceneRect doesn't
    // start at 0,0 so we have to "rotate" the pixmap so that it looks right
    // when tiled.
    QPixmap background(size);
    background.fill(Qt::transparent);
    p.begin(&background);
    p.drawTiledPixmap(background.rect(), unrotated, -sceneRect().topLeft().toPoint());
    p.end();

    setBackgroundBrush(background);

    update();
}

void Killbots::Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    getMouseDirection(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void Killbots::Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    HeroAction actionFromPosition = getMouseDirection(event->scenePos());

    if (actionFromPosition != NoAction) {
        Settings::ClickAction userAction = Settings::Nothing;

        if (event->button() == Qt::LeftButton) {
            if (event->modifiers() & Qt::ControlModifier) {
                userAction = Settings::middleClickAction();
            } else {
                userAction = Settings::Step;
            }
        } else if (event->button() == Qt::RightButton) {
            userAction = Settings::rightClickAction();
        } else if (event->button() == Qt::MiddleButton) {
            userAction = Settings::middleClickAction();
        }

        if (userAction == Settings::Step) {
            Q_EMIT clicked(actionFromPosition);
        } else if (userAction == Settings::RepeatedStep) {
            Q_EMIT clicked(-actionFromPosition - 1);
        } else if (userAction == Settings::Teleport) {
            Q_EMIT clicked(Teleport);
        } else if (userAction == Settings::TeleportSafely) {
            Q_EMIT clicked(TeleportSafely);
        } else if (userAction == Settings::TeleportSafelyIfPossible) {
            Q_EMIT clicked(TeleportSafelyIfPossible);
        } else if (userAction == Settings::WaitOutRound) {
            Q_EMIT clicked(WaitOutRound);
        }
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

Killbots::HeroAction Killbots::Scene::getMouseDirection(QPointF cursorPosition) const
{
    HeroAction result;
    const bool heroOnScreen = m_hero && sceneRect().contains(m_hero->sceneBoundingRect());

    if (heroOnScreen && !popupAtPosition(cursorPosition)) {
        if (m_hero->sceneBoundingRect().contains(cursorPosition)) {
            result = Hold;
        } else {
            const qreal piOver4 = 0.78539816339744830961566L;

            QPointF delta = cursorPosition - m_hero->sceneBoundingRect().center();
            int direction = qRound(atan2(-delta.y(), delta.x()) / piOver4);
            if (direction < 0) {
                direction += 8;
            }

            result = static_cast<HeroAction>(direction);
        }

        views().first()->setCursor(Renderer::self()->cursorFromAction(result));
    } else {
        views().first()->unsetCursor();
        result = NoAction;
    }

    return result;
}

bool Killbots::Scene::popupAtPosition(QPointF position) const
{
    const auto itemsAtPos = items(position);
    for (QGraphicsItem *item : itemsAtPos) {
        if (dynamic_cast<KGamePopupItem *>(item) != nullptr) {
            return true;
        }
    }
    return false;
}

void Killbots::Scene::updateSpritePos(Sprite *sprite, QPoint gridPosition) const
{
    sprite->setPos(gridPosition.x() * m_cellSize.width(), gridPosition.y() * m_cellSize.height());
}

#include "moc_scene.cpp"
