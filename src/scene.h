/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_SCENE_H
#define KILLBOTS_SCENE_H

#include "actions.h"
#include "sprite.h"

#include <QGraphicsScene>

namespace Killbots
{
class NumericDisplayItem;

class Scene : public QGraphicsScene
{
    Q_OBJECT

public: // functions
    explicit Scene(QObject *parent = nullptr);
    ~Scene() override;

    void addNumericDisplay(NumericDisplayItem *displayItem);
    void setGridSize(int rows, int columns);
    void forgetHero();

    Sprite *createSprite(SpriteType type, QPoint position);
    void animateSprites(const QList<Sprite *> &newSprites,
                        const QList<Sprite *> &slidingSprites,
                        const QList<Sprite *> &teleportingSprites,
                        const QList<Sprite *> &destroyedSprites,
                        qreal value
                       ) const;

public Q_SLOTS:
    void doLayout();

Q_SIGNALS:
    void clicked(int action);

protected: // functions
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private: // functions
    HeroAction getMouseDirection(QPointF cursorPosition) const;
    bool popupAtPosition(QPointF position) const;
    void updateSpritePos(Sprite *sprite, QPoint gridPosition) const;

private: // data members
    Sprite *m_hero;

    QList<NumericDisplayItem *> m_numericDisplays;

    QSize m_cellSize;
    int m_rows;
    int m_columns;
};
}

#endif
