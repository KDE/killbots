/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
#include "killbots_version.h"

#include <KLocalizedString>
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif
#include <QCommandLineParser>
#include <QApplication>

int main(int argc, char **argv)
{
#if HAVE_KICONTHEME
    KIconTheme::initTheme();
#endif
	QApplication app(argc, argv);
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("killbots"));

    KAboutData about(QStringLiteral("killbots"), i18n("Killbots"), QStringLiteral(KILLBOTS_VERSION_STRING));
    about.setShortDescription(i18n("A KDE game of killer robots and teleportation."));
    about.setLicense(KAboutLicense::GPL_V2);
    about.setCopyrightStatement(i18n("© 2006-2009, Parker Coates"));
    about.addAuthor(i18n("Parker Coates"), i18n("Developer"), QStringLiteral("coates@kde.org"));
    about.addCredit(i18n("Mark Rae"), i18n("Author of Gnome Robots. Invented safe teleports, pushing junkheaps and fast robots."), QString(), QStringLiteral("https://wiki.gnome.org/Apps/Robots"));
    about.setHomepage(QStringLiteral("https://apps.kde.org/killbots"));

    KAboutData::setApplicationData(about);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("killbots")));

    KCrash::initialize();

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    KDBusService service;

    Killbots::MainWindow *mainWindow = new Killbots::MainWindow;
    mainWindow->show();

    return app.exec();
}
