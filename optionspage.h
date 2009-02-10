/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@gmail.com>
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

#ifndef KILLBOTS_OPTIONSPAGE_H
#define KILLBOTS_OPTIONSPAGE_H

class KComboBox;

class QCheckBox;
class QSlider;
#include <QtGui/QWidget>

namespace Killbots
{
	class OptionsPage : public QWidget
	{
		Q_OBJECT
	
	public: // functions
		explicit OptionsPage( QWidget * parent = 0 );
		virtual ~OptionsPage();
	
	public: // data members
		QCheckBox * kcfg_PreventUnsafeMoves;
		KComboBox * kcfg_MiddleClickAction;
		KComboBox * kcfg_RightClickAction;
		QSlider * kcfg_AnimationSpeed;
	};
}

#endif
