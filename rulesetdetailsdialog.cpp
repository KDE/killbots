/*
 *  Copyright 2009  Parker Coates <parker.coates@kdemail.net>
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

Killbots::RulesetDetailsDialog::RulesetDetailsDialog( QWidget * parent )
  : KDialog( parent )
{
	setButtons( KDialog::Close );
}


void Killbots::RulesetDetailsDialog::loadRuleset( const Ruleset * ruleset )
{
	static QStringList maskedItems = QStringList() << "Name" << "Author" << "AuthorContact" << "Description";
	static QStringList junkheapEnumText = QStringList()
                                          << i18nc("Quantity of junkheaps that can be pushed", "None")
                                          << i18nc("Quantity of junkheaps that can be pushed", "One")
                                          << i18nc("Quantity of junkheaps that can be pushed", "Many");


	// If the dialog hasn't been setup already, do so.
	if ( m_labels.size() == 0 )
	{
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

		setMainWidget( widget );
	}

	QMap<QString,QLabel*>::const_iterator it = m_labels.constBegin();
	QMap<QString,QLabel*>::const_iterator end = m_labels.constEnd();
	for ( ; it != end; it++ )
	{
		const KConfigSkeletonItem * item = ruleset->findItem( it.key() );

		QString valueText;
		if ( dynamic_cast<const KCoreConfigSkeleton::ItemBool *>( item ) )
			valueText = item->property().toBool() ? i18n("Yes") : i18n("No");
		else if ( it.key() == "PushableJunkheaps" )
			valueText = junkheapEnumText.at( item->property().toInt() );
		else
			valueText = item->property().toString();

		it.value()->setText( valueText );
	}

	setCaption( i18n("Details of %1 Game Type", ruleset->name() ) );
}



