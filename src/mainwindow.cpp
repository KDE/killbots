/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

#include <KGameHighScoreDialog>
#include <KGameStandardAction>
#include <KGameThemeProvider>
#include <KGameThemeSelector>

#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShortcutsDialog>
#include <KStandardAction>

#include <QAction>
#include <QDir>
#include <QIcon>
#include <QTimer>

#include "killbots_debug.h"

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
    connect(Renderer::self()->themeProvider(), &KGameThemeProvider::currentThemeChanged, m_scene, &Scene::doLayout);

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
                              i18nc("@title", "General"),
                              QStringLiteral("configure"),
                              i18nc("@info", "Configure general settings")
                             );
        configDialog->addPage(new RulesetSelector(this),
                              i18nc("@title", "Game Type"),
                              QStringLiteral("games-config-custom"),
                              i18nc("@info", "Select a game type")
                             );
        configDialog->addPage(new KGameThemeSelector(Renderer::self()->themeProvider()),
                              i18nc("@title", "Appearance"),
                              QStringLiteral("games-config-theme"),
                              i18nc("@info", "Select a graphical theme")
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
        qCDebug(KILLBOTS_LOG) << "Detected a changed in ruleset. From" << m_engine->ruleset()->fileName() << "to" << Settings::ruleset();

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
                || KMessageBox::questionTwoActions(this,
                                              i18n("A new game type has been selected, but there is already a game in progress."),
                                              i18nc("@title:window", "Game Type Changed"),
                                              KGuiItem(i18nc("@action:button", "Continue Current Game")),
                                              KGuiItem(i18nc("@action:button", "Start a New Game"))
                                             ) == KMessageBox::SecondaryAction
           ) {
            m_coordinator->requestNewGame();
        }

        m_rulesetChanged = false;
    }
}

void Killbots::MainWindow::createScoreDialog()
{
    m_scoreDialog = new KGameHighScoreDialog(KGameHighScoreDialog::Name, this);
    m_scoreDialog->addField(KGameHighScoreDialog::Level, i18nc("@title:column round of the game", "Round"), QStringLiteral("round"));
    m_scoreDialog->setModal(false);
    QStringList fileList;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("killbots/ruleset"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
        for (const QString &file : fileNames) {
            fileList.append(dir + QLatin1Char('/') + file);
        }
    }
    for (const QString &fileName : std::as_const(fileList)) {
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

        KGameHighScoreDialog::FieldInfo scoreEntry;
        scoreEntry[ KGameHighScoreDialog::Score ].setNum(score);
        scoreEntry[ KGameHighScoreDialog::Level ].setNum(round);

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
    KActionCollection::setDefaultShortcuts(action, {QKeySequence(translatedShortcut), QKeySequence(alternateShortcut), QKeySequence(alternateShortcut | Qt::KeypadModifier)});
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
    KGameStandardAction::gameNew(m_coordinator, &Coordinator::requestNewGame, actionCollection());
    KGameStandardAction::highscores(this, &MainWindow::showHighscores, actionCollection());
    KGameStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
    KStandardAction::preferences(this, &MainWindow::configurePreferences, actionCollection());

    createMappedAction(TeleportSafely,
                       QStringLiteral("teleport_safely"),
                       i18nc("@action", "Teleport Safely"),
                       i18nc("Shortcut for teleport safely. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "T"),
                       Qt::Key_Plus,
                       i18nc("@info:tooltip", "Teleport to a safe location"),
                       QStringLiteral("games-solve")
                      );
    createMappedAction(Teleport,
                       QStringLiteral("teleport"),
                       i18nc("@action", "Teleport"),
                       i18nc("Shortcut for teleport. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "R"),
                       Qt::Key_Minus,
                       i18nc("@info:tooltip", "Teleport to a random location"),
                       QStringLiteral("roll")
                      );
    createMappedAction(TeleportSafelyIfPossible,
                       QStringLiteral("teleport_sip"),
                       i18nc("@action", "Teleport (Safely if Possible)"),
                       i18nc("Shortcut for teleport safely if possible. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Space"),
                       Qt::Key_0,
                       i18nc("@info:tooltip", "Teleport safely if that action is enabled, otherwise teleport randomly")
                      );
    createMappedAction(Vaporizer,
                       QStringLiteral("vaporizer"),
                       i18nc("@action", "Vaporizer"),
                       i18nc("Shortcut for vaporizer. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "F"),
                       Qt::Key_Period,
                       i18nc("@info:tooltip", "Destroy all enemies in neighboring cells"),
                       QStringLiteral("edit-bomb")
                      );
    createMappedAction(WaitOutRound,
                       QStringLiteral("wait_out_round"),
                       i18nc("@action", "Wait Out Round"),
                       i18nc("Shortcut for wait out round. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "V"),
                       Qt::Key_Asterisk,
                       i18nc("@info:tooltip", "Risk remaining in place until the end of the round for bonuses"),
                       QStringLiteral("process-stop")
                      );
    createMappedAction(UpLeft,
                       QStringLiteral("move_up_left"),
                       i18nc("@action", "Move Up and Left"),
                       i18nc("Shortcut for move up and left. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Q"),
                       Qt::Key_7
                      );
    createMappedAction(Up,
                       QStringLiteral("move_up"),
                       i18nc("@action", "Move Up"),
                       i18nc("Shortcut for move up. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "W"),
                       Qt::Key_8
                      );
    createMappedAction(UpRight,
                       QStringLiteral("move_up_right"),
                       i18nc("@action", "Move Up and Right"),
                       i18nc("Shortcut for move up and right. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "E"),
                       Qt::Key_9
                      );
    createMappedAction(Left,
                       QStringLiteral("move_left"),
                       i18nc("@action", "Move Left"),
                       i18nc("Shortcut for move left. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "A"),
                       Qt::Key_4
                      );
    createMappedAction(Hold,
                       QStringLiteral("stand_still"),
                       i18nc("@action", "Stand Still"),
                       i18nc("Shortcut for stand still. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "S"),
                       Qt::Key_5
                      );
    createMappedAction(Right,
                       QStringLiteral("move_right"),
                       i18nc("@action", "Move Right"),
                       i18nc("Shortcut for move right. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "D"),
                       Qt::Key_6
                      );
    createMappedAction(DownLeft,
                       QStringLiteral("move_down_left"),
                       i18nc("@action", "Move Down and Left"),
                       i18nc("Shortcut for move down and left. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "Z"),
                       Qt::Key_1
                      );
    createMappedAction(Down,
                       QStringLiteral("move_down"),
                       i18nc("@action", "Move Down"),
                       i18nc("Shortcut for move down. See https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "X"),
                       Qt::Key_2
                      );
    createMappedAction(DownRight,
                       QStringLiteral("move_down_right"),
                       i18nc("@action", "Move Down and Right"),
                       i18nc("Shortcut for move down and right. https://quickgit.kde.org/?p=killbots.git&a=blob&f=README.translators&o=plain", "C"),
                       Qt::Key_3
                      );
}

#include "moc_mainwindow.cpp"
