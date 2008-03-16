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

#ifndef KILLBOTS_RULESET_H
#define KILLBOTS_RULESET_H

#include <QtCore/QString>


namespace Killbots
{
	class Ruleset
	{
	  public:
		explicit Ruleset( const QString & filePath = QString() );
		~Ruleset();

		bool load( const QString & filename );
		bool loadDefault();

		bool isValid() const;
		QString filePath() const;
		QString fileName() const;

		QString name() const;
		int rows() const;
		int columns() const;
		int robotsAtGameStart() const;
		int robotsAddedEachRound() const;
		int fastbotsAtGameStart() const;
		int fastbotsAddedEachRound() const;
		int energyAtGameStart() const;
		int energyAddedEachRound() const;
		int maxEnergyAtGameStart() const;
		int maxEnergyAddedEachRound() const;
		int costOfSafeTeleport() const;
		bool junkheapsArePushable() const;
		int junkheapsAtGameStart() const;
		int junkheapsAddedEachRound() const;
		int pointsPerRobotKilled() const;
		int pointsPerFastbotKilled() const;
		int waitKillPointBonus() const;
		int waitKillEnergyBonus() const;
		int squashKillPointBonus() const;
		int squashKillEnergyBonus() const;

	  private:
		QString m_filePath;
		bool m_valid;

		QString m_name;
		int m_rows;
		int m_columns;
		int m_robotsAtGameStart;
		int m_robotsAddedEachRound;
		int m_fastbotsAtGameStart;
		int m_fastbotsAddedEachRound;
		int m_energyAtGameStart;
		int m_energyAddedEachRound;
		int m_maxEnergyAtGameStart;
		int m_maxEnergyAddedEachRound;
		int m_costOfSafeTeleport;
		bool m_junkheapsArePushable;
		int m_junkheapsAtGameStart;
		int m_junkheapsAddedEachRound;
		int m_pointsPerRobotKilled;
		int m_pointsPerFastbotKilled;
		int m_waitKillPointBonus;
		int m_waitKillEnergyBonus;
		int m_squashKillPointBonus;
		int m_squashKillEnergyBonus;
	};
}

#endif
