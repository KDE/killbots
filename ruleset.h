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

		int columns() const;
		int rows() const;
		int initialEnemyCount() const;
		int enemiesAddedEachRound() const;
		int initialFastEnemyCount() const;
		int fastEnemiesAddedEachRound() const;
		int initialEnergy() const;
		int maximumEnergy() const;
		int energyPerWaitKill() const;
		int energyPerSquashKill() const;
		int costOfSafeTeleport() const;
		bool includePushableJunkheaps() const;
		int initialJunkheapCount() const;
		int pointsPerRobotKill() const;
		int pointsPerSuperbotKill() const;
		int pointsPerWaitKill() const;
		int pointsPerSquashKill() const;

		bool includeSuperbots() const;
		bool includeEnergy() const;

	  private:
		QString m_filePath;
		bool m_valid;

		QString m_name;
		int m_columns;
		int m_rows;
		int m_initialEnemyCount;
		int m_enemiesAddedEachRound;
		int m_initialFastEnemyCount;
		int m_fastEnemiesAddedEachRound;
		int m_initialEnergy;
		int m_maximumEnergy;
		int m_energyPerWaitKill;
		int m_energyPerSquashKill;
		int m_costOfSafeTeleport;
		bool m_includePushableJunkheaps;
		int m_initialJunkheapCount;
		int m_pointsPerRobotKill;
		int m_pointsPerSuperbotKill;
		int m_pointsPerWaitKill;
		int m_pointsPerSquashKill;
	};
}

#endif
