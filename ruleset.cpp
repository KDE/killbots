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

		m_name = group.readEntry( "Name", QString("Unnamed") );

		m_rows = group.readEntry( "Rows", 16);
		m_columns = group.readEntry( "Columns", 16);
		m_robotsAtGameStart = group.readEntry( "RobotsAtGameStart", 8);
		m_robotsAddedEachRound = group.readEntry( "RobotsAddedEachRound", 4);
		m_fastbotsAtGameStart = group.readEntry( "FastbotsAtGameStart", -2);
		m_fastbotsAddedEachRound = group.readEntry( "FastbotsAddedEachRound", 2);
		m_energyAtGameStart = group.readEntry( "EnergyAtGameStart", 0);
		m_energyAddedEachRound = group.readEntry( "EnergyAddedEachRound", 0);
		m_maxEnergyAtGameStart = group.readEntry( "MaxEnergyAtGameStart", 12);
		m_maxEnergyAddedEachRound = group.readEntry( "MaxEnergyAddedEachRound", 0);
		m_costOfSafeTeleport = group.readEntry( "CostOfSafeTeleport", 1);
		m_junkheapsArePushable = group.readEntry( "JunkheapsArePushable", true);
		m_junkheapsAtGameStart = group.readEntry( "JunkheapsAtGameStart", 0);
		m_junkheapsAddedEachRound = group.readEntry( "JunkheapsAddedEachRound", 0);
		m_pointsPerRobotKilled = group.readEntry( "PointsPerRobotKilled", 5);
		m_pointsPerFastbotKilled = group.readEntry( "PointsPerFastbotKilled", 10);
		m_waitKillPointBonus = group.readEntry( "WaitKillPointBonus", 0);
		m_waitKillEnergyBonus = group.readEntry( "WaitKillEnergyBonus", 1);
		m_squashKillPointBonus = group.readEntry( "SquashKillPointBonus", 0);
		m_squashKillEnergyBonus = group.readEntry( "SquashKillEnergyBonus", 1);
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


int Killbots::Ruleset::rows() const
{
	return m_rows;
}


int Killbots::Ruleset::columns() const
{
	return m_columns;
}


int Killbots::Ruleset::robotsAtGameStart() const
{
	return m_robotsAtGameStart;
}


int Killbots::Ruleset::robotsAddedEachRound() const
{
	return m_robotsAddedEachRound;
}


int Killbots::Ruleset::fastbotsAtGameStart() const
{
	return m_fastbotsAtGameStart;
}


int Killbots::Ruleset::fastbotsAddedEachRound() const
{
	return m_fastbotsAddedEachRound;
}


int Killbots::Ruleset::energyAtGameStart() const
{
	return m_energyAtGameStart;
}


int Killbots::Ruleset::energyAddedEachRound() const
{
	return m_energyAddedEachRound;
}


int Killbots::Ruleset::maxEnergyAtGameStart() const
{
	return m_maxEnergyAtGameStart;
}


int Killbots::Ruleset::maxEnergyAddedEachRound() const
{
	return m_maxEnergyAddedEachRound;
}


int Killbots::Ruleset::costOfSafeTeleport() const
{
	return m_costOfSafeTeleport;
}


bool Killbots::Ruleset::junkheapsArePushable() const
{
	return m_junkheapsArePushable;
}


int Killbots::Ruleset::junkheapsAtGameStart() const
{
	return m_junkheapsAtGameStart;
}


int Killbots::Ruleset::junkheapsAddedEachRound() const
{
	return m_junkheapsAddedEachRound;
}


int Killbots::Ruleset::pointsPerRobotKilled() const
{
	return m_pointsPerRobotKilled;
}


int Killbots::Ruleset::pointsPerFastbotKilled() const
{
	return m_pointsPerFastbotKilled;
}


int Killbots::Ruleset::waitKillPointBonus() const
{
	return m_waitKillPointBonus;
}


int Killbots::Ruleset::waitKillEnergyBonus() const
{
	return m_waitKillEnergyBonus;
}


int Killbots::Ruleset::squashKillPointBonus() const
{
	return m_squashKillPointBonus;
}


int Killbots::Ruleset::squashKillEnergyBonus() const
{
	return m_squashKillEnergyBonus;
}
