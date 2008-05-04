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

#include <rulesetbase.h>

namespace Killbots {

	class Ruleset : public RulesetBase
	{
	  public:
		static Ruleset * load( const QString & fileName );
		static Ruleset * loadDefault();

		~Ruleset();

		QString filePath() const;
		QString fileName() const;
		QString untranslatedName() const;

	  private:
		Ruleset( const QString & filePath );
		QString m_filePath;
		QString m_untranslatedName;
	};

}

#endif
