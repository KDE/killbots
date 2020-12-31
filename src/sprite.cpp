/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sprite.h"

#include "renderer.h"


Killbots::Sprite::Sprite()
    : KGameRenderedItem(Renderer::self(), QString())
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setTransformationMode(Qt::FastTransformation);
}

Killbots::Sprite::~Sprite()
{
}

Killbots::SpriteType Killbots::Sprite::spriteType() const
{
    return m_type;
}

void Killbots::Sprite::setSpriteType(Killbots::SpriteType type)
{
    Q_ASSERT(type != NoSprite);

    m_type = type;

    switch (m_type) {
    case Hero :
        setSpriteKey(QStringLiteral("hero"));
        break;
    case Robot :
        setSpriteKey(QStringLiteral("enemy"));
        break;
    case Fastbot :
        setSpriteKey(QStringLiteral("fastenemy"));
        break;
    case Junkheap :
        setSpriteKey(QStringLiteral("junkheap"));
        break;
    default :
        break;
    }
}

void Killbots::Sprite::enqueueGridPos(QPoint position)
{
    m_gridPositions << position;
}

QPoint Killbots::Sprite::currentGridPos() const
{
    return m_gridPositions.first();
}

QPoint Killbots::Sprite::nextGridPos() const
{
    Q_ASSERT(m_gridPositions.size() > 1);
    return m_gridPositions.at(1);
}

QPoint Killbots::Sprite::gridPos() const
{
    return m_gridPositions.last();
}

void Killbots::Sprite::advanceGridPosQueue()
{
    if (m_gridPositions.size() > 1) {
        m_gridPositions.removeFirst();
    }
}

int Killbots::Sprite::type() const
{
    return Type;
}

void Killbots::Sprite::receivePixmap(const QPixmap &pixmap)
{
    KGameRenderedItem::receivePixmap(pixmap);
    setOffset(-0.5 * QPointF(pixmap.width(), pixmap.height()));
}

