Killbots::Ruleset::~Ruleset()
{
}
qCDebug(KILLBOTS_LOG) << "Failed to load " << fileName;
QString untranslatedName = KConfigGroup(config(), QStringLiteral("KillbotsRuleset")).readEntryUntranslated("Name");
m_scoreGroupKey = untranslatedName.simplified().remove(QLatin1Char(' ')).toLatin1();
/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2007-2009 Parker Coates
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ruleset.h"

#include <KConfigGroup>
#include "killbots_debug.h"

#include <QFileInfo>
#include <memory>  // for std::unique_ptr

namespace Killbots {

std::unique_ptr<Ruleset> Ruleset::load(const QString &fileName)
{
    if (fileName.isEmpty()) {
        qCDebug(KILLBOTS_LOG) << "Failed to load: empty file name.";
        return nullptr;
    }

    // Locate the ruleset path
    const QString filePath = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QLatin1String("killbots/rulesets/") + fileName
    );

    if (filePath.isEmpty()) {
        qCDebug(KILLBOTS_LOG) << "Failed to locate" << fileName;
        return nullptr;
    }

    // Check config validity
    KConfig configFile(filePath, KConfig::SimpleConfig);
    if (!configFile.hasGroup(QStringLiteral("KillbotsRuleset"))) {
        qCDebug(KILLBOTS_LOG) << "Invalid ruleset (no [KillbotsRuleset] group):" << fileName;
        return nullptr;
    }

    // Create and return a new Ruleset instance
    return std::unique_ptr<Ruleset>(new Ruleset(filePath));
}

Ruleset::Ruleset(const QString &filePath)
    : RulesetBase(KSharedConfig::openConfig(filePath))
    , m_filePath(filePath)
{
    // Build the score group key from the "Name" entry
    const QString untranslatedName = KConfigGroup(
        config(), QStringLiteral("KillbotsRuleset")
    ).readEntryUntranslated("Name");

    // Simplify and remove spaces, then convert to Latin-1
    m_scoreGroupKey = untranslatedName
        .simplified()
        .remove(QLatin1Char(' '))
        .toLatin1();
}

// No need for an empty destructor if there's nothing special to do

QString Ruleset::filePath() const
{
    return m_filePath;
}

QString Ruleset::fileName() const
{
    return QFileInfo(m_filePath).fileName();
}

QByteArray Ruleset::scoreGroupKey() const
{
    return m_scoreGroupKey;
}

} // namespace Killbots

