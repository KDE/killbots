/*
 *  Copyright 2006-2009  Parker Coates <coates@kde.org>
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

#include "mainwindow.h"

#include "coordinator.h"
#include "engine.h"
#include "optionspage.h"
#include "renderer.h"
#include "rulesetselector.h"
#include "scene.h"
#include "settings.h"
#include "view.h"

#include <KgThemeProvider>
#include <KgThemeSelector>
#include <highscore/kscoredialog.h>
#include <kstandardgameaction.h>

#include <QAction>
#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <QIcon>

#include <QDebug>
#include <QTimer>

Killbots::MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      m_scoreDialog(nullptr),
      m_rulesetChanged(false)
{
    setAcceptDrops(false);

    m_coordinator = new Coordinator(this);
    m_coordinator->setAnimationSpeed(Settings::animationSpeed());

    m_engine = new Engine(m_coordinator, this);
    m_coordinator->setEngine(m_engine);
    connect(m_engine, &Engine::gameOver, this, &MainWindow::onGameOver);

    m_scene = new Scene(this);
    m_coordinator->setScene(m_scene);
    connect(m_scene, &Scene::clicked, m_coordinator, &Coordinator::requestAction);
    connect(Renderer::self()->themeProvider(), &KgThemeProvider::currentThemeChanged, m_scene, &Scene::doLayout);

    m_view = new View(m_scene, this);
    m_view->setMinimumSize(400, 280);
    m_view->setWhatsThis(i18n("This is the main game area used to interact with Killbots. It shows the current state of the game grid and allows one to control the hero using the mouse. It also displays certain statistics about the game in progress."));
    setCentralWidget(m_view);
    connect(m_view, &View::sizeChanged, m_scene, &Scene::doLayout);

    setupActions();
    connect(m_engine, &Engine::teleportAllowed,       actionCollection()->action(QStringLiteral("teleport")),        &QAction::setEnabled);
    connect(m_engine, &Engine::teleportAllowed,       actionCollection()->action(QStringLiteral("teleport_sip")),    &QAction::setEnabled);
    connect(m_engine, &Engine::teleportSafelyAllowed, actionCollection()->action(QStringLiteral("teleport_safely")), &QAction::setEnabled);
    connect(m_engine, &Engine::vaporizerAllowed,      actionCollection()->action(QStringLiteral("vaporizer")),       &QAction::setEnabled);
    connect(m_engine, &Engine::waitOutRoundAllowed,   actionCollection()->action(QStringLiteral("wait_out_round")),  &QAction::setEnabled);

    setupGUI(Save | Create | ToolBar | Keys);

    // Delaying the start of the first game by 50ms to avoid resize events after
    // the game has been started. Delaying by 0ms doesn't seem to be enough.
    QTimer::singleShot(50, m_coordinator, &Coordinator::requestNewGame);
}

Killbots::MainWindow::~MainWindow()
{
    Killbots::Renderer::cleanup();
}

void Killbots::MainWindow::configurePreferences()
{
    // An instance of the dialog could be already created and could be cached,
    // in which case we want to display the cached dialog instead of creating
    // another one
    if (!KConfigDialog::showDialog(QStringLiteral("configurePreferencesDialog"))) {
        // Creating a dialog because KConfigDialog didn't find an instance of it
        KConfigDialog *configDialog = new KConfigDialog(this, QStringLiteral("configurePreferencesDialog"), Settings::self());

        // Creating setting pages and adding them to the dialog
        configDialog->addPage(new OptionsPage(this),
                              i18n("General"),
                              QStringLiteral("configure"),
                              i18n("Configure general settings")
                             );
        configDialog->addPage(new RulesetSelector(this),
                              i18n("Game Type"),
                              QStringLiteral("games-config-custom"),
                              i18n("Select a game type")
                             );
        configDialog->addPage(new KgThemeSelector(Renderer::self()->themeProvider()),
                              i18n("Appearance"),
                              QStringLiteral("games-config-theme"),
                              i18n("Select a graphical theme")
                             );

        configDialog->setMaximumSize(800, 600);

        // Update the sprite style if it has changed
        connect(configDialog, &KConfigDialog::settingsChanged, this, &MainWindow::onSettingsChanged);

        configDialog->show();
    }
}

void Killbots::MainWindow::onSettingsChanged()
{
    m_coordinator->setAnimationSpeed(Settings::animationSpeed());

    if (m_engine->ruleset()->fileName() != Settings::ruleset()) {
        qDebug() << "Detected a changed in ruleset. From" << m_engine->ruleset()->fileName() << "to" << Settings::ruleset();

        // We don't act on the changed ruleset here because the config dialog
        // is still visible. We want the game in progress to be visible when
        // we display the message box. We also don't want a new game to start
        // while hidden behind the config dialog. So we wait until the config
        // dialog is closed. See onConfigDialogClosed below.
        m_rulesetChanged = true;
    }
    onConfigDialogClosed();
}

void Killbots::MainWindow::onConfigDialogClosed()
{
    if (m_rulesetChanged) {
        if (!m_engine->gameHasStarted()
                || KMessageBox::questionYesNo(this,
                                              i18n("A new game type has been selected, but there is already a game in progress."),
                                              i18n("Game Type Changed"),
                                              KGuiItem(i18n("Continue Current Game")),
                                              KGuiItem(i18n("Start a New Game"))
                                             ) == KMessageBox::No
           ) {
            m_coordinator->requestNewGame();
        }

        m_rulesetChanged = false;
    }
}

void Killbots::MainWindow::createScoreDialog()
{
    m_scoreDialog = new KScoreDialog(KScoreDialog::Name, this);
    m_scoreDialog->addField(KScoreDialog::Level, i18n("Round"), QStringLiteral("round"));
    m_scoreDialog->setModal(false);
    QStringList fileList;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("killbots/ruleset"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString &dir, dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
        Q_FOREACH (const QString &file, fileNames) {
            fileList.append(dir + QLatin1Char('/') + file);
        }
    }
    foreach (const QString &fileName, fileList) {
        const Ruleset *ruleset = Ruleset::load(fileName);
        if (ruleset) {
            m_scoreDialog->addLocalizedConfigGroupName(qMakePair(ruleset->scoreGroupKey(), ruleset->name()));
        }
        delete ruleset;
    }
}

void Killbots::MainWindow::onGameOver(int score, int round)
{
    if (score && m_engine->ruleset()) {
        if (!m_scoreDialog) {
            createScoreDialog();
        }

        m_scoreDialog->setConfigGroup(qMakePair(m_engine->ruleset()->scoreGroupKey(), m_engine->ruleset()->name()));

        KScoreDialog::FieldInfo scoreEntry;
        scoreEntry[ KScoreDialog::Score ].setNum(score);
        scoreEntry[ KScoreDialog::Level ].setNum(round);

        if (m_scoreDialog->addScore(scoreEntry)) {
            QTimer::singleShot(1000, this, &MainWindow::showHighscores);
        }
    }
}

void Killbots::MainWindow::showHighscores()
{
    if (!m_scoreDialog) {
        createScoreDialog();
    }

    // TODO: Find out why this line breaks adding high scores.
    //m_scoreDialog->setConfigGroup(qMakePair(m_engine->ruleset()->scoreGroupKey(), m_engine->ruleset()->name()));
    m_scoreDialog->exec();
}

QAction *Killbots::MainWindow::createMappedAction(int mapping,
        const QString &internalName,
        const QString &displayName,
        const QString &translatedShortcut,
        const int alternateShortcut,
        const QString &helpText,
        const QString &icon
                                                 )
{
    QAction *action = new QAction(displayName, actionCollection());
    action->setObjectName(internalName);
    actionCollection()->setDefaultShortcuts(action, QList<QKeySequence>() << QKeySequence(translatedShortcut) << QKeySequence(alternateShortcut) << QKeySequence(alternateShortcut + Qt::KeypadModifier));
    if (!helpText.isEmpty()) {
        action->setWhatsThis(helpText);
        action->setToolTip(helpText);
    }
    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    connect(action, &QAction::triggered, this, [this, mapping] { m_coordinator->requestAction(mapping); });
    actionCollection()->addAction(internalName, action);

    return action;
}

void Killbots::MainWindow::setupActions()
{
    KStandardGameAction::gameNew(m_coordinator, SLOT(requestNewGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
    KStandardGameAction::quit(qApp, SLOT(quit()), actionCollection());
    KStandardAction::preferences(this, SLOT(configurePreferences()), actionCollection());

    createMappedAction(TeleportSafely,
                       QStringLiteral("teleport_safely"),
                       i18n("Teleport Safely"),
                       i18nc("Shortcut for teleport safely. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "T"),
                       Qt::Key_Plus,
                       i18n("Teleport to a safe location"),
                       QStringLiteral("games-solve")
                      );
    createMappedAction(Teleport,
                       QStringLiteral("teleport"),
                       i18n("Teleport"),
                       i18nc("Shortcut for teleport. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "R"),
                       Qt::Key_Minus,
                       i18n("Teleport to a random location"),
                       QStringLiteral("roll")
                      );
    createMappedAction(TeleportSafelyIfPossible,
                       QStringLiteral("teleport_sip"),
                       i18n("Teleport, Safely If Possible"),
                       i18nc("Shortcut for teleport safely if possible. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Space"),
                       Qt::Key_0,
                       i18n("Teleport safely if that action is enabled, otherwise teleport randomly")
                      );
    createMappedAction(Vaporizer,
                       QStringLiteral("vaporizer"),
                       i18n("Vaporizer"),
                       i18nc("Shortcut for vaporizer. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "F"),
                       Qt::Key_Period,
                       i18n("Destroy all enemies in neighboring cells"),
                       QStringLiteral("edit-bomb")
                      );
    createMappedAction(WaitOutRound,
                       QStringLiteral("wait_out_round"),
                       i18n("Wait Out Round"),
                       i18nc("Shortcut for wait out round. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "V"),
                       Qt::Key_Asterisk,
                       i18n("Risk remaining in place until the end of the round for bonuses"),
                       QStringLiteral("process-stop")
                      );
    createMappedAction(UpLeft,
                       QStringLiteral("move_up_left"),
                       i18n("Move Up and Left"),
                       i18nc("Shortcut for move up and left. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Q"),
                       Qt::Key_7
                      );
    createMappedAction(Up,
                       QStringLiteral("move_up"),
                       i18n("Move Up"),
                       i18nc("Shortcut for move up. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "W"),
                       Qt::Key_8
                      );
    createMappedAction(UpRight,
                       QStringLiteral("move_up_right"),
                       i18n("Move Up and Right"),
                       i18nc("Shortcut for move up and right. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "E"),
                       Qt::Key_9
                      );
    createMappedAction(Left,
                       QStringLiteral("move_left"),
                       i18n("Move Left"),
                       i18nc("Shortcut for move left. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "A"),
                       Qt::Key_4
                      );
    createMappedAction(Hold,
                       QStringLiteral("stand_still"),
                       i18n("Stand Still"),
                       i18nc("Shortcut for stand still. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "S"),
                       Qt::Key_5
                      );
    createMappedAction(Right,
                       QStringLiteral("move_right"),
                       i18n("Move Right"),
                       i18nc("Shortcut for move right. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "D"),
                       Qt::Key_6
                      );
    createMappedAction(DownLeft,
                       QStringLiteral("move_down_left"),
                       i18n("Move Down and Left"),
                       i18nc("Shortcut for move down and left. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Z"),
                       Qt::Key_1
                      );
    createMappedAction(Down,
                       QStringLiteral("move_down"),
                       i18n("Move Down"),
                       i18nc("Shortcut for move down. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "X"),
                       Qt::Key_2
                      );
    createMappedAction(DownRight,
                       QStringLiteral("move_down_right"),
                       i18n("Move Down and Right"),
                       i18nc("Shortcut for move down and right. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "C"),
                       Qt::Key_3
                      );
}

#include "moc_mainwindow.cpp"
