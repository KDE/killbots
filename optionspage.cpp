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
    QStringList clickActionList;
    clickActionList << i18n("Nothing");
    clickActionList << i18n("Step");
    clickActionList << i18n("Repeated Step");
    clickActionList << i18n("Teleport");
    clickActionList << i18n("Teleport Safely");
    clickActionList << i18n("Teleport (Safely If Possible)");
    clickActionList << i18n("Wait Out Round");

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
    QLabel *slowLabel = new QLabel(i18n("Slow"));
    slowLabel->setAlignment(Qt::AlignLeft);
    QLabel *fastLabel = new QLabel(i18n("Fast"));
    fastLabel->setAlignment(Qt::AlignCenter);
    QLabel *instantLabel = new QLabel(i18n("Instant"));
    instantLabel->setAlignment(Qt::AlignRight);

    QGridLayout *speedLayout = new QGridLayout();
    speedLayout->setContentsMargins(0, 0, 0, 0);
    speedLayout->setSpacing(0);
    speedLayout->addWidget(kcfg_AnimationSpeed, 0, 0, 1, 3);
    speedLayout->addWidget(slowLabel, 1, 0);
    speedLayout->addWidget(fastLabel, 1, 1);
    speedLayout->addWidget(instantLabel, 1, 2);

    QLabel *speedLabel = new QLabel(i18n("Animation &speed:"));
    speedLabel->setBuddy(kcfg_AnimationSpeed);

    kcfg_PreventUnsafeMoves = new QCheckBox(i18n("Prevent &unsafe moves"));
    kcfg_PreventUnsafeMoves->setObjectName(QStringLiteral("kcfg_PreventUnsafeMoves"));

    QFormLayout *formLayout = new QFormLayout(this);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->addRow(i18n("&Middle-click action:"), kcfg_MiddleClickAction);
    formLayout->addRow(i18n("&Right-click action:"), kcfg_RightClickAction);
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));
    formLayout->addRow(speedLabel, speedLayout);
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));
    formLayout->addRow(nullptr, kcfg_PreventUnsafeMoves);
}

Killbots::OptionsPage::~OptionsPage()
{
}

#include "moc_optionspage.cpp"
