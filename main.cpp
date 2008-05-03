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

#include "mainwindow.h"

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>


int main( int argc, char ** argv )
{
	KAboutData about( "killbots", "", ki18n("Killbots"), "0.1.0" );
	about.setShortDescription( ki18n("The BSD classic \"robots\" ported to KDE.") );
	about.setLicense( KAboutData::License_GPL_V2 );
	about.setCopyrightStatement( ki18n("© 2007-2008, Parker Coates") );
	about.addAuthor( ki18n("Parker Coates"), ki18n("Developer"), "parker.coates@gmail.com" );
	about.addCredit( ki18n("Mark Rae"), ki18n("Author of Gnome Robots. Invented safe teleports, pushing junkheaps and fast robots."), "", "http://live.gnome.org/Robots" );

	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;

	KGlobal::locale()->insertCatalog("libkdegames");

	KGlobal::dirs()->addResourceType("ruleset", "data", about.appName() + "/rulesets/");

	Killbots::MainWindow * mainWindow = new Killbots::MainWindow;
	mainWindow->show();

	return app.exec();
}
