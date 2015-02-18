/*
 *  Copyright 2007-2009  Parker Coates <coates@kde.org>
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
    setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);
}

Killbots::View::~View()
{
}

void Killbots::View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    emit sizeChanged(event->size());
}

#include "moc_view.cpp"
