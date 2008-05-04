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
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>

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
	kcfg_AnimationSpeed->setSingleStep( 1 );
	kcfg_AnimationSpeed->setPageStep( 1 );

	QGridLayout * speedLayout = new QGridLayout();
	speedLayout->setMargin( 0 );
	speedLayout->setSpacing( 0 );
	speedLayout->addWidget( kcfg_AnimationSpeed, 0, 0, 1, 3 );
	QLabel * label = new QLabel( i18n("Slow"), this );
	label->setAlignment( Qt::AlignLeft );
	speedLayout->addWidget( label, 1, 0 );
	label = new QLabel( i18n("Fast"), this );
	label->setAlignment( Qt::AlignCenter );
	speedLayout->addWidget( label, 1, 1 );
	label = new QLabel( i18n("Instant"), this );
	label->setAlignment( Qt::AlignRight );
	speedLayout->addWidget( label, 1, 2 );

	QFormLayout * formLayout = new QFormLayout( this );
	formLayout->setLabelAlignment( Qt::AlignRight );
	formLayout->setMargin( 0 );
	formLayout->setSpacing( KDialog::spacingHint() );
	formLayout->addRow( i18n("Middle-click action: "), kcfg_MiddleClickAction );
	formLayout->addRow( i18n("Right-click action: "), kcfg_RightClickAction );
	formLayout->addRow( i18n("Animation speed: "), speedLayout );
	formLayout->addRow( 0, kcfg_AllowUnsafeMoves );
}

OptionsPage::~OptionsPage()
{
}

#include "optionspage.moc"
