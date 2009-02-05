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

#ifndef KILLBOTS_RULESETDETAILSDIALOG_H
#define KILLBOTS_RULESETDETAILSDIALOG_H

#include <KDE/KDialog>

#include <QtCore/QMap>
class QLabel;

namespace Killbots {
	class Ruleset;

	class RulesetDetailsDialog : public KDialog
	{
	public:
		RulesetDetailsDialog( QWidget * parent = 0 );
		void loadRuleset( const Ruleset * ruleset );

	private:
		static const QStringList maskedItems;
		static const QStringList junkheapEnumText;

		QMap<QString, QLabel *> m_labels;
	};

}

#endif // KILLBOTS_RULESETDETAILSDIALOG_H
