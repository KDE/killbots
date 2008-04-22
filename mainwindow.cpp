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

#include "engine.h"
#include "optionspage.h"
#include "renderer.h"
#include "rulesetselector.h"
#include "scene.h"
#include "settings.h"
#include "view.h"

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KApplication>
#include <KDE/KConfigDialog>
#include <KDE/KGameThemeSelector>
#include <KDE/KLocalizedString>
#include <KDE/KScoreDialog>
#include <KDE/KShortcutsDialog>
#include <KDE/KStandardAction>
#include <KDE/KStandardGameAction>

#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>

Killbots::MainWindow::MainWindow( QWidget * parent )
  : KXmlGuiWindow( parent ),
	m_keyboardActions( new KActionCollection( this, KGlobal::mainComponent() ) ),
	m_keyboardMapper( new QSignalMapper( this ) )
{
	setAcceptDrops(false);

	if ( !Renderer::loadTheme( Settings::theme() ) )
		Renderer::loadDefaultTheme();

	m_scene = new Scene( this );
	m_engine = new Engine( m_scene, this );

	m_view = new View( m_scene, this );
	m_view->setMinimumSize( 400, 280 );
	setCentralWidget( m_view );

	setupActions();

	setupGUI( Save | Create | ToolBar );

	connect( m_view, SIGNAL(sizeChanged(QSize)), m_scene, SLOT(doLayout()) );

	connect( m_scene, SIGNAL(clicked(int)), m_engine, SLOT(doAction(int)) );
	connect( m_scene, SIGNAL(animationDone()), m_engine, SLOT(goToNextPhase()), Qt::QueuedConnection );

	connect( m_engine, SIGNAL(newGame(const Ruleset*)), m_scene, SLOT(onNewGame(const Ruleset*)) );
	connect( m_engine, SIGNAL(roundComplete()), m_scene, SLOT(onRoundComplete()) );
	connect( m_engine, SIGNAL(gameOver(QString,int,int)), m_scene, SLOT(onGameOver()) );
	connect( m_engine, SIGNAL(gameOver(QString,int,int)), this, SLOT(onGameOver(QString,int,int)) );
	connect( m_engine, SIGNAL(boardFull()), m_scene, SLOT(onBoardFull()) );

	connect( m_engine, SIGNAL(roundChanged(int)), m_scene, SLOT(updateRound(int)) );
	connect( m_engine, SIGNAL(scoreChanged(int)), m_scene, SLOT(updateScore(int)) );
	connect( m_engine, SIGNAL(enemyCountChanged(int)), m_scene, SLOT(updateEnemyCount(int)) );
	connect( m_engine, SIGNAL(energyChanged(int)), m_scene, SLOT(updateEnergy(int)) );
	connect( m_engine, SIGNAL(canAffordSafeTeleport(bool)), m_safeTeleportAction, SLOT(setEnabled(bool)) );

	m_engine->newGame();
}


Killbots::MainWindow::~MainWindow()
{
}


void Killbots::MainWindow::setupActions()
{
	KStandardGameAction::gameNew( m_engine, SLOT(newGame()), actionCollection() );
	KStandardGameAction::highscores( this, SLOT(showHighscores()), actionCollection() );
	KStandardGameAction::quit( kapp, SLOT(quit()), actionCollection() );

	KStandardAction::keyBindings( this, SLOT(configureShortcuts()), actionCollection() );
	KStandardAction::preferences( this, SLOT(configurePreferences()), actionCollection() );

	setupMappedAction( actionCollection(), "Teleport",            "teleport",      Qt::Key_R, Qt::Key_Minus, Teleport,       "roll" );
	setupMappedAction( actionCollection(), "Teleport Safely",     "safe_teleport", Qt::Key_T, Qt::Key_Plus,  TeleportSafely, "love" );
	setupMappedAction( actionCollection(), "Wait Out Round",      "wait",          Qt::Key_Y, Qt::Key_Enter, WaitOutRound,    "no"  );

	// Keyboard Actions - these are shown in Configure Shortcuts but not in Configure Toolbars
	setupMappedAction( m_keyboardActions,  "Move Up and Left",    "move_up_left",     Qt::Key_Q, Qt::Key_7,     UpLeft                 );
	setupMappedAction( m_keyboardActions,  "Move Up",             "move_up",          Qt::Key_W, Qt::Key_8,     Up                     );
	setupMappedAction( m_keyboardActions,  "Move Up and Right",   "move_up_right",    Qt::Key_E, Qt::Key_9,     UpRight                );
	setupMappedAction( m_keyboardActions,  "Move Left",           "move_left",        Qt::Key_A, Qt::Key_4,     Left                   );
	setupMappedAction( m_keyboardActions,  "Stand Still",         "stand_still", Qt::Key_S, Qt::Key_5,     Hold                   );
	setupMappedAction( m_keyboardActions,  "Move Right",          "move_right",       Qt::Key_D, Qt::Key_6,     Right                  );
	setupMappedAction( m_keyboardActions,  "Move Down and Left",  "move_down_left",   Qt::Key_Z, Qt::Key_1,     DownLeft               );
	setupMappedAction( m_keyboardActions,  "Move Down",           "move_down",        Qt::Key_X, Qt::Key_2,     Down                   );
	setupMappedAction( m_keyboardActions,  "Move Down and Right", "move_down_right",  Qt::Key_C, Qt::Key_3,     DownRight              );

	connect( m_keyboardMapper, SIGNAL(mapped(int)), m_engine, SLOT(doAction(int)) );

	m_safeTeleportAction = actionCollection()->action("safe_teleport");

	m_keyboardActions->associateWidget(this);
}


void Killbots::MainWindow::setupMappedAction( KActionCollection * collection, const char * displayName, const QString & internalName, const QKeySequence & primaryShortcut, const QKeySequence & alternateShortcut, int mapping, const QString & icon )
{
	KAction * action = new KAction( i18n( displayName ), collection );
	action->setObjectName( internalName );
	action->setShortcut( KShortcut( primaryShortcut, alternateShortcut ) );
	if ( !icon.isEmpty() )
		action->setIcon( KIcon( icon ) );

	connect( action, SIGNAL(triggered()), m_keyboardMapper, SLOT(map()) );
	m_keyboardMapper->setMapping( action, mapping );
	collection->addAction( internalName, action );
}


void Killbots::MainWindow::configureShortcuts()
{
	KShortcutsDialog dialog( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this );
	dialog.addCollection( actionCollection(), i18n("General") );
	dialog.addCollection( m_keyboardActions, i18n("Movement Controls") );
	dialog.configure();
}


void Killbots::MainWindow::configurePreferences()
{
	// An instance of the dialog could be already created and could be cached,
	// in which case we want to display the cached dialog instead of creating
	// another one
	if ( ! KConfigDialog::showDialog( "configurePreferencesDialog" ) )
	{
		// Creating a dialog because KConfigDialog didn't find an instance of it
		m_configDialog = new KConfigDialog( this, "configurePreferencesDialog", Settings::self() );

		// Creating setting pages and adding them to the dialog
		m_configDialog->addPage( new OptionsPage( this ), i18n("General"), "configure" );
		m_configDialog->addPage( new RulesetSelector( this ), i18n("Rules"), "games-config-custom" );
		m_configDialog->addPage( new KGameThemeSelector( this, Settings::self(), KGameThemeSelector::NewStuffDisableDownload ), i18n("Theme"), "games-config-theme" );

		m_configDialog->setMaximumSize( 800, 600 );
		m_configDialog->setInitialSize( QSize( 600, 450 ) );

		// Update the sprite style if it has changed
		connect( m_configDialog, SIGNAL(settingsChanged(QString)), this, SLOT(onSettingsChanged()) );

		m_configDialog->show();
	}
}


void Killbots::MainWindow::onSettingsChanged()
{
	if ( Renderer::loadTheme( Settings::theme() ) )
	{
		m_view->resetCachedContent();
		m_scene->doLayout();
	}

	m_scene->setAnimationSpeed( Settings::animationSpeed() );
}


void Killbots::MainWindow::onGameOver( QString rulesetName, int score, int round )
{
	KScoreDialog scoreDialog( KScoreDialog::Name | KScoreDialog::Level, this );
	scoreDialog.setConfigGroup( rulesetName );

	KScoreDialog::FieldInfo scoreEntry;
	scoreEntry[ KScoreDialog::Score ].setNum( score );
	scoreEntry[ KScoreDialog::Level ].setNum( round );

	if ( scoreDialog.addScore( scoreEntry ) )
		scoreDialog.exec();
}


void Killbots::MainWindow::showHighscores()
{
	KScoreDialog scoreDialog( KScoreDialog::Name | KScoreDialog::Level, this );
	scoreDialog.exec();
}

#include "mainwindow.moc"
