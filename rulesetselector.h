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

#ifndef KILLBOTS_RULESETSELECTOR_H
#define KILLBOTS_RULESETSELECTOR_H

class KLineEdit;

#include <QtCore/QMap>
class QLabel;
class QListWidget;
class QTableWidget;
#include <QtGui/QWidget>

namespace Killbots
{
	class Ruleset;
	class RulesetDetailsDialog;

	class RulesetSelector : public QWidget
	{
		Q_OBJECT

	public: // functions
		explicit RulesetSelector( QWidget * parent = 0 );
		virtual ~RulesetSelector();

	public: // data members
		KLineEdit * kcfg_Ruleset;

	private: // functions
		void findRulesets();

	private slots:
		void selectionChanged( QString rulesetName );
		void showDetailsDialog();

	private: // data members
		QListWidget * m_listWidget;
		QLabel * m_author;
		QLabel * m_authorContact;
		QLabel * m_description;
		QMap< QString, const Ruleset * > m_rulesetMap;
		RulesetDetailsDialog * m_detailsDialog;
	};
}

#endif
