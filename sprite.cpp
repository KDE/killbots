/*
 *  Copyright 2006-2009  Parker Coates <coates@kde.org>
 *
 *  This file is part of Killbots.
 *
 *  Killbots is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Killbots is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Killbots. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sprite.h"

#include "renderer.h"

#include <QPainter>

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
        setSpriteKey("hero");
        break;
    case Robot :
        setSpriteKey("enemy");
        break;
    case Fastbot :
        setSpriteKey("fastenemy");
        break;
    case Junkheap :
        setSpriteKey("junkheap");
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

