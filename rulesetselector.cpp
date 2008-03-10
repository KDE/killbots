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
#include <KDE/KLocale>
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

	kcfg_ruleset = new QLineEdit( this );
	kcfg_ruleset->setObjectName( "kcfg_ruleset" );
	kcfg_ruleset->hide();
	layout->addWidget( kcfg_ruleset );

	m_listWidget = new QListWidget( this );
	#warning This isn't the right way to do this.
	m_listWidget->setMaximumHeight( 100 );
	layout->addWidget( m_listWidget );

	QGroupBox * groupBox = new QGroupBox( i18n("Details"), this );
	layout->addWidget( groupBox );
	layout->setStretchFactor( groupBox, 10000000 );

	QVBoxLayout * boxLayout = new QVBoxLayout( groupBox );

	QScrollArea * scrollArea = new QScrollArea();
	scrollArea->setFrameShape( QFrame::NoFrame );
	scrollArea->setWidgetResizable( true );
	boxLayout->addWidget( scrollArea );

	m_details = new QLabel( scrollArea );
	m_details->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_details->setWordWrap( true );
	scrollArea->setWidget( m_details );

	// Transfer the margin from the layout to the label
	m_details->setMargin( boxLayout->margin() );
	boxLayout->setMargin( 0 );

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
			m_rulesetMap.insert( ruleset->name(), ruleset );
			m_listWidget->addItem( ruleset->name() );

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
	kDebug() << "Updating ruleset data for " << rulesetName;
	Ruleset * ruleset = m_rulesetMap[rulesetName];

	kcfg_ruleset->setText( ruleset->fileName() );

	QString details;

	details += i18n("The game field is <b>%1</b> rows tall by <b>%2</b> columns wide.", ruleset->rows(), ruleset->columns() ) + "<br/><br/>";
	details += i18np("The game starts with <b>%1</b> enemy on the field.", "The game starts with <b>%1</b> enemies on the field.", ruleset->initialEnemyCount() ) + ' ';
	details += i18np("<b>%1</b> more enemy is added each round.", "<b>%1</b> more enemies are added each round.", ruleset->enemiesAddedEachRound() ) + ' ';

	if ( ruleset->includeSuperbots() )
	{
		details += i18np("There is <b>%1</b> fast enemy at the start of the game.", "There are <b>%1</b> fast enemies at the start of the game.", ruleset->initialFastEnemyCount() ) + ' ';
		details += i18np("<b>%1</b> more enemy is added each round.", "<b>%1</b> more enemies are added each round.", ruleset->fastEnemiesAddedEachRound() );
	}

	details += "<br/><br/>";

	if ( ruleset->initialJunkheapCount() )
		details += i18np("The game starts with <b>%1</b> junkheap.", "The game starts with <b>%1</b> junkheaps.", ruleset->initialJunkheapCount() ) + ' ';
	details += ( ruleset->includePushableJunkheaps() ? i18n("The hero <b>can</b> push junkheaps.") : i18n("The hero <b>cannot</b> push junkheaps.") ) + "<br/><br/>";

	if ( ruleset->includeEnergy() )
	{
		details += i18np("The hero starts with <b>%1</b> energy", "The hero starts with <b>%1</b> energy", ruleset->initialEnergy() ) + ' ';
		details += i18np("and can accumulate up to <b>%1</b> energy.", "and can accumulate up to <b>%1</b> energy.", ruleset->maximumEnergy() ) + ' ';
		if ( ruleset->energyPerWaitKill() )
			details += i18np("<b>%1</b> energy is awarded for each enemy destroyed while waiting out the round.", "<b>%1</b> energy are awarded for each enemy destroyed while waiting out the round.", ruleset->energyPerWaitKill() ) + ' ';
		if ( ruleset->includePushableJunkheaps() && ruleset->energyPerSquashKill() )
			details += i18np("<b>%1</b> energy is awarded for each enemy destroyed by pushing a junkheap.", "<b>%1</b> energy are awarded for each enemy destroyed by pushing a junkheap.", ruleset->energyPerSquashKill() ) + ' ';
		details += "<br/><br/>";
	}

	details += i18np("<b>%1</b> point is awarded for each enemy destroyed.", "<b>%1</b> points are awarded for each enemy destroyed.", ruleset->pointsPerRobotKill() ) + ' ';
	if ( ruleset->includeSuperbots() )
		details += i18np("<b>%1</b> point is awarded for each fast enemy destroyed.", "<b>%1</b> points are awarded for each fast enemy destroyed.", ruleset->pointsPerSuperbotKill() ) + ' ';
	if ( ruleset->pointsPerWaitKill() )
		details += i18np("<b>%1</b> point is awarded for each enemy destroyed while waiting out the round.", "<b>%1</b> points are awarded for each enemy destroyed while waiting out the round.", ruleset->pointsPerWaitKill() ) + ' ';
	if ( ruleset->includePushableJunkheaps() && ruleset->pointsPerSquashKill() )
		details += i18np("<b>%1</b> point is awarded for each enemy destroyed by pushing a junkheap.", "<b>%1</b> points are awarded for each enemy destroyed by pushing a junkheap.", ruleset->pointsPerSquashKill() ) + ' ';

	m_details->setText( details );
}
