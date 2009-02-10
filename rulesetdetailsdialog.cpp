/*
 *  Copyright 2009  Parker Coates <parker.coates@gmail.com>
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

#include "rulesetdetailsdialog.h"

#include "ruleset.h"

#include <KDE/KLocale>

#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QScrollArea>

const QStringList Killbots::RulesetDetailsDialog::maskedItems = QStringList() << "Name" << "Author" << "AuthorContact" << "Description";
const QStringList Killbots::RulesetDetailsDialog::junkheapEnumText = QStringList()
                                                                     << i18nc("Quantity of junkheaps that can be pushed", "None")
                                                                     << i18nc("Quantity of junkheaps that can be pushed", "One")
                                                                     << i18nc("Quantity of junkheaps that can be pushed", "Many");


Killbots::RulesetDetailsDialog::RulesetDetailsDialog( QWidget * parent )
  : KDialog( parent )
{
	setButtons( KDialog::Close );
}


void Killbots::RulesetDetailsDialog::loadRuleset( const Ruleset * ruleset )
{
	if ( m_labels.size() == 0 )
	{
//		QScrollArea * scroll = new QScrollArea( this );
		QWidget * widget = new QWidget();
		QFormLayout * layout = new QFormLayout( widget );

		foreach ( const KConfigSkeletonItem * item, ruleset->items() )
		{
			if ( !maskedItems.contains( item->name() ) )
			{
				QString labelText = item->label().isEmpty() ? item->name() : item->label();
				labelText = i18nc( "%1 is a pretranslated string that we're turning into a label", "%1:", labelText );

				QLabel * valueLabel = new QLabel();
				m_labels[ item->name() ] = valueLabel;
				layout->addRow( new QLabel( labelText ), valueLabel );
			}
		}
// 		widget->adjustSize();
// 		scroll->setWidget( widget );
// 		scroll->setWidgetResizable( true );
// 		scroll->setAlignment( Qt::AlignHCenter | Qt:: AlignTop );
// 		scroll->resize( widget->size() );
// 		setMainWidget( scroll );
		setMainWidget( widget );
	}

	foreach ( const KConfigSkeletonItem * item, ruleset->items() )
	{
		if ( !maskedItems.contains( item->name() ) )
		{
			QString valueText;
			if ( dynamic_cast<const KCoreConfigSkeleton::ItemBool *>( item ) )
				valueText = item->property().toBool() ? i18n("Yes") : i18n("No");
			else if ( item->name() == "PushableJunkheaps" )
				valueText = junkheapEnumText.at( item->property().toInt() );
			else
				valueText = item->property().toString();

			m_labels.value( item->name() )->setText( valueText );
		}
	}

	setCaption( i18n("Details of %1 Game Type", ruleset->name() ) );
}



