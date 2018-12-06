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

#ifndef KILLBOTS_VIEW_H
#define KILLBOTS_VIEW_H

#include <QGraphicsView>
class QResizeEvent;

namespace Killbots
{
class View : public QGraphicsView
{
    Q_OBJECT

public: // functions
    explicit View(QGraphicsScene *scene, QWidget *parent = nullptr);
    ~View() override;

Q_SIGNALS:
    void sizeChanged(QSize newSize);

protected: // functions
    void resizeEvent(QResizeEvent *event) override;
};
}

#endif
