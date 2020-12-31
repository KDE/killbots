/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "optionspage.h"

#include <KComboBox>
#include <KLocalizedString>

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>

Killbots::OptionsPage::OptionsPage(QWidget *parent)
    : QWidget(parent)
{
    const QStringList clickActionList {
        i18nc("@item:inlistbox No action on click", "Nothing"),
        i18nc("@action", "Step"),
        i18nc("@action", "Repeated Step"),
        i18nc("@action", "Teleport"),
        i18nc("@action", "Teleport Safely"),
        i18nc("@action", "Teleport (Safely If Possible)"),
        i18nc("@action", "Wait Out Round"),
    };

    kcfg_MiddleClickAction = new KComboBox();
    kcfg_MiddleClickAction->setObjectName(QStringLiteral("kcfg_MiddleClickAction"));
    kcfg_MiddleClickAction->addItems(clickActionList);

    kcfg_RightClickAction = new KComboBox();
    kcfg_RightClickAction->setObjectName(QStringLiteral("kcfg_RightClickAction"));
    kcfg_RightClickAction->addItems(clickActionList);

    kcfg_AnimationSpeed = new QSlider(Qt::Horizontal);
    kcfg_AnimationSpeed->setObjectName(QStringLiteral("kcfg_AnimationSpeed"));
    kcfg_AnimationSpeed->setSingleStep(1);
    kcfg_AnimationSpeed->setMinimumWidth(200);
    QLabel *slowLabel = new QLabel(i18nc("@item:inrange", "Slow"));
    slowLabel->setAlignment(Qt::AlignLeft);
    QLabel *fastLabel = new QLabel(i18nc("@item:inrange", "Fast"));
    fastLabel->setAlignment(Qt::AlignCenter);
    QLabel *instantLabel = new QLabel(i18nc("@item:inrange", "Instant"));
    instantLabel->setAlignment(Qt::AlignRight);

    QGridLayout *speedLayout = new QGridLayout();
    speedLayout->setContentsMargins(0, 0, 0, 0);
    speedLayout->setSpacing(0);
    speedLayout->addWidget(kcfg_AnimationSpeed, 0, 0, 1, 3);
    speedLayout->addWidget(slowLabel, 1, 0);
    speedLayout->addWidget(fastLabel, 1, 1);
    speedLayout->addWidget(instantLabel, 1, 2);

    QLabel *speedLabel = new QLabel(i18nc("@label:slider", "Animation &speed:"));
    speedLabel->setBuddy(kcfg_AnimationSpeed);

    kcfg_PreventUnsafeMoves = new QCheckBox(i18nc("@option:check", "Prevent &unsafe moves"));
    kcfg_PreventUnsafeMoves->setObjectName(QStringLiteral("kcfg_PreventUnsafeMoves"));

    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->addRow(i18nc("@label:listbox", "&Middle-click action:"), kcfg_MiddleClickAction);
    formLayout->addRow(i18nc("@label:listbox", "&Right-click action:"), kcfg_RightClickAction);
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));
    formLayout->addRow(speedLabel, speedLayout);
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));
    formLayout->addRow(nullptr, kcfg_PreventUnsafeMoves);
}

Killbots::OptionsPage::~OptionsPage()
{
}

#include "moc_optionspage.cpp"
