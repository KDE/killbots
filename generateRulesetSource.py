#! /usr/bin/python

import sys

orderedKeys= []
typeDict = {}
valueDict = {}

desktopFile = open(sys.argv[1])

for line in desktopFile :
	if line.isspace() or line.startswith('[') or line.startswith("Name"):
		continue

	(key, sep, value) = line.rstrip('\n').partition('=')

	orderedKeys.append(key)

	if value == "true" or value == "false" :
		typeDict[key] = "bool"
	else :
		typeDict[key] = "int"

	valueDict[key] = value




header = '''/*
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
'''

for key in orderedKeys :
	header += "\t\t" + typeDict[key] + ' ' + key[0].lower() + key[1:] + "() const;\n"

header += '''
	  private:
		QString m_filePath;
		bool m_valid;

		QString m_name;
'''

for key in orderedKeys :
	header += "\t\t" + typeDict[key] + ' m_' + key[0].lower() + key[1:] + ";\n"

header += '''	};
}

#endif
'''

print "Header"
print "-" * 78
print header
print "-" * 78 + "\n\n"

try :
	headerFile = open("ruleset.h", "w" )
	headerFile.write(header)
except :
	print "\nERROR WRITING TO HEADER FILE"


source = '''/*
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

'''

for key in orderedKeys :
	source += "\t\tm_" + key[0].lower() + key[1:] + " = group.readEntry( \"" + key + "\", " + valueDict[key] + ");\n"

source += '''	}

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
'''

for key in orderedKeys :
	source += "\n\n" +typeDict[key] + ' Killbots::Ruleset::' + key[0].lower() + key[1:] + "() const\n"
	source += "{\n"
	source += "\treturn m_" + key[0].lower() + key[1:] + ";\n"
	source += "}\n"

print "Source"
print "-" * 78
print source
print "-" * 78 + "\n\n"

try :
	sourceFile = open("ruleset.cpp", "w" )
	sourceFile.write(source)
except :
	print "\nERROR WRITING TO SOURCE FILE"
