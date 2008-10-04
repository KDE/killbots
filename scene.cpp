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

#include "scene.h"

#include "gamestatusdisplayitem.h"
#include "render.h"
#include "ruleset.h"
#include "settings.h"
#include "sprite.h"

#include <KDE/KDebug>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>
#include <KDE/KGamePopupItem>

#include <QtGui/QFontInfo>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsView>

#include <cmath>


Killbots::Scene::Scene( QObject * parent )
  : QGraphicsScene( parent ),
    m_hero( 0 ),
    m_timeLine( 1000, this ),
    m_popupMessage( new KGamePopupItem ),
    m_roundDisplay( new GameStatusDisplayItem() ),
    m_scoreDisplay( new GameStatusDisplayItem() ),
    m_enemyCountDisplay( new GameStatusDisplayItem() ),
    m_energyDisplay( new GameStatusDisplayItem() )
{
	setItemIndexMethod( QGraphicsScene::NoIndex );

	m_timeLine.setCurveShape( QTimeLine::LinearCurve );
	setAnimationSpeed( Settings::animationSpeed() );
	connect( &m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animate(qreal)) );
	connect( &m_timeLine, SIGNAL(finished()), this, SIGNAL(animationDone()) );

	m_popupMessage->setMessageOpacity( 0.85 );
	addItem( m_popupMessage );
	connect( m_popupMessage, SIGNAL(hidden()), this, SIGNAL(animationDone()) );

	m_roundDisplay->setText( i18n("Round:") );
	m_roundDisplay->setDigits( 2 );
	addItem( m_roundDisplay );

	m_scoreDisplay->setText( i18n("Score:") );
	m_scoreDisplay->setDigits( 5 );
	addItem( m_scoreDisplay );

	m_enemyCountDisplay->setText( i18n("Enemies:") );
	m_enemyCountDisplay->setDigits( 3 );
	addItem( m_enemyCountDisplay );

	m_energyDisplay->setText( i18n("Energy:") );
	m_energyDisplay->setDigits( 2 );
	addItem( m_energyDisplay );
}


Killbots::Scene::~Scene()
{
}


void Killbots::Scene::doLayout()
{
	QSize size = views().first()->size();

	// Make certain layout properties proportional to the scene height,
	// but clamp them between reasonable values. There's probably room for more
	// tweaking here.
	const int baseDimension = qMin( size.width(), size.height() ) / 35;
	const int spacing = qBound( 5, baseDimension, 15 );
	const int newPixelSize = qBound( QFontInfo( QFont() ).pixelSize(), baseDimension, 25 );
	const qreal aspectRatio = Render::aspectRatio();

	// If the font size has changed, resize the display items.
	if ( m_roundDisplay->font().pixelSize() != newPixelSize )
	{
		QFont font = m_roundDisplay->font();
		font.setPixelSize( newPixelSize  );

		// After being given the new font, the displays will automatically
		// adjust their size to fit their contents.
		m_roundDisplay->setFont( font );
		m_scoreDisplay->setFont( font );
		m_enemyCountDisplay->setFont( font );
		m_energyDisplay->setFont( font );

		// Determine the size of the widest display. (This will depend on the locale.)
		qreal widest = qMax( m_roundDisplay->boundingRect().width(), m_scoreDisplay->boundingRect().width() );
		widest = qMax( widest, m_enemyCountDisplay->boundingRect().width() );
		if ( m_energyDisplay->isVisible() )
			widest = qMax( widest, m_energyDisplay->boundingRect().width() );
		m_displaySize = QSize( qRound( widest ), qRound( m_roundDisplay->boundingRect().height() ) );

		// Apply largest size to all displays.
		m_roundDisplay->setSize( m_displaySize );
		m_scoreDisplay->setSize( m_displaySize );
		m_enemyCountDisplay->setSize( m_displaySize );
		m_energyDisplay->setSize( m_displaySize );
	}

	// Determine the total width required to arrange the displays horizontally.
	int widthOfDisplaysOnTop = 3 * m_displaySize.width() + 2 * spacing;
	if ( m_energyDisplay->isVisible() )
		widthOfDisplaysOnTop += m_displaySize.width() + spacing;

	// The displays can either be placed centred, across the top of the
	// scene or top-aligned, down the side of the scene. We first calculate
	// what the cell size would be for both options.
	qreal cellWidthSide;
	int availableWidth = size.width() - 3 * spacing - m_displaySize.width();
	int availableHeight = size.height() - 2 * spacing;
	if ( availableWidth / m_columns < availableHeight / m_rows * aspectRatio )
		cellWidthSide = availableWidth / m_columns;
	else
		cellWidthSide = availableHeight / m_rows * aspectRatio;

	qreal cellWidthTop;
	availableWidth = size.width() - 2 * spacing;
	availableHeight = size.height() - 3 * spacing - m_displaySize.height();
	if ( availableWidth / m_columns < availableHeight / m_rows * aspectRatio )
		cellWidthTop = availableWidth / m_columns;
	else
		cellWidthTop = availableHeight / m_rows * aspectRatio;

	// If placing the displays on top would result in larger cells, we take
	// that option, but only if the displays would actually fit.
	const bool displaysOnTop = ( cellWidthTop > cellWidthSide && size.width() > widthOfDisplaysOnTop );
	const qreal newCellWidth = displaysOnTop ? cellWidthTop : cellWidthSide;
	const QSize newCellSize = QSize( qRound( newCellWidth ), qRound( newCellWidth / aspectRatio ) );

	// If the cellSize has actually changed, update all the sprites.
	if ( newCellSize != m_cellSize )
	{
		m_cellSize = newCellSize;

		foreach ( QGraphicsItem * item, items() )
		{
			Sprite * sprite = qgraphicsitem_cast<Sprite *>( item );
			if ( sprite )
			{
				sprite->setSize( m_cellSize );
				updateSpritePos( sprite );
			}
		}
	}

	kDebug() << "Doing layout @" << size;

	if ( displaysOnTop )
	{
		// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
		const qreal sceneRectXPos = -( size.width() - m_cellSize.width() * ( m_columns - 1 ) ) / 2.0;
		const qreal centeredYPos = - ( size.height() - m_cellSize.height() * ( m_rows - 1 ) ) / 2.0;
		const qreal indentedYPos = - ( m_cellSize.height() / 2.0 + 2 * spacing + m_displaySize.height() );
		const qreal sceneRectYPos = qMin( centeredYPos, indentedYPos );
		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );

		// Position the display items centered at the top of the scene
		const qreal displayYPos = ( sceneRectYPos - ( m_displaySize.height() + m_cellSize.height() / 2.0 ) ) / 2;

		m_roundDisplay->setPos( sceneRectXPos + ( size.width() - widthOfDisplaysOnTop ) / 2.0, displayYPos );
		m_scoreDisplay->setPos( m_roundDisplay->sceneBoundingRect().right() + spacing, displayYPos );
		m_enemyCountDisplay->setPos( m_scoreDisplay->sceneBoundingRect().right() + spacing, displayYPos );
		m_energyDisplay->setPos( m_enemyCountDisplay->sceneBoundingRect().right() + spacing, displayYPos );
	}
	else
	{
		qreal sceneRectXPos;
		const qreal centeredXPos = - ( size.width() - m_cellSize.width() * ( m_columns - 1 ) ) / 2.0;
		const qreal sceneRectYPos = -( size.height() - m_cellSize.height() * ( m_rows - 1 ) ) / 2.0;
		qreal displayXPos;

		// If the application layout is LTR, place the displays on left,
		// otherwise, place them on the right.
		if ( views().first()->layoutDirection() == Qt::LeftToRight )
		{
			// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
			const qreal indentedXPos = - ( m_cellSize.width() / 2.0 + 2 * spacing + m_displaySize.width() );
			sceneRectXPos = qMin( centeredXPos, indentedXPos );

			// Position the display items to the left of the grid
			displayXPos = - ( spacing + m_displaySize.width() + m_cellSize.width() / 2 );
		}
		else
		{
			// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
			const qreal indentedXPos = ( m_cellSize.width() * m_columns + 1 * spacing + m_displaySize.width() ) - size.width();
			sceneRectXPos = qMax( centeredXPos, indentedXPos );

			// Position the display items to the right of the grid
			displayXPos = m_cellSize.width() * ( m_columns - 0.5 ) + spacing;
		}

		m_roundDisplay->setPos( displayXPos, -m_cellSize.height() / 2 );
		m_scoreDisplay->setPos( displayXPos, m_roundDisplay->sceneBoundingRect().bottom() + spacing );
		m_enemyCountDisplay->setPos( displayXPos, m_scoreDisplay->sceneBoundingRect().bottom() + spacing );
		m_energyDisplay->setPos( displayXPos, m_enemyCountDisplay->sceneBoundingRect().bottom() + spacing );

		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );
	}

	update();
}


Killbots::Sprite * Killbots::Scene::createSprite( SpriteType type, QPoint position )
{
	Sprite * sprite = new Sprite();
	sprite->setSpriteType( type );
	sprite->setSize( m_cellSize );
	sprite->setGridPos( position );
	sprite->setPos( -1000000.0, -1000000.0 );
	sprite->setZValue( type );

	addItem( sprite );
	m_spritesToCreate << sprite;

	if ( type == Hero )
		m_hero = sprite;

	return sprite;
}


void Killbots::Scene::slideSprite( Sprite * sprite, QPoint position )
{
	sprite->storeGridPos();
	sprite->setGridPos( position );
	m_spritesToSlide << sprite;
}


void Killbots::Scene::teleportSprite( Sprite * sprite, QPoint position )
{
	sprite->storeGridPos();
	sprite->setGridPos( position );
	m_spritesToTeleport << sprite;
}


void Killbots::Scene::destroySprite( Sprite * sprite )
{
	if ( sprite->spriteType() == Hero )
		m_hero = 0;

	m_spritesToDestroy << sprite;
}


void Killbots::Scene::setAnimationSpeed( int speed )
{
	// Equation converts speed in the range 0 to 10 to a duration in the range
	// 1 to 0.05 seconds. There's probably a better way to do this.
	m_timeLine.setDuration( int( pow( 1.35, -speed ) * 1000 ) );
}


void Killbots::Scene::startAnimation()
{
	if ( m_spritesToCreate.isEmpty()
	     && m_spritesToSlide.isEmpty()
	     && m_spritesToTeleport.isEmpty()
	     && m_spritesToDestroy.isEmpty()
	   )
	{
		emit animationDone();
	}
	else if ( m_timeLine.duration() < 60 )
	{
		animate( 1.0 );
		emit animationDone();
	}
	else
		m_timeLine.start();
}


void Killbots::Scene::animate( qreal value )
{
	static bool halfDone;

	if ( value == 0.0 )
	{
		halfDone = false;
		foreach ( Sprite * sprite, m_spritesToCreate )
		{
			sprite->scale( value, value );
			updateSpritePos( sprite );
		}
	}
	else if ( 0.0 < value && value < 1.0 )
	{
		foreach ( Sprite * sprite, m_spritesToCreate )
		{
			updateSpritePos( sprite );
			sprite->resetTransform();
			sprite->scale( value, value );
		}

		foreach ( Sprite * sprite, m_spritesToSlide )
		{
			QPointF posInGridCoordinates = value * QPointF( sprite->gridPos() - sprite->storedGridPos() ) + sprite->storedGridPos();
			sprite->setPos( QPointF( posInGridCoordinates.x() * m_cellSize.width(), posInGridCoordinates.y() * m_cellSize.height() ) );
		}

		qreal scaleFactor;
		if ( value >= 0.5 )
		{
			if ( !halfDone )
			{
				halfDone = true;
				foreach ( Sprite * sprite, m_spritesToTeleport )
					updateSpritePos( sprite );
			}
			scaleFactor = 2 * value - 1.0;
		}
		else
			scaleFactor = 1.0 - 2 * value;

		foreach ( Sprite * sprite, m_spritesToTeleport )
		{
			sprite->resetTransform();
			sprite->scale( scaleFactor, scaleFactor );
		}

		foreach ( Sprite * sprite, m_spritesToDestroy )
		{
			sprite->resetTransform();
			sprite->scale( 1 - value, 1 - value );
			sprite->rotate( value * 360 );
		}
	}
	else if ( value == 1.0 )
	{
		foreach ( Sprite * sprite, m_spritesToSlide + m_spritesToTeleport + m_spritesToCreate )
		{
			updateSpritePos( sprite );
			sprite->resetTransform();
			sprite->storeGridPos();
		}

		m_spritesToCreate.clear();
		m_spritesToSlide.clear();
		m_spritesToTeleport.clear();

		qDeleteAll( m_spritesToDestroy );
		m_spritesToDestroy.clear();
	}
}


void Killbots::Scene::onNewGame( int rows, int columns, bool gameIncludesEnergy )
{
	m_rows = rows;
	m_columns = columns;
	m_energyDisplay->setVisible( gameIncludesEnergy );

	doLayout();

	// Prevent the "New Game" popup from appearing when the application is first launched.
	static bool firstGame = true;
	if ( firstGame )
		firstGame = false;
	else
		showMessagePopup( i18n("New game."), 3000 );
}


void Killbots::Scene::onRoundComplete()
{
	showMessagePopup( i18n("Round complete."), 3000 );
}


void Killbots::Scene::onBoardFull()
{
	showMessagePopup( i18n("Board is full.\nResetting enemy counts."), 4000 );
}


void Killbots::Scene::onGameOver()
{
	showMessagePopup( i18n("Game over."), 20000 );
}


void Killbots::Scene::updateRound( int round )
{
	m_roundDisplay->setValue( round );
}


void Killbots::Scene::updateScore( int score )
{
	m_scoreDisplay->setValue( score );
}


void Killbots::Scene::updateEnemyCount( int enemyCount )
{
	m_enemyCountDisplay->setValue( enemyCount );
}


void Killbots::Scene::updateEnergy( int energy )
{
	m_energyDisplay->setValue( energy );
}


void Killbots::Scene::drawBackground( QPainter * painter, const QRectF & )
{
	painter->drawPixmap( sceneRect().topLeft(), Render::renderElement( "background", QSize( qRound( sceneRect().width() ), qRound( sceneRect().height() ) ) ) );
	painter->drawPixmap( -m_cellSize.width() / 2, -m_cellSize.height() / 2, Render::renderGrid( m_columns, m_rows, m_cellSize ) );
}


void Killbots::Scene::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
	getMouseDirection( event->scenePos() );
	event->accept();
	QGraphicsScene::mouseMoveEvent( event );
}


void Killbots::Scene::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
	Settings::ClickAction userAction = Settings::Nothing;

	if ( event->button() == Qt::LeftButton )
	{
		if ( event->modifiers() & Qt::ControlModifier )
			userAction = Settings::middleClickAction();
		else
			userAction = Settings::Step;
	}
	else if ( event->button() == Qt::RightButton )
		userAction = Settings::rightClickAction();
	else if ( event->button() == Qt::MidButton )
		userAction = Settings::middleClickAction();

	if ( userAction == Settings::Step )
		emit clicked( getMouseDirection( event->scenePos() ) );
	else if ( userAction == Settings::RepeatedStep )
		emit clicked( -getMouseDirection( event->scenePos() ) - 1 );
	else if ( userAction == Settings::Teleport )
		emit clicked( Teleport );
	else if ( userAction == Settings::TeleportSafely )
		emit clicked( TeleportSafely );
	else if ( userAction == Settings::TeleportSafelyIfPossible )
		emit clicked( TeleportSafelyIfPossible );
	else if ( userAction == Settings::WaitOutRound )
		emit clicked( WaitOutRound );

	event->accept();

	QGraphicsScene::mouseReleaseEvent( event );
}


Killbots::HeroAction Killbots::Scene::getMouseDirection( QPointF cursorPosition )
{
	HeroAction result;

	if ( m_hero )
	{
		if ( m_hero->sceneBoundingRect().contains( cursorPosition ) )
			result = Hold;
		else
		{
			static const qreal piOver4 = 0.78539816339744830961566L;

			QPointF delta = cursorPosition - m_hero->sceneBoundingRect().center();
			int direction = qRound( atan2( -delta.y(), delta.x() ) / piOver4 );
			if ( direction < 0 )
				direction += 8;

			result = static_cast<HeroAction>( direction );
		}

		views().first()->setCursor( Render::cursorFromAction( result ) );
	}
	else
	{
		views().first()->unsetCursor();
		result = Hold;
	}

	return result;
}


void Killbots::Scene::updateSpritePos( Sprite * sprite ) const
{
	sprite->setPos( QPointF( sprite->gridPos().x() * m_cellSize.width(), sprite->gridPos().y() * m_cellSize.height() ) );
}


void Killbots::Scene::showMessagePopup( const QString & text, int timeout )
{
	KGamePopupItem::Position corner = views().first()->layoutDirection() == Qt::LeftToRight ? KGamePopupItem::TopRight : KGamePopupItem::TopLeft;
	m_popupMessage->setMessageTimeout( timeout );
	m_popupMessage->showMessage( text, corner, KGamePopupItem::ReplacePrevious );
}


#include "scene.moc"
