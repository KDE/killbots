/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_RULESET_H
#define KILLBOTS_RULESET_H

#include <rulesetbase.h>

namespace Killbots
{
class Ruleset : public RulesetBase
{
public: // static functions
    static const Ruleset *load(const QString &fileName);

public: // functions
    ~Ruleset() override;
    QString filePath() const;
    QString fileName() const;
    QByteArray scoreGroupKey() const;

private: // functions
    explicit Ruleset(const QString &filePath);    // hidden
    QString m_filePath;
    QByteArray m_scoreGroupKey;
};
}

#endif
