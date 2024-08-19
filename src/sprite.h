/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_SPRITE_H
#define KILLBOTS_SPRITE_H

#include <KGameRenderedItem>

namespace Killbots
{
enum SpriteType {
    NoSprite,
    Junkheap,
    Hero,
    Robot,
    Fastbot
};

class Sprite : public KGameRenderedItem
{
public: // types
    enum {
        Type = UserType + 1
    };

public: // functions
    explicit Sprite();
    ~Sprite() override;

    SpriteType spriteType() const;
    void setSpriteType(SpriteType type);

    void enqueueGridPos(QPoint position);
    QPoint currentGridPos() const;
    QPoint nextGridPos() const;
    QPoint gridPos() const;
    void advanceGridPosQueue();

    int type() const override;

protected:
    void receivePixmap(const QPixmap &pixmap) override;

private: // data members
    SpriteType m_type;
    QList<QPoint> m_gridPositions;
};
}

#endif
