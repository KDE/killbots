/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_MAINWINDOW_H
#define KILLBOTS_MAINWINDOW_H

class QAction;
class KGameHighScoreDialog;
#include <KXmlGuiWindow>

namespace Killbots
{
class Engine;
class Coordinator;
class Scene;
class View;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public: // functions
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private: // functions
    void setupActions();
    QAction *createMappedAction(int mapping, const QString &internalName, const QString &displayName, const QString &translatedShortcut, const int alternateShortcut, const QString &toolTip = QString(), const QString &icon = QString());
    void createScoreDialog();

private Q_SLOTS:
    void showHighscores();
    void configurePreferences();
    void onGameOver(int score, int round);
    void onSettingsChanged();
    void onConfigDialogClosed();

private : // data members
    Scene *m_scene;
    Engine *m_engine;
    View *m_view;
    Coordinator *m_coordinator;
    KGameHighScoreDialog *m_scoreDialog;
    bool m_rulesetChanged;
};

}

#endif

