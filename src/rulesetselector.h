/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_RULESETSELECTOR_H
#define KILLBOTS_RULESETSELECTOR_H

class KLineEdit;

#include <QMap>
class QLabel;
class QListWidget;
#include <QWidget>

namespace Killbots
{
class Ruleset;
class RulesetDetailsDialog;

class RulesetSelector : public QWidget
{
    Q_OBJECT

public: // functions
    explicit RulesetSelector(QWidget *parent = nullptr);
    virtual ~RulesetSelector();

public: // data members
    KLineEdit *kcfg_Ruleset;

private: // functions
    void findRulesets();

private Q_SLOTS:
    void selectionChanged(const QString &rulesetName);
    void showDetailsDialog();

private: // data members
    QListWidget *m_listWidget;
    QLabel *m_author;
    QLabel *m_authorContact;
    QLabel *m_description;
    QMap< QString, const Ruleset * > m_rulesetMap;
    RulesetDetailsDialog *m_detailsDialog;
};
}

#endif
