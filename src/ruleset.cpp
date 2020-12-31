/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ruleset.h"

#include <KConfigGroup>
#include "killbots_debug.h"

#include <QFileInfo>

const Killbots::Ruleset *Killbots::Ruleset::load(const QString &fileName)
{
    const Ruleset *result = nullptr;
    if (!fileName.isEmpty()) {
        QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("killbots/rulesets/") + fileName);
        if (!filePath.isEmpty()) {
            // Our only check for validity is that we can open the file as a config
            // file and that it contains a group named "KillbotsRuleset".
            KConfig configFile(filePath, KConfig::SimpleConfig);
            if (configFile.hasGroup("KillbotsRuleset")) {
                result = new Ruleset(filePath);
            }
        }
    }
    if (!result) {
        qCDebug(KILLBOTS_LOG) << "Failed to load " << fileName;
    }

    return result;
}

Killbots::Ruleset::Ruleset(const QString &filePath)
    : RulesetBase(filePath)
{
    m_filePath = filePath;
    QString untranslatedName = KConfigGroup(config(), "KillbotsRuleset").readEntryUntranslated("Name");
    m_scoreGroupKey = untranslatedName.simplified().remove(QLatin1Char(' ')).toLatin1();
}

Killbots::Ruleset::~Ruleset()
{
}

QString Killbots::Ruleset::filePath() const
{
    return m_filePath;
}

QString Killbots::Ruleset::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

QByteArray Killbots::Ruleset::scoreGroupKey() const
{
    return m_scoreGroupKey;
}
