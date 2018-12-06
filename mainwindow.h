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

#ifndef KILLBOTS_MAINWINDOW_H
#define KILLBOTS_MAINWINDOW_H

class QAction;
class KActionCollection;
class KScoreDialog;
#include <KXmlGuiWindow>

class QSignalMapper;

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
    virtual ~MainWindow();

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
    KScoreDialog *m_scoreDialog;
    QSignalMapper *m_keyboardMapper;
    bool m_rulesetChanged;
};

}

#endif

