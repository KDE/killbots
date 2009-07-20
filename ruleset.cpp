/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@gmail.com>
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

#include "ruleset.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

#include <QtCore/QFileInfo>

const Killbots::Ruleset * Killbots::Ruleset::load( const QString & fileName )
{
	const Ruleset * result = 0;

	if ( !fileName.isEmpty() )
	{
		QString filePath = KStandardDirs::locate( "ruleset", fileName );
		if ( !filePath.isEmpty() )
		{
			// Our only check for validity is that we can open the file as a config
			// file and that it contains a group named "KillbotsRuleset".
			KConfig configFile( filePath, KConfig::SimpleConfig );
			if ( configFile.hasGroup("KillbotsRuleset") )
				result = new Ruleset( filePath );
		}
	}

	if ( !result )
		kDebug() << "Failed to load " << fileName;

	return result;
}


Killbots::Ruleset::Ruleset( const QString & filePath )
  : RulesetBase( filePath )
{
	m_filePath = filePath;
	QString untranslatedName = KConfigGroup( config(), "KillbotsRuleset").readEntryUntranslated("Name");
	m_scoreGroupKey = untranslatedName.simplified().remove(' ').toLatin1();
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
	return QFileInfo( m_filePath ).fileName();
}


QByteArray Killbots::Ruleset::scoreGroupKey() const
{
	return m_scoreGroupKey;
}
