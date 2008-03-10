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

#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KStandardDirs>

#include <QtCore/QFileInfo>


Killbots::Ruleset::Ruleset( const QString & filePath )
  : m_filePath( filePath ),
	m_valid( false )
{
	if ( ! m_filePath.isEmpty() )
		load( m_filePath );
}


Killbots::Ruleset::~Ruleset()
{
}


bool Killbots::Ruleset::load( const QString & fileName )
{
	kDebug() << "Attempting to load ruleset at" << fileName;

	QString filePath = KStandardDirs::locate("ruleset", fileName);

	static const QString groupName("KillbotsRuleset");

	KConfig configFile( filePath, KConfig::SimpleConfig );

	if ( configFile.hasGroup( groupName ) )
	{
		m_filePath = filePath;
		m_valid = true;

		KConfigGroup group = configFile.group( groupName );

		m_name = group.readEntry( "name", QString("Unnamed") );

		m_columns = group.readEntry( "columns", 25 );
		m_rows = group.readEntry( "rows", 25 );

		m_initialEnemyCount = group.readEntry( "initialEnemyCount", 8 );
		m_enemiesAddedEachRound = group.readEntry( "enemiesAddedEachRound", 4 );
		m_initialFastEnemyCount = group.readEntry( "initialFastEnemyCount", 0 );
		m_fastEnemiesAddedEachRound = group.readEntry( "fastEnemiesAddedEachRound", 0 );

		m_initialEnergy = group.readEntry( "initialEnergy", 0 );
		m_maximumEnergy = group.readEntry( "maximumEnergy", 0 );
		m_energyPerWaitKill = group.readEntry( "energyPerWaitKill", 0 );
		m_energyPerSquashKill = group.readEntry( "energyPerSquashKill", 0 );

		m_costOfSafeTeleport = group.readEntry( "costOfSafeTeleport", 1 );
		m_includePushableJunkheaps = group.readEntry( "includePushableJunkheaps", false );
		m_initialJunkheapCount = group.readEntry( "initialJunkheapCount", 0 );
		m_pointsPerRobotKill = group.readEntry( "pointsPerRobotKill", 5 );
		m_pointsPerSuperbotKill = group.readEntry( "pointsPerSuperbotKill", 0 );
		m_pointsPerWaitKill = group.readEntry( "pointsPerWaitKill", 1 );
		m_pointsPerSquashKill = group.readEntry( "pointsPerSquashKill", 0 );
	}

	return isValid();
}


bool Killbots::Ruleset::loadDefault()
{
	return load( "default.desktop" );
}


bool Killbots::Ruleset::isValid() const
{
	return m_valid;
}


QString Killbots::Ruleset::filePath() const
{
	return m_filePath;
}


QString Killbots::Ruleset::fileName() const
{
	return QFileInfo( m_filePath ).fileName();
}


QString Killbots::Ruleset::name() const
{
	return m_name;
}


int Killbots::Ruleset::columns() const
{
	return m_columns;
}


int Killbots::Ruleset::rows() const
{
	return m_rows;
}


int Killbots::Ruleset::initialEnemyCount() const
{
	return m_initialEnemyCount;
}


int Killbots::Ruleset::enemiesAddedEachRound() const
{
	return m_enemiesAddedEachRound;
}


int Killbots::Ruleset::initialFastEnemyCount() const
{
	return m_initialFastEnemyCount;
}


int Killbots::Ruleset::fastEnemiesAddedEachRound() const
{
	return m_fastEnemiesAddedEachRound;
}


int Killbots::Ruleset::initialEnergy() const
{
	return m_initialEnergy;
}


int Killbots::Ruleset::maximumEnergy() const
{
	return m_maximumEnergy;
}


int Killbots::Ruleset::energyPerWaitKill() const
{
	return m_energyPerWaitKill;
}


int Killbots::Ruleset::energyPerSquashKill() const
{
	return m_energyPerSquashKill;
}


int Killbots::Ruleset::costOfSafeTeleport() const
{
	return m_costOfSafeTeleport;
}


bool Killbots::Ruleset::includePushableJunkheaps() const
{
	return m_includePushableJunkheaps;
}


int Killbots::Ruleset::initialJunkheapCount() const
{
	return m_initialJunkheapCount;
}


int Killbots::Ruleset::pointsPerRobotKill() const
{
	return m_pointsPerRobotKill;
}


int Killbots::Ruleset::pointsPerSuperbotKill() const
{
	return m_pointsPerSuperbotKill;
}


int Killbots::Ruleset::pointsPerWaitKill() const
{
	return m_pointsPerWaitKill;
}


int Killbots::Ruleset::pointsPerSquashKill() const
{
	return m_pointsPerSquashKill;
}


bool Killbots::Ruleset::includeSuperbots() const
{
	return m_initialFastEnemyCount != 0 || m_fastEnemiesAddedEachRound != 0;
}


bool Killbots::Ruleset::includeEnergy() const
{
	return m_maximumEnergy != 0;
}
