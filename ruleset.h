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

#ifndef KILLBOTS_RULESET_H
#define KILLBOTS_RULESET_H

#include <rulesetbase.h>

namespace Killbots {

	class Ruleset : public RulesetBase
	{
	public: // static functions
		static Ruleset * load( const QString & fileName );
		static Ruleset * loadDefault();

	public: // functions
		virtual ~Ruleset();
		QString filePath() const;
		QString fileName() const;
		QByteArray scoreGroupKey() const;

	private: // functions
		Ruleset( const QString & filePath ); // hidden
		QString m_filePath;
		QByteArray m_scoreGroupKey;
	};

}

#endif
