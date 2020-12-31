/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "numericdisplayitem.h"

#include "renderer.h"

#include <QFontMetrics>
#include <QPainter>

Killbots::NumericDisplayItem::NumericDisplayItem(const QString &label, QGraphicsItem *parent)
    : QObject(),
      KGameRenderedItem(Renderer::self(), QStringLiteral("status"), parent),
      m_label(label),
      m_value(0),
      m_digits(3)
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setFont(QFont());
    setRenderSize(preferredSize());
}

Killbots::NumericDisplayItem::~NumericDisplayItem()
{
}

int Killbots::NumericDisplayItem::value() const
{
    return m_value;
}

QString Killbots::NumericDisplayItem::label() const
{
    return m_label;
}

int Killbots::NumericDisplayItem::digits() const
{
    return m_digits;
}

QFont Killbots::NumericDisplayItem::font() const
{
    return m_font;
}

QSize Killbots::NumericDisplayItem::preferredSize()
{
    QSize labelSize = QFontMetrics(m_font).boundingRect(m_label).size();
    QSize digitsSize = QFontMetrics(m_boldFont).boundingRect(QString(m_digits + 1, QLatin1Char('8'))).size();

    return QSize(labelSize.width() + digitsSize.width() + 2 * m_margin,
                 qMax(labelSize.height(), digitsSize.height()) + 2 * m_margin
                );
}

void Killbots::NumericDisplayItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    KGameRenderedItem::paint(p, option, widget);

    p->save();

    QRectF textRect = boundingRect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
    p->setPen(Renderer::self()->textColor());

    p->setFont(m_font);
    p->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, m_label);

    p->setFont(m_boldFont);
    p->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, QString::number(m_value));

    p->restore();
}

void Killbots::NumericDisplayItem::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        update();
    }
}

void Killbots::NumericDisplayItem::setLabel(const QString &label)
{
    if (label != m_label) {
        m_label = label;
        update();
    }
}

void Killbots::NumericDisplayItem::setDigits(int digits)
{
    if (digits != m_digits) {
        m_digits = digits;
    }
}

void Killbots::NumericDisplayItem::setFont(const QFont &font)
{
    m_font = font;
    m_boldFont = m_font;
    m_boldFont.setBold(true);

    m_margin = int(QFontMetrics(m_boldFont).height() * 0.6);
}

