/*
    This file is part of Killbots.

    SPDX-FileCopyrightText: 2006-2009 Parker Coates <coates@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KILLBOTS_ACTIONS_H
#define KILLBOTS_ACTIONS_H

namespace Killbots
{
enum HeroAction {
    Right = 0,
    UpRight,
    Up,
    UpLeft,
    Left,
    DownLeft,
    Down,
    DownRight,
    Hold,

    Teleport,
    TeleportSafely,
    TeleportSafelyIfPossible,
    WaitOutRound,
    Vaporizer,
    NoAction,

    RepeatRight = -(Right + 1),
    RepeatUpRight = -(UpRight + 1),
    RepeatUp = -(Up + 1),
    RepeatUpLeft = -(UpLeft + 1),
    RepeatLeft = -(Left + 1),
    RepeatDownLeft = -(DownLeft + 1),
    RepeatDown = -(Down + 1),
    RepeatDownRight = -(DownRight + 1),
    RepeatHold = -(Hold + 1)
};
}

#endif
