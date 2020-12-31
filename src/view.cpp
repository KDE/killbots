/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "view.h"

#include <QResizeEvent>

Killbots::View::View(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setMinimumSize(QSize(300, 200));
    setFrameShape(QFrame::NoFrame);

    // This makes the background of the widget transparent so that Oxygen's
    // (or any other style's) window gradient is visible in unpainted areas of
    // the scene.
    QPalette p = palette();
    QColor c = p.color(QPalette::Base);
    c.setAlpha(0);
    p.setColor(QPalette::Base, c);
    setPalette(p);
    setBackgroundRole(QPalette::Base);

    setCacheMode(QGraphicsView::CacheBackground);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    // Including QGraphicsView::DontAdjustForAntialiasing here sometimes caused
    // painting traces in certain situations like pushing junkheaps.
    setOptimizationFlags( QGraphicsView::DontSavePainterState);
}

Killbots::View::~View()
{
}

void Killbots::View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    Q_EMIT sizeChanged(event->size());
}

#include "moc_view.cpp"
