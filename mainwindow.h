/*
 *  Copyright 2006-2009  Parker Coates <parker.coates@gmail.com>
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

#ifndef KILLBOTS_MAINWINDOW_H
#define KILLBOTS_MAINWINDOW_H

class KAction;
class KActionCollection;
class KScoreDialog;
#include <KDE/KXmlGuiWindow>

class QSignalMapper;

namespace Killbots
{
	class Engine;
	class Scene;
	class View;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT

	public: // functions
		explicit MainWindow( QWidget * parent = 0 );
		virtual ~MainWindow();

	private: // functions
		void setupActions();
		KAction * setupMappedAction( KActionCollection * collection, const QString & displayName, const QString & internalName, const QKeySequence & primaryShortcut, const QKeySequence & alternateShortcut, int mapping, const QString & icon );
		KAction * createMappedAction( KActionCollection * collection, int mapping, const QString & internalName, const QString & displayName, const QString & translatedShortcut, const QKeySequence & alternateShortcut, const QString & toolTip = QString(), const QString & icon = QString() );
		void createScoreDialog();

	private slots:
		void showHighscores();
		void configureShortcuts();
		void configurePreferences();
		void onGameOver( int score, int round );
		void onSettingsChanged();
		void onConfigDialogClosed();

	private : // data members
		Scene * m_scene;
		Engine * m_engine;
		View * m_view;
		KScoreDialog * m_scoreDialog;

		KActionCollection * m_keyboardActions;
		QSignalMapper * m_keyboardMapper;

		bool m_rulesetChanged;
	};

}

#endif

