/*
 *  Killbots
 *  Copyright (C) 2006-2009  Parker Coates <parker.coates@gmail.com>
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
#include "render.h"
#include "rulesetselector.h"
#include "scene.h"
#include "settings.h"
#include "view.h"

#include <kgamethemeselector.h>
#include <highscore/kscoredialog.h>
#include <kstandardgameaction.h>

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KApplication>
#include <KDE/KConfigDialog>
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KShortcutsDialog>
#include <KDE/KStandardAction>
#include <KDE/KStandardDirs>

#include <QtCore/QSignalMapper>
#include <QtCore/QTimer>

Killbots::MainWindow::MainWindow( QWidget * parent )
  : KXmlGuiWindow( parent ),
    m_scoreDialog( 0 ),
    m_keyboardActions( new KActionCollection( this, KGlobal::mainComponent() ) ),
    m_keyboardMapper( new QSignalMapper( this ) ),
    m_rulesetChanged( false )
{
	setAcceptDrops(false);

	if ( !Render::loadTheme( Settings::theme() ) )
	{
		Render::loadDefaultTheme();
		Settings::setTheme("themes/default.desktop");
	}

	m_scene = new Scene( this );
	m_engine = new Engine( m_scene, this );

	m_view = new View( m_scene, this );
	m_view->setMinimumSize( 400, 280 );
	m_view->setWhatsThis( i18n("This is the main game area used to interact with Killbots. It shows the current state of the game grid and allows one to control the hero using the mouse. It also displays certain statistics about the game in progress.") );
	setCentralWidget( m_view );

	setupActions();

	setupGUI( Save | Create | ToolBar );

	connect( m_view, SIGNAL(sizeChanged(QSize)), m_scene, SLOT(doLayout()) );

	connect( m_scene, SIGNAL(clicked(int)), m_engine, SLOT(requestAction(int)) );

	connect( m_engine, SIGNAL(newGame(int,int,bool)), m_scene, SLOT(onNewGame(int,int,bool)) );
	connect( m_engine, SIGNAL(gameOver(int,int)), this, SLOT(onGameOver(int,int)) );

	connect( m_engine, SIGNAL(roundChanged(int)), m_scene, SLOT(updateRound(int)) );
	connect( m_engine, SIGNAL(scoreChanged(int)), m_scene, SLOT(updateScore(int)) );
	connect( m_engine, SIGNAL(enemyCountChanged(int)), m_scene, SLOT(updateEnemyCount(int)) );
	connect( m_engine, SIGNAL(energyChanged(int)), m_scene, SLOT(updateEnergy(int)) );

	connect( m_engine, SIGNAL(showNewGameMessage()), m_scene, SLOT(showNewGameMessage()) );
	connect( m_engine, SIGNAL(showRoundCompleteMessage()), m_scene, SLOT(showRoundCompleteMessage()) );
	connect( m_engine, SIGNAL(showBoardFullMessage()), m_scene, SLOT(showBoardFullMessage()) );
	connect( m_engine, SIGNAL(showGameOverMessage()), m_scene, SLOT(showGameOverMessage()) );

	connect( m_engine, SIGNAL(teleportAllowed(bool)),         actionCollection()->action("teleport"),        SLOT(setEnabled(bool)) );
	connect( m_engine, SIGNAL(teleportAllowed(bool)),         actionCollection()->action("teleport_sip"),    SLOT(setEnabled(bool)) );
	connect( m_engine, SIGNAL(teleportSafelyAllowed(bool)),   actionCollection()->action("teleport_safely"), SLOT(setEnabled(bool)) );
	connect( m_engine, SIGNAL(sonicScrewdriverAllowed(bool)), actionCollection()->action("screwdriver"),     SLOT(setEnabled(bool)) );
	connect( m_engine, SIGNAL(waitOutRoundAllowed(bool)),     actionCollection()->action("wait_out_round"),  SLOT(setEnabled(bool)) );

	QTimer::singleShot( 25, m_engine, SLOT(requestNewGame()) );
}


Killbots::MainWindow::~MainWindow()
{
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
	if ( !KConfigDialog::showDialog( "configurePreferencesDialog" ) )
	{
		// Creating a dialog because KConfigDialog didn't find an instance of it
		KConfigDialog * configDialog = new KConfigDialog( this, "configurePreferencesDialog", Settings::self() );

		// Creating setting pages and adding them to the dialog
		configDialog->addPage( new OptionsPage( this ),
		                       i18n("General"),
		                       "configure",
		                       i18n("Configure general settings")
		                     );
		configDialog->addPage( new RulesetSelector( this ),
		                       i18n("Game Type"),
		                       "games-config-custom",
		                       i18n("Select a game type")
		                     );
		configDialog->addPage( new KGameThemeSelector( this, Settings::self(), KGameThemeSelector::NewStuffDisableDownload ),
		                       i18n("Theme"),
		                       "games-config-theme",
		                       i18n("Select a graphical theme")
		                     );

		configDialog->setMaximumSize( 800, 600 );
		configDialog->setInitialSize( QSize( 600, 450 ) );

		// Update the sprite style if it has changed
		connect( configDialog, SIGNAL(settingsChanged(QString)), this, SLOT(onSettingsChanged()) );
		connect( configDialog, SIGNAL(finished()), this, SLOT(onConfigDialogClosed()) );

		configDialog->show();
	}
}


void Killbots::MainWindow::onSettingsChanged()
{
	if ( Render::loadTheme( Settings::theme() ) )
	{
		m_view->resetCachedContent();
		m_scene->doLayout();
	}

	m_scene->setAnimationSpeed( Settings::animationSpeed() );

	if ( m_engine->ruleset()->fileName() != Settings::ruleset() )
	{
		kDebug() << "Detected a changed in ruleset. From" << m_engine->ruleset()->fileName() << "to" << Settings::ruleset();

		// We don't act on the changed ruleset here because the config dialog
		// is still visible. We want the game in progress to be visible when
		// we display the message box. We also don't want a new game to start
		// while hidden behind the config dialog. So we wait until the config
		// dialog is closed. See onConfigDialogClosed below.
		m_rulesetChanged = true;
	}
}


void Killbots::MainWindow::onConfigDialogClosed()
{
	if ( m_rulesetChanged )
	{
		if ( !m_engine->gameHasStarted()
		     || KMessageBox::questionYesNo( this,
		                                    i18n("A new ruleset has been selected, but there is already a game in progress."),
		                                    i18n("Ruleset Changed"),
		                                    KGuiItem( i18n("Continue Current Game") ),
		                                    KGuiItem( i18n("Start a New Game") )
		                                  ) == KMessageBox::No
		   )
		{
			m_engine->requestNewGame();
		}

		m_rulesetChanged = false;
	}
}


void Killbots::MainWindow::createScoreDialog()
{
	m_scoreDialog = new KScoreDialog( KScoreDialog::Name, this );
	m_scoreDialog->addField( KScoreDialog::Level, i18n("Round"), "round" );
	m_scoreDialog->setModal( false );

	QStringList fileList;
	KGlobal::dirs()->findAllResources ( "ruleset", "*.desktop", KStandardDirs::NoDuplicates, fileList );
	foreach ( const QString & fileName, fileList )
	{
		Ruleset * ruleset = Ruleset::load( fileName );
		if ( ruleset )
			m_scoreDialog->addLocalizedConfigGroupName( qMakePair( ruleset->scoreGroupKey(), ruleset->name() ) );
		delete ruleset;
	}
}


void Killbots::MainWindow::onGameOver( int score, int round )
{
	if ( score && m_engine->ruleset() )
	{
		if ( !m_scoreDialog )
			createScoreDialog();

		m_scoreDialog->setConfigGroup( qMakePair( m_engine->ruleset()->scoreGroupKey(), m_engine->ruleset()->name() ) );

		KScoreDialog::FieldInfo scoreEntry;
		scoreEntry[ KScoreDialog::Score ].setNum( score );
		scoreEntry[ KScoreDialog::Level ].setNum( round );

		if ( m_scoreDialog->addScore( scoreEntry ) )
			QTimer::singleShot( 1000, this, SLOT(showHighscores()) );
	}
}


void Killbots::MainWindow::showHighscores()
{
	if ( !m_scoreDialog )
		createScoreDialog();

	m_scoreDialog->exec();
}


KAction * Killbots::MainWindow::createMappedAction( KActionCollection * collection,
                                                    int mapping,
                                                    const QString & internalName,
                                                    const QString & displayName,
                                                    const QString & translatedShortcut,
                                                    const QKeySequence & alternateShortcut,
                                                    const QString & toolTip,
                                                    const QString & icon
                                                  )
{
	KAction * action = new KAction( displayName, collection );
	action->setObjectName( internalName );
	action->setShortcut( KShortcut( QKeySequence( translatedShortcut ), alternateShortcut ) );
	if ( !toolTip.isEmpty() )
		action->setToolTip( toolTip );
	if ( !icon.isEmpty() )
		action->setIcon( KIcon( icon ) );

	connect( action, SIGNAL(triggered()), m_keyboardMapper, SLOT(map()) );
	m_keyboardMapper->setMapping( action, mapping );
	collection->addAction( internalName, action );

	return action;
}


void Killbots::MainWindow::setupActions()
{
	KStandardGameAction::gameNew( m_engine, SLOT(requestNewGame()), actionCollection() );
	KStandardGameAction::highscores( this, SLOT(showHighscores()), actionCollection() );
	KStandardGameAction::quit( kapp, SLOT(quit()), actionCollection() );

	KStandardAction::keyBindings( this, SLOT(configureShortcuts()), actionCollection() );
	KStandardAction::preferences( this, SLOT(configurePreferences()), actionCollection() );

	createMappedAction( actionCollection(),
	                    TeleportSafely,
	                    "teleport_safely",
	                    i18n("Teleport Safely"),
	                    i18nc("Shortcut for teleport safely. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "T"),
	                    Qt::Key_Plus,
	                    i18n("Teleport to a safe location"),
	                    "games-solve"
	                  );
	createMappedAction( actionCollection(),
	                    Teleport,
	                    "teleport",
	                    i18n("Teleport"),
	                    i18nc("Shortcut for teleport. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "R"),
	                    Qt::Key_Minus,
	                    i18n("Teleport to a random location"),
	                    "roll"
	                  );
	createMappedAction( actionCollection(),
	                    TeleportSafelyIfPossible,
	                    "teleport_sip",
	                    i18n("Teleport, Safely If Possible"),
	                    i18nc("Shortcut for teleport safely if possible. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "Space"),
	                    Qt::Key_0,
	                    i18n("Teleport safely if that action is enabled, otherwise teleport randomly")
	                  );
	createMappedAction( actionCollection(),
	                    SonicScrewdriver,
	                    "screwdriver",
	                    i18n("Sonic Screwdriver"),
	                    i18nc("Shortcut for sonicscrewdriver. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "F"),
	                    Qt::Key_Period,
	                    i18n("Destroy all enemies in neighboring cells"),
	                    "edit-bomb"
	                  );
	createMappedAction( actionCollection(),
	                    WaitOutRound,
	                    "wait_out_round",
	                    i18n("Wait Out Round"),
	                    i18nc("Shortcut for wait out round. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "V"),
	                    Qt::Key_Asterisk,
	                    i18n("Risk remaining in place until the end of the round for bonuses"),
	                    "process-stop"
	                  );


	// Keyboard Actions - these are shown in Configure Shortcuts but not in Configure Toolbars
	createMappedAction( m_keyboardActions,
	                    UpLeft,
	                    "move_up_left",
	                    i18n("Move Up and Left"),
	                    i18nc("Shortcut for move up and left. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "Q"),
	                    Qt::Key_7
	                  );
	createMappedAction( m_keyboardActions,
	                    Up,
	                    "move_up",
	                    i18n("Move Up"),
	                    i18nc("Shortcut for move up. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "W"),
	                    Qt::Key_8
	                  );
	createMappedAction( m_keyboardActions,
	                    UpRight,
	                    "move_up_right",
	                    i18n("Move Up and Right"),
	                    i18nc("Shortcut for move up and right. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "E"),
	                    Qt::Key_9
	                  );
	createMappedAction( m_keyboardActions,
	                    Left,
	                    "move_left",
	                    i18n("Move Left"),
	                    i18nc("Shortcut for move left. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "A"),
	                    Qt::Key_4
	                  );
	createMappedAction( m_keyboardActions,
	                    Hold,
	                    "stand_still",
	                    i18n("Stand Still"),
	                    i18nc("Shortcut for stand still. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "S"),
	                    Qt::Key_5
	                  );
	createMappedAction( m_keyboardActions,
	                    Right,
	                    "move_right",
	                    i18n("Move Right"),
	                    i18nc("Shortcut for move right. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "D"),
	                    Qt::Key_6
	                  );
	createMappedAction( m_keyboardActions,
	                    DownLeft,
	                    "move_down_left",
	                    i18n("Move Down and Left"),
	                    i18nc("Shortcut for move down and left. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "Z"),
	                    Qt::Key_1
	                    );
	createMappedAction( m_keyboardActions,
	                    Down,
	                    "move_down",
	                    i18n("Move Down"),
	                    i18nc("Shortcut for move down. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "X"),
	                    Qt::Key_2
	                  );
	createMappedAction( m_keyboardActions,
	                    DownRight,
	                    "move_down_right",
	                    i18n("Move Down and Right"),
	                    i18nc("Shortcut for move down and right. See http://websvn.kde.org/trunk/KDE/kdegames/killbots/README.translators?view=markup", "C"),
	                    Qt::Key_3
	                  );

	connect( m_keyboardMapper, SIGNAL(mapped(int)), m_engine, SLOT(requestAction(int)) );

	m_keyboardActions->associateWidget(this);
}

#include "moc_mainwindow.cpp"
