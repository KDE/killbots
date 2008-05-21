/*
 *  Killbots
 *  Copyright (C) 2006-2008  Parker Coates <parker.coates@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses>
 *  or write to the Free Software Foundation, Inc., 51 Franklin Street,
 *  Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ruleset.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

#include <QtCore/QFileInfo>

Killbots::Ruleset * Killbots::Ruleset::load( const QString & fileName )
{
	Ruleset * result = 0;

	kDebug() << "Attempting to load ruleset at" << fileName;

	if ( ! fileName.isEmpty() )
	{
		QString filePath = KStandardDirs::locate( "ruleset", fileName );
		if ( ! filePath.isEmpty() )
		{
			KConfig configFile( filePath, KConfig::SimpleConfig );
			if ( configFile.hasGroup("KillbotsRuleset") )
				result = new Ruleset( filePath );
		}
	}

	return result;
}


Killbots::Ruleset * Killbots::Ruleset::loadDefault()
{
	return load("default.desktop");
}


Killbots::Ruleset::Ruleset( const QString & filePath )
 : RulesetBase( filePath )
{
	m_filePath = filePath;
	m_untranslatedName = KConfigGroup( config(), "KillbotsRuleset").readEntryUntranslated("Name").toUtf8();
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


QByteArray Killbots::Ruleset::untranslatedName() const
{
	return m_untranslatedName;
}
