/*
 *  Copyright 2007-2009  Parker Coates <coates@kde.org>
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

#include "rulesetselector.h"

#include "ruleset.h"
#include "rulesetdetailsdialog.h"
#include "settings.h"

#include <KDE/KDebug>
#include <KDE/KLineEdit>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QTableWidget>

Killbots::RulesetSelector::RulesetSelector( QWidget * parent )
  : QWidget( parent ),
    m_rulesetMap(),
    m_detailsDialog( 0 )
{
	// Create a hidden KLineEdit to use the automatic KConfigXT connection
	kcfg_Ruleset = new KLineEdit();
	kcfg_Ruleset->setObjectName( QLatin1String( "kcfg_Ruleset" ) );
	kcfg_Ruleset->hide();

	m_listWidget = new QListWidget();
	m_listWidget->setWhatsThis( i18n("A list of the Killbots rulesets installed on this computer.") );

	QGroupBox * groupBox = new QGroupBox( i18n("Game Type Details") );
	groupBox->setWhatsThis( i18n("Lists information on the currently selected game type.") );

	QLabel * authorLabel = new QLabel( i18n("Author:") );
	authorLabel->setAlignment( Qt::AlignRight | Qt::AlignTop );

	m_author = new QLabel();
	m_author->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_author->setWordWrap( true );

	QLabel * contactLabel = new QLabel( i18n("Contact:") );
	contactLabel->setAlignment( Qt::AlignRight | Qt::AlignTop );

	m_authorContact = new QLabel();
	m_authorContact->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_authorContact->setWordWrap( true );
	m_authorContact->setOpenExternalLinks ( true );

	QLabel * descriptionLabel = new QLabel( i18n("Description:") );
	descriptionLabel->setAlignment( Qt::AlignRight | Qt::AlignTop );

	m_description = new QLabel();
	m_description->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_description->setWordWrap( true );

	QPushButton * detailsButton = new QPushButton( i18n("Details...") );
	detailsButton->setToolTip( i18n("Show the detailed parameters of the selected game type") );
	detailsButton->setWhatsThis( i18n("Opens a dialog listing the values of all internal parameters for the selected game type.") );

	QGridLayout * boxLayout = new QGridLayout( groupBox );
	boxLayout->addWidget( authorLabel, 1, 0 );
	boxLayout->addWidget( m_author, 1, 1 );
	boxLayout->addWidget( contactLabel, 2, 0 );
	boxLayout->addWidget( m_authorContact, 2, 1 );
	boxLayout->addWidget( descriptionLabel, 3, 0 );
	boxLayout->addWidget( m_description, 3, 1 );
	boxLayout->addWidget( detailsButton, 4, 1, Qt::AlignLeft );
	boxLayout->setColumnStretch( 1, 10 );
	boxLayout->setRowStretch( 5, 10 );

	QVBoxLayout * layout = new QVBoxLayout( this );
	layout->setMargin( 0 );
	layout->addWidget( kcfg_Ruleset );
	layout->addWidget( m_listWidget );
	layout->addWidget( groupBox, 10 );

	connect( m_listWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(selectionChanged(QString)) );
	connect( detailsButton, SIGNAL(clicked()), this, SLOT(showDetailsDialog()) );

	findRulesets();
}


Killbots::RulesetSelector::~RulesetSelector()
{
	qDeleteAll( m_rulesetMap );
}


void Killbots::RulesetSelector::findRulesets()
{
	qDeleteAll( m_rulesetMap );
	m_rulesetMap.clear();

	m_listWidget->clear();
	m_listWidget->setSortingEnabled( true );

	QStringList fileList;
	KGlobal::dirs()->findAllResources ( "ruleset", "*.desktop", KStandardDirs::NoDuplicates, fileList );
	foreach ( const QString & fileName, fileList )
	{
		const Ruleset * ruleset = Ruleset::load( fileName );
		if ( ruleset )
		{
			QString name = ruleset->name();
			while ( m_rulesetMap.contains( name ) )
				name += '_';

			m_rulesetMap.insert( name, ruleset );

			QListWidgetItem * item = new QListWidgetItem( name, m_listWidget );
			if ( fileName == Settings::ruleset() )
				m_listWidget->setCurrentItem( item );
		}
		else
			delete ruleset;
	}

	// Set the maximum height of the list widget to be no more than the size of its contents
	// This is slightly hackish, but the effect is nice.
	const int itemHeight = m_listWidget->visualItemRect( m_listWidget->item( 0 ) ).height();
	const int verticalMargin = m_listWidget->height() - m_listWidget->viewport()->height();
	m_listWidget->setMaximumHeight( itemHeight * m_listWidget->count() + verticalMargin );
}


void Killbots::RulesetSelector::selectionChanged( QString rulesetName )
{
	const Ruleset * ruleset = m_rulesetMap.value( rulesetName );

	kcfg_Ruleset->setText( ruleset->fileName() );

	m_author->setText( ruleset->author() );
	if ( ruleset->authorContact().contains(' ') )
		m_authorContact->setText( ruleset->authorContact() );
	else if ( ruleset->authorContact().contains('@') )
		m_authorContact->setText( QString("<qt><a href=\"mailto:%1\">%1</a></qt>").arg( ruleset->authorContact() ) );
	else if ( ruleset->authorContact().contains('.') )
		m_authorContact->setText( QString("<qt><a href=\"http://%1\">%1</a></qt>").arg( ruleset->authorContact() ) );
	else
		m_authorContact->setText( ruleset->authorContact() );
	m_description->setText( ruleset->description() );

	if ( m_detailsDialog && m_detailsDialog->isVisible() )
		m_detailsDialog->loadRuleset( ruleset );
}


void Killbots::RulesetSelector::showDetailsDialog()
{
	if ( !m_detailsDialog )
		m_detailsDialog = new RulesetDetailsDialog( this );

	const Ruleset * ruleset = m_rulesetMap.value( m_listWidget->currentItem()->text() );
	m_detailsDialog->loadRuleset( ruleset );
	m_detailsDialog->show();
}

