/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_OPTIONSPAGE_H
#define KILLBOTS_OPTIONSPAGE_H

class KComboBox;

class QCheckBox;
class QSlider;
#include <QWidget>

namespace Killbots
{
class OptionsPage : public QWidget
{
    Q_OBJECT

public: // functions
    explicit OptionsPage(QWidget *parent = nullptr);
    ~OptionsPage() override;

public: // data members
    QCheckBox *kcfg_PreventUnsafeMoves;
    KComboBox *kcfg_MiddleClickAction;
    KComboBox *kcfg_RightClickAction;
    QSlider *kcfg_AnimationSpeed;
};
}

#endif
