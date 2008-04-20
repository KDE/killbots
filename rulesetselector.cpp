/*
 *  Killbots
 *  Copyright (C) 2006-2008  Parker Coates <parker.coates@gmail.com>
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

#include "rulesetselector.h"

#include "ruleset.h"
#include "settings.h"

#include <KDE/KDebug>
#include <KDE/KDialog>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QScrollArea>

Killbots::RulesetSelector::RulesetSelector( QWidget * parent )
  : QWidget( parent ),
	m_rulesetMap()
{
	QVBoxLayout * layout = new QVBoxLayout( this );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( 0 );

	kcfg_Ruleset = new QLineEdit( this );
	kcfg_Ruleset->setObjectName( "kcfg_Ruleset" );
	kcfg_Ruleset->hide();
	layout->addWidget( kcfg_Ruleset );

	m_listWidget = new QListWidget( this );
	layout->addWidget( m_listWidget );

	QGroupBox * groupBox = new QGroupBox( i18n("Details"), this );
	layout->addWidget( groupBox );
	layout->setStretchFactor( groupBox, 10000000 );

	QGridLayout * boxLayout = new QGridLayout( groupBox );
	QLabel * label;

	label = new QLabel( i18n("Grid:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 0, 0 );
	m_gridDetails = new QLabel( groupBox );
	m_gridDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_gridDetails->setWordWrap( true );
	boxLayout->addWidget( m_gridDetails, 0, 1 );

	label = new QLabel( i18n("Robots:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 1, 0 );
	m_robotDetails = new QLabel( groupBox );
	m_robotDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_robotDetails->setWordWrap( true );
	boxLayout->addWidget( m_robotDetails, 1, 1 );

	label = new QLabel( i18n("Fastbots:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 2, 0 );
	m_fastbotDetails = new QLabel( groupBox );
	m_fastbotDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_fastbotDetails->setWordWrap( true );
	boxLayout->addWidget( m_fastbotDetails, 2, 1 );

	label = new QLabel( i18n("Junkheaps:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 3, 0 );
	m_junkheapDetails = new QLabel( groupBox );
	m_junkheapDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_junkheapDetails->setWordWrap( true );
	boxLayout->addWidget( m_junkheapDetails, 3, 1 );

	label = new QLabel( i18n("Score:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 4, 0 );
	m_scoreDetails = new QLabel( groupBox );
	m_scoreDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_scoreDetails->setWordWrap( true );
	boxLayout->addWidget( m_scoreDetails, 4, 1 );

	label = new QLabel( i18n("Energy:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 5, 0 );
	m_energyDetails = new QLabel( groupBox );
	m_energyDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_energyDetails->setWordWrap( true );
	boxLayout->addWidget( m_energyDetails, 5, 1 );

	label = new QLabel( i18n("Max Energy:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 6, 0 );
	m_maxEnergyDetails = new QLabel( groupBox );
	m_maxEnergyDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_maxEnergyDetails->setWordWrap( true );
	boxLayout->addWidget( m_maxEnergyDetails, 6, 1 );

	label = new QLabel( i18n("Wait out round:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 7, 0 );
	m_waitDetails = new QLabel( groupBox );
	m_waitDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_waitDetails->setWordWrap( true );
	boxLayout->addWidget( m_waitDetails, 7, 1 );

	label = new QLabel( i18n("Junkheap push:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 8, 0 );
	m_squashDetails = new QLabel( groupBox );
	m_squashDetails->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_squashDetails->setWordWrap( true );
	boxLayout->addWidget( m_squashDetails, 8, 1 );

	boxLayout->setHorizontalSpacing( 10 );
	boxLayout->setColumnStretch( 0, 1 );
	boxLayout->setColumnStretch( 1, 3 );
	boxLayout->setRowStretch( 9, 10 );

	connect( m_listWidget, SIGNAL(currentTextChanged(QString)), this, SLOT(selectionChanged(QString)) );

	findRulesets();
}


Killbots::RulesetSelector::~RulesetSelector()
{
	qDeleteAll( m_rulesetMap );
}


void Killbots::RulesetSelector::findRulesets()
{
	m_listWidget->clear();

	QStringList fileList;
	KGlobal::dirs()->findAllResources ( "ruleset", "*.desktop", KStandardDirs::NoDuplicates, fileList );
	foreach ( QString fileName, fileList )
	{
		Ruleset * ruleset = new Ruleset( fileName );
		if ( ruleset->isValid() )
		{
			QString name = ruleset->name();
			if ( m_rulesetMap.contains( name ) )
				name += '_';

			m_rulesetMap.insert( name, ruleset );
			m_listWidget->addItem( name );

			if ( fileName == Settings::ruleset() )
				m_listWidget->setCurrentRow( m_listWidget->count() - 1 );
		}
		else
			delete ruleset;
	}

	m_listWidget->sortItems( Qt::AscendingOrder );
}


void Killbots::RulesetSelector::selectionChanged( QString rulesetName )
{
	kDebug() << "Reading ruleset details for " << rulesetName;
	Ruleset * ruleset = m_rulesetMap[rulesetName];

	kcfg_Ruleset->setText( ruleset->fileName() );

	#warning I don't think these strings are translator friendly.
	m_gridDetails->setText( i18n("%1 rows, %2 columns", ruleset->rows(), ruleset->columns() ) );
	m_robotDetails->setText( i18n("%1 at start of game, %2 more added each round", ruleset->robotsAtGameStart(), ruleset->robotsAddedEachRound() ) );
	m_fastbotDetails->setText( i18n("%1 at start of game, %2 more added each round", ruleset->fastbotsAtGameStart(), ruleset->fastbotsAddedEachRound() ) );
	m_junkheapDetails->setText( i18n("%1 pushable. %2 added at start of round", ruleset->junkheapsArePushable() ? i18n("Are") : i18n("Are not"), ruleset->junkheapsAtGameStart() ) );
	m_scoreDetails->setText( i18n("%1 points for each robot destroyed, %2 for each fastbot", ruleset->pointsPerRobotKilled(), ruleset->pointsPerFastbotKilled() ) );
	m_energyDetails->setText( i18n("%1 at start of game", ruleset->energyAtGameStart() ) );
	m_maxEnergyDetails->setText( i18n("%1 at start of game", ruleset->maxEnergyAtGameStart() ) );
	m_waitDetails->setText( i18n("%1 points, %2 energy bonus for each enemy destroyed", ruleset->waitKillPointBonus(), ruleset->waitKillEnergyBonus() ) );
	m_squashDetails->setText( i18n("%1 points, %2 energy bonus for each enemy destroyed", ruleset->squashKillPointBonus(), ruleset->squashKillEnergyBonus() ) );

}
