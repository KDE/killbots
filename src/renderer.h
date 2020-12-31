/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2010 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_RENDERER_H
#define KILLBOTS_RENDERER_H

#include <KGameRenderer>

#include <QHash>

namespace Killbots
{
class Renderer : public KGameRenderer
{
public:
    static Renderer *self();
    static void cleanup();

    Renderer();

    QCursor cursorFromAction(int direction);
    QColor textColor();
    qreal aspectRatio();

private:
    QColor m_textColor;
    QByteArray m_cachedTheme;
};
}

#endif
