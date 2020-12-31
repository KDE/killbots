/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_RULESETDETAILSDIALOG_H
#define KILLBOTS_RULESETDETAILSDIALOG_H

#include <QDialog>

#include <QMap>
class QLabel;

namespace Killbots
{
class Ruleset;

class RulesetDetailsDialog : public QDialog
{
public:
    explicit RulesetDetailsDialog(QWidget *parent = nullptr);
    void loadRuleset(const Ruleset *ruleset);

private:
//      static const QStringList maskedItems;
//      static const QStringList junkheapEnumText;

    QMap<QString, QLabel *> m_labels;
    QWidget *mMainWidget;
};

}

#endif // KILLBOTS_RULESETDETAILSDIALOG_H
