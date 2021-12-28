/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
#include "killbots_version.h"

#include <KLocalizedString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <QApplication>
#include <KAboutData>
#include <KCrash>
#include <QCommandLineParser>
#include <KDBusService>

int main(int argc, char **argv)
{
    // Fixes blurry icons with fractional scaling
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
	QApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("killbots"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("killbotsrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("killbotsui.rc"));
    migrate.migrate();
#endif

    KLocalizedString::setApplicationDomain("killbots");

    KAboutData about(QStringLiteral("killbots"), i18n("Killbots"), QStringLiteral(KILLBOTS_VERSION_STRING));
    about.setShortDescription(i18n("A KDE game of killer robots and teleportation."));
    about.setLicense(KAboutLicense::GPL_V2);
    about.setCopyrightStatement(i18n("© 2006-2009, Parker Coates"));
    about.addAuthor(i18n("Parker Coates"), i18n("Developer"), QStringLiteral("coates@kde.org"));
    about.addCredit(i18n("Mark Rae"), i18n("Author of Gnome Robots. Invented safe teleports, pushing junkheaps and fast robots."), QLatin1String(""), QStringLiteral("https://wiki.gnome.org/Apps/Robots"));
    about.setHomepage(QStringLiteral("https://www.kde.org/applications/games/killbots/"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    KCrash::initialize();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    KDBusService service;

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("killbots")));

    Killbots::MainWindow *mainWindow = new Killbots::MainWindow;
    mainWindow->show();

    return app.exec();
}
