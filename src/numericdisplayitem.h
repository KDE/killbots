/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_NUMERICDISPLAYITEM_H
#define KILLBOTS_NUMERICDISPLAYITEM_H

#include <KGameRenderedItem>

#include <QFont>

namespace Killbots
{

class NumericDisplayItem : public QObject, public KGameRenderedItem
{
    Q_OBJECT

public: // functions
    explicit NumericDisplayItem(const QString &label = QString(), QGraphicsItem *parent = nullptr);
    ~NumericDisplayItem() override;

    int value() const;
    QString label() const;
    int digits() const;
    QFont font() const;
    QSize preferredSize();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

public Q_SLOTS:
    void setValue(int value);
    void setLabel(const QString &label);
    void setDigits(int digits);
    void setFont(const QFont &font);

private: // data members
    QString m_label;
    int m_value;
    int m_digits;

    int m_margin;

    QFont m_font;
    QFont m_boldFont;
};
}

#endif
