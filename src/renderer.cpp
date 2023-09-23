/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2010 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "renderer.h"

#include "settings.h"

// KDEGames
#include <KgTheme>
#include <KgThemeProvider>

#include <QCursor>

static KgThemeProvider *provider()
{
    KgThemeProvider *prov = new KgThemeProvider;
    prov->discoverThemes(
        QStringLiteral("themes"),   // theme file location
        QStringLiteral("robotkill") // default theme file name
    );
    return prov;
}

static Killbots::Renderer *r = nullptr;

Killbots::Renderer *Killbots::Renderer::self()
{
    if (!r) {
        r = new Killbots::Renderer();
    }
    return r;
}

void Killbots::Renderer::cleanup()
{
    delete r;
    r = nullptr;
}

Killbots::Renderer::Renderer()
    : KGameRenderer(provider())
{
}

QCursor Killbots::Renderer::cursorFromAction(int direction)
{
    QString element = QStringLiteral("cursor%1").arg(direction);
    QPixmap pixmap = spritePixmap(element, QSize(42, 42));
    return QCursor(pixmap);
}

QColor Killbots::Renderer::textColor()
{
    if (m_cachedTheme != theme()->identifier()) {
        m_textColor = spritePixmap(QStringLiteral("textcolor"), QSize(3, 3)).toImage().pixel(1, 1);
        m_cachedTheme = theme()->identifier();
    }
    return m_textColor;
}

qreal Killbots::Renderer::aspectRatio()
{
    const QRectF tileRect = boundsOnSprite(QStringLiteral("cell"));
    return qBound<qreal>(0.3333, tileRect.width() / tileRect.height(), 3.0);
}
