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

#include "optionspage.h"

#include <KDE/KComboBox>
#include <KDE/KDialog>
#include <KDE/KLocalizedString>

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>


OptionsPage::OptionsPage( QWidget *parent )
  : QWidget( parent )
{
	kcfg_AllowUnsafeMoves = new QCheckBox( i18n("Allow unsafe moves"), this );
	kcfg_AllowUnsafeMoves->setObjectName( "kcfg_AllowUnsafeMoves" );

	QStringList clickActionList;
	clickActionList << i18n("Nothing");
	clickActionList << i18n("Step");
	clickActionList << i18n("Repeated Step");
	clickActionList << i18n("Teleport");
	clickActionList << i18n("Teleport Safely");
	clickActionList << i18n("Teleport (Safely If Possible)");
	clickActionList << i18n("Wait Out Round");

	kcfg_MiddleClickAction = new KComboBox( false, this );
	kcfg_MiddleClickAction->setObjectName( "kcfg_MiddleClickAction" );
	kcfg_MiddleClickAction->addItems( clickActionList );

	kcfg_RightClickAction = new KComboBox( false, this );
	kcfg_RightClickAction->setObjectName( "kcfg_RightClickAction" );
	kcfg_RightClickAction->addItems( clickActionList );

	kcfg_AnimationSpeed = new QSlider( Qt::Horizontal, this );
	kcfg_AnimationSpeed->setObjectName( "kcfg_AnimationSpeed" );

	QLabel * label;

	QGridLayout * topLayout = new QGridLayout( this );
	topLayout->setMargin( 0 );
	topLayout->setSpacing( KDialog::spacingHint() );

	topLayout->addWidget( kcfg_AllowUnsafeMoves, 0, 0, 1, 2 );

	label = new QLabel( i18n("Middle-click action: "), this );
	label->setBuddy( kcfg_MiddleClickAction );
	label->setAlignment( Qt::AlignRight );
	topLayout->addWidget( label, 1, 0 );
	topLayout->addWidget( kcfg_MiddleClickAction, 1, 1 );

	label = new QLabel( i18n("Right-click action: "), this );
	label->setBuddy( kcfg_RightClickAction );
	label->setAlignment( Qt::AlignRight );
	topLayout->addWidget( label, 2, 0 );
	topLayout->addWidget( kcfg_RightClickAction, 2, 1 );

	label = new QLabel( i18n("Animation speed: "), this );
	label->setBuddy( kcfg_AnimationSpeed );
	label->setAlignment( Qt::AlignRight );
	topLayout->addWidget( label, 4, 0 );

	QGridLayout * speedLayout = new QGridLayout();
	speedLayout->setMargin( 0 );
	speedLayout->setSpacing( 0 );
	speedLayout->addWidget( kcfg_AnimationSpeed, 0, 0, 1, 3 );
	label = new QLabel( i18n("Slow"), this );
	label->setAlignment( Qt::AlignLeft );
	speedLayout->addWidget( label, 1, 0 );
	label = new QLabel( i18n("Fast"), this );
	label->setAlignment( Qt::AlignCenter );
	speedLayout->addWidget( label, 1, 1 );
	label = new QLabel( i18n("Instant"), this );
	label->setAlignment( Qt::AlignRight );
	speedLayout->addWidget( label, 1, 2 );
	topLayout->addLayout( speedLayout, 4, 1 );

	topLayout->setRowStretch( 5, 10 );
	topLayout->setColumnStretch( 1, 10 );
}

OptionsPage::~OptionsPage()
{
}

#include "optionspage.moc"
