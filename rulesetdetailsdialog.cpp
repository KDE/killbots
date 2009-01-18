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

#include "rulesetdetailsdialog.h"

#include "ruleset.h"

#include <KDE/KLocale>

#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QScrollArea>

Killbots::RulesetDetailsDialog::RulesetDetailsDialog( const Ruleset * ruleset, QWidget * parent )
  : KDialog( parent )
{
	const QStringList maskedItems = QStringList() << "Name" << "Author" << "AuthorContact" << "Description";
	const QStringList junkheapEnumText = QStringList()
	                                     << i18nc("Quantity of junkheaps that can be pushed", "None")
	                                     << i18nc("Quantity of junkheaps that can be pushed", "One")
	                                     << i18nc("Quantity of junkheaps that can be pushed", "Many");

//	QScrollArea * scroll = new QScrollArea( this );
	QWidget * widget = new QWidget();
	QFormLayout * layout = new QFormLayout( widget );

	foreach( const KConfigSkeletonItem * item, ruleset->items() )
	{
		if ( !maskedItems.contains( item->name() ) )
		{
			QString labelText = item->label().isEmpty() ? item->name() : item->label();
			labelText = i18nc( "%1 is a pretranslated string that we're turning into a label", "%1:", labelText );

			QString valueText;
			if ( dynamic_cast<const KCoreConfigSkeleton::ItemBool *>( item ) )
				valueText = item->property().toBool() ? i18n("Yes") : i18n("No");
			else if ( item->name() == "PushableJunkheaps" )
				valueText = junkheapEnumText.at( item->property().toInt() );
			else
				valueText = item->property().toString();

			layout->addRow( new QLabel( labelText ), new QLabel( valueText ) );
		}
	}

// 	scroll->setWidget( widget );
// 	scroll->setAlignment( Qt::AlignHCenter | Qt:: AlignTop );
// 	setMainWidget( scroll );
	setMainWidget( widget );
	setButtons( KDialog::Close );
	setCaption( i18n("Details of %1 Game Type", ruleset->name() ) );
}



