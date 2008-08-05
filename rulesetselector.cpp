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
#include <KDE/KLineEdit>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QScrollArea>

Killbots::RulesetSelector::RulesetSelector( QWidget * parent )
  : QWidget( parent ),
	m_rulesetMap()
{
	QVBoxLayout * layout = new QVBoxLayout( this );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( 0 );

	kcfg_Ruleset = new KLineEdit( this );
	kcfg_Ruleset->setObjectName( "kcfg_Ruleset" );
	kcfg_Ruleset->hide();
	layout->addWidget( kcfg_Ruleset );

	m_listWidget = new QListWidget( this );
	m_listWidget->setWhatsThis( i18n("A list of the Killbots rulesets installed on this computer.") );
	layout->addWidget( m_listWidget );

	QGroupBox * groupBox = new QGroupBox( i18n("Details"), this );
	groupBox->setWhatsThis( i18n("Information on the currently selected ruleset.") );
	layout->addWidget( groupBox, 10 );

	QGridLayout * boxLayout = new QGridLayout( groupBox );
	QLabel * label;

	label = new QLabel( i18n("Author:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 1, 0 );
	m_author = new QLabel( groupBox );
	m_author->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_author->setWordWrap( true );
	boxLayout->addWidget( m_author, 1, 1 );

	label = new QLabel( i18n("Contact:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 2, 0 );
	m_authorContact = new QLabel( groupBox );
	m_authorContact->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_authorContact->setWordWrap( true );
	m_authorContact->setOpenExternalLinks ( true );
	boxLayout->addWidget( m_authorContact, 2, 1 );

	label = new QLabel( i18n("Description:"), groupBox );
	label->setAlignment( Qt::AlignRight | Qt::AlignTop );
	boxLayout->addWidget( label, 3, 0 );
	m_description = new QLabel( groupBox );
	m_description->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	m_description->setWordWrap( true );
	boxLayout->addWidget( m_description, 3, 1 );

	boxLayout->setHorizontalSpacing( 10 );
 	boxLayout->setColumnStretch( 0, 1 );
 	boxLayout->setColumnStretch( 1, 4 );
	boxLayout->setRowStretch( 4, 10 );

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
	m_listWidget->setSortingEnabled( true );

	QStringList fileList;
	KGlobal::dirs()->findAllResources ( "ruleset", "*.desktop", KStandardDirs::NoDuplicates, fileList );
	foreach ( const QString & fileName, fileList )
	{
		Ruleset * ruleset = Ruleset::load( fileName );
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

	// Restrict the height of the list widget to be slightly more than it's contents.
	// This is a bit hacky. There's probably a better way to do this, but it's the best I've found.
	m_listWidget->setMaximumHeight( 1.333 * m_listWidget->count() * m_listWidget->visualItemRect( m_listWidget->item( 0 ) ).height() );
}


void Killbots::RulesetSelector::selectionChanged( QString rulesetName )
{
	kDebug() << "Reading ruleset details for " << rulesetName;
	Ruleset * ruleset = m_rulesetMap[rulesetName];

	kcfg_Ruleset->setText( ruleset->fileName() );

	m_author->setText( ruleset->author() );
	if ( ruleset->authorContact().contains(' ') )
		m_authorContact->setText( ruleset->authorContact() );
	else if ( ruleset->authorContact().contains('@') )
		m_authorContact->setText( QString("<qt><a href=\"mailto:%1\">%1</a></qt>").arg( ruleset->authorContact() ) );
	else
		m_authorContact->setText( QString("<qt><a href=\"http://%1\">%1</a></qt>").arg( ruleset->authorContact() ) );
	m_description->setText( ruleset->description() );
}

