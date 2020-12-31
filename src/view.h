/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
