/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@gmail.com>
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

#include "scene.h"

#include "gamestatusdisplayitem.h"
#include "render.h"
#include "ruleset.h"
#include "settings.h"
#include "sprite.h"

#include <kgamepopupitem.h>

#include <KDE/KDebug>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>

#include <QtGui/QFontInfo>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsView>

#include <cmath>


struct Killbots::Scene::AnimationStage
{
	AnimationStage()
	  : newRound( -1 ),
	    newScore( -1 ),
	    newEnemyCount( -1 ),
	    newEnergy( -1 )
	{};

	bool isEmpty() const
	{
		return spritesToCreate.isEmpty()
			&& spritesToSlide.isEmpty()
			&& spritesToTeleport.isEmpty()
			&& spritesToDestroy.isEmpty()
			&& message.isEmpty()
			&& newRound == -1
			&& newScore == -1
			&& newEnemyCount == -1
			&& newEnergy == -1;
	};

	QList<Sprite *> spritesToCreate;
	QList<Sprite *> spritesToSlide;
	QList<Sprite *> spritesToTeleport;
	QList<Sprite *> spritesToDestroy;
	QString message;
	int oldRound, newRound;
	int oldScore, newScore;
	int oldEnemyCount, newEnemyCount;
	int oldEnergy, newEnergy;
};


Killbots::Scene::Scene( QObject * parent )
  : QGraphicsScene( parent ),
    m_hero( 0 ),
    m_timeLine( 1000, this ),
    m_unqueuedPopup( new KGamePopupItem ),
    m_queuedPopup( new KGamePopupItem ),
    m_roundDisplay( new GameStatusDisplayItem() ),
    m_scoreDisplay( new GameStatusDisplayItem() ),
    m_enemyCountDisplay( new GameStatusDisplayItem() ),
    m_energyDisplay( new GameStatusDisplayItem() ),
    m_rows( 0 ),
    m_columns( 0 )
{
	setItemIndexMethod( QGraphicsScene::NoIndex );

	m_timeLine.setCurveShape( QTimeLine::EaseInOutCurve );
	setAnimationSpeed( Settings::animationSpeed() );
	connect( &m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animate(qreal)) );
	connect( &m_timeLine, SIGNAL(finished()), this, SIGNAL(animationStageDone()) );
	connect( this, SIGNAL(animationStageDone()), this, SLOT(nextAnimationStage()) );

	m_unqueuedPopup->setMessageOpacity( 0.85 );
	m_unqueuedPopup->setHideOnMouseClick( true );
	addItem( m_unqueuedPopup );

	m_queuedPopup->setMessageOpacity( 0.85 );
	m_queuedPopup->setHideOnMouseClick( true );
	addItem( m_queuedPopup );
	connect( m_queuedPopup, SIGNAL(hidden()), this, SIGNAL(animationStageDone()) );

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

	QList<GameStatusDisplayItem *> displayList;
	displayList << m_roundDisplay << m_scoreDisplay << m_enemyCountDisplay;
	if ( m_energyDisplay->isVisible() )
		displayList << m_energyDisplay;

	// If no game has been started
	if ( m_rows == 0 || m_columns == 0 )
	{
		setSceneRect( QRectF( QPointF( 0, 0 ), size ) );
		foreach ( GameStatusDisplayItem * display, displayList )
			display->setPos( -1000000, 0 );
		return;
	}

	kDebug() << "Laying out scene at" << size;

	// Make certain layout properties proportional to the scene height,
	// but clamp them between reasonable values. There's probably room for more
	// tweaking here.
	const int baseDimension = qMin( size.width(), size.height() ) / 35;
	const int spacing = qBound( 5, baseDimension, 15 );
	const int newPixelSize = qBound( QFontInfo( QFont() ).pixelSize(), baseDimension, 25 );
	const qreal aspectRatio = Render::aspectRatio();



	QSize displaySize;
	// If the font size has changed, resize the display items.
	// Note that we check the font size of the last display in the list so we
	// notice if the energy display has just been included.
	if ( displayList.last()->font().pixelSize() != newPixelSize )
	{
		QFont font = displayList.first()->font();
		font.setPixelSize( newPixelSize  );

		foreach ( GameStatusDisplayItem * display, displayList )
		{
			display->setFont( font );
			QSize preferredSize = display->preferredSize();
			if ( preferredSize.width() > displaySize.width() )
				displaySize.setWidth( preferredSize.width() );
			if ( preferredSize.height() > displaySize.height() )
				displaySize.setHeight( preferredSize.height() );
		}
		foreach ( GameStatusDisplayItem * display, displayList )
			display->setSize( displaySize );
	}
	else
	{
		displaySize = displayList.first()->boundingRect().size().toSize();
	}

	// Determine the total width required to arrange the displays horizontally.
	int widthOfDisplaysOnTop = displayList.size() * displaySize.width()
	                           + ( displayList.size() - 1 ) * spacing;

	// The displays can either be placed centred, across the top of the
	// scene or top-aligned, down the side of the scene. We first calculate
	// what the cell size would be for both options.
	qreal cellWidthSide;
	int availableWidth = size.width() - 3 * spacing - displaySize.width();
	int availableHeight = size.height() - 2 * spacing;
	if ( availableWidth / m_columns < availableHeight / m_rows * aspectRatio )
		cellWidthSide = availableWidth / m_columns;
	else
		cellWidthSide = availableHeight / m_rows * aspectRatio;

	qreal cellWidthTop;
	availableWidth = size.width() - 2 * spacing;
	availableHeight = size.height() - 3 * spacing - displaySize.height();
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

	if ( displaysOnTop )
	{
		// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
		const qreal sceneRectXPos = -( size.width() - m_cellSize.width() * ( m_columns - 1 ) ) / 2.0;
		const qreal centeredYPos = - ( size.height() - m_cellSize.height() * ( m_rows - 1 ) ) / 2.0;
		const qreal indentedYPos = - ( m_cellSize.height() / 2.0 + 2 * spacing + displaySize.height() );
		const qreal sceneRectYPos = qMin( centeredYPos, indentedYPos );
		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );

		// Position the display items centered at the top of the scene
		const qreal displayYPos = ( sceneRectYPos - ( displaySize.height() + m_cellSize.height() / 2.0 ) ) / 2;

		int xPos = sceneRectXPos + ( size.width() - widthOfDisplaysOnTop ) / 2.0;
		foreach ( GameStatusDisplayItem * display, displayList )
		{
			display->setPos( xPos, displayYPos );
			xPos += displaySize.width() + spacing;
		}
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
			const qreal indentedXPos = - ( m_cellSize.width() / 2.0 + 2 * spacing + displaySize.width() );
			sceneRectXPos = qMin( centeredXPos, indentedXPos );

			// Position the display items to the left of the grid
			displayXPos = - ( spacing + displaySize.width() + m_cellSize.width() / 2 );
		}
		else
		{
			// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
			const qreal indentedXPos = ( m_cellSize.width() * m_columns + 1 * spacing + displaySize.width() ) - size.width();
			sceneRectXPos = qMax( centeredXPos, indentedXPos );

			// Position the display items to the right of the grid
			displayXPos = m_cellSize.width() * ( m_columns - 0.5 ) + spacing;
		}

		int yPos = -m_cellSize.height() / 2;
		foreach ( GameStatusDisplayItem * display, displayList )
		{
			display->setPos( displayXPos, yPos );
			yPos += displaySize.height() + spacing;
		}

		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );
	}

	update();
}


void Killbots::Scene::setAnimationSpeed( int speed )
{
	// Equation converts speed in the range 0 to 10 to a duration in the range
	// 1 to 0.05 seconds. There's probably a better way to do this.
	m_timeLine.setDuration( int( pow( 1.35, -speed ) * 1000 ) );
}


void Killbots::Scene::beginNewAnimationStage()
{
	if ( m_stages.isEmpty() )
	{
		AnimationStage newStage;
		newStage.oldRound = m_roundDisplay->value();
		newStage.oldScore = m_scoreDisplay->value();
		newStage.oldEnemyCount = m_enemyCountDisplay->value();
		newStage.oldEnergy = m_energyDisplay->value();
		m_stages << newStage;
	}
	else if ( !m_stages.last().isEmpty() )
	{
		AnimationStage newStage;
		const AnimationStage & lastStage = m_stages.last();
		newStage.oldRound = lastStage.newRound == -1 ? lastStage.oldRound : lastStage.newRound;
		newStage.oldScore = lastStage.newScore == -1 ? lastStage.oldScore : lastStage.newScore;
		newStage.oldEnemyCount = lastStage.newEnemyCount == -1 ? lastStage.oldEnemyCount : lastStage.newEnemyCount;
		newStage.oldEnergy = lastStage.newEnergy == -1 ? lastStage.oldEnergy : lastStage.newEnergy;
		m_stages << newStage;
	}
}


Killbots::Sprite * Killbots::Scene::createSprite( SpriteType type, QPoint position )
{
	Sprite * sprite = new Sprite();
	sprite->setSpriteType( type );
	sprite->setSize( m_cellSize );
	sprite->setGridPos( position );
	sprite->setPos( -1000000.0, -1000000.0 );

	// A bit of a hack, but we use the sprite type for stacking order.
	sprite->setZValue( type );

	addItem( sprite );
	m_stages.last().spritesToCreate << sprite;

	if ( type == Hero )
		m_hero = sprite;

	return sprite;
}


void Killbots::Scene::slideSprite( Sprite * sprite, QPoint position )
{
	sprite->storeGridPos();
	sprite->setGridPos( position );
	m_stages.last().spritesToSlide << sprite;
}


void Killbots::Scene::teleportSprite( Sprite * sprite, QPoint position )
{
	sprite->storeGridPos();
	sprite->setGridPos( position );
	m_stages.last().spritesToTeleport << sprite;
}


void Killbots::Scene::destroySprite( Sprite * sprite )
{
	if ( sprite->spriteType() == Hero )
		m_hero = 0;

	m_stages.last().spritesToDestroy << sprite;
}


void Killbots::Scene::showQueuedMessage( const QString & message )
{
	m_stages.last().message = message;
}


void Killbots::Scene::showUnqueuedMessage( const QString & message, int timeout )
{
	if ( !m_queuedPopup->isVisible() )
	{
		KGamePopupItem::Position corner = views().first()->layoutDirection() == Qt::LeftToRight ? KGamePopupItem::TopRight : KGamePopupItem::TopLeft;
		m_unqueuedPopup->setMessageTimeout( timeout );
		m_unqueuedPopup->showMessage( message, corner, KGamePopupItem::ReplacePrevious );
	}
}


void Killbots::Scene::updateRound( int round )
{
	m_stages.last().newRound = round;
}


void Killbots::Scene::updateScore( int score )
{
	m_stages.last().newScore = score;
}


void Killbots::Scene::updateEnemyCount( int enemyCount )
{
	m_stages.last().newEnemyCount = enemyCount;
}


void Killbots::Scene::updateEnergy( int energy )
{
	m_stages.last().newEnergy = energy;
}


void Killbots::Scene::startAnimation()
{
	startAnimationStage();
}


void Killbots::Scene::startAnimationStage()
{
	QString message = m_stages.first().message;

	if ( m_timeLine.duration() < 60 && message.isEmpty() )
	{
		animate( 1.0 );
		emit animationStageDone();
	}
	else
	{
		if ( !message.isEmpty() )
		{
			if ( m_unqueuedPopup->isVisible() )
				m_unqueuedPopup->hide();
			KGamePopupItem::Position corner = views().first()->layoutDirection() == Qt::LeftToRight ? KGamePopupItem::TopRight : KGamePopupItem::TopLeft;
			m_queuedPopup->setMessageTimeout( 3000 );
			m_queuedPopup->showMessage( message, corner, KGamePopupItem::ReplacePrevious );
		}

		m_timeLine.start();
	}
}


void Killbots::Scene::animate( qreal value )
{
	static bool halfDone;
	AnimationStage stage = m_stages.first();

	if ( stage.newRound != -1 )
		m_roundDisplay->setValue( int( stage.oldRound + value * ( stage.newRound - stage.oldRound ) ) );

	if ( stage.newScore != -1 )
		m_scoreDisplay->setValue( int( stage.oldScore + value * ( stage.newScore - stage.oldScore ) ) );

	if ( stage.newEnemyCount != -1 )
		m_enemyCountDisplay->setValue( int( stage.oldEnemyCount + value * ( stage.newEnemyCount - stage.oldEnemyCount ) ) );

	if ( stage.newEnergy != -1 )
		m_energyDisplay->setValue( int( stage.oldEnergy + value * ( stage.newEnergy - stage.oldEnergy ) ) );

	if ( value == 0.0 )
	{
		halfDone = false;
		foreach ( Sprite * sprite, stage.spritesToCreate )
		{
			sprite->scale( value, value );
			updateSpritePos( sprite );
		}
	}
	else if ( 0.0 < value && value < 1.0 )
	{
		foreach ( Sprite * sprite, stage.spritesToCreate )
		{
			updateSpritePos( sprite );
			sprite->resetTransform();
			sprite->scale( value, value );
		}

		foreach ( Sprite * sprite, stage.spritesToSlide )
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
				foreach ( Sprite * sprite, stage.spritesToTeleport )
					updateSpritePos( sprite );
			}
			scaleFactor = 2 * value - 1.0;
		}
		else
		{
			scaleFactor = 1.0 - 2 * value;
		}

		foreach ( Sprite * sprite, stage.spritesToTeleport )
		{
			sprite->resetTransform();
			sprite->scale( scaleFactor, scaleFactor );
		}

		foreach ( Sprite * sprite, stage.spritesToDestroy )
		{
			sprite->resetTransform();
			sprite->scale( 1 - value, 1 - value );
			sprite->rotate( value * 360 );
		}
	}
	else if ( value == 1.0 )
	{
		foreach ( Sprite * sprite, stage.spritesToSlide + stage.spritesToTeleport + stage.spritesToCreate )
		{
			updateSpritePos( sprite );
			sprite->resetTransform();
			sprite->storeGridPos();
		}

		qDeleteAll( stage.spritesToDestroy );
	}
}


void Killbots::Scene::nextAnimationStage()
{
	// Wait for both the timeline and the popup to finish before moving to the next stage.
	if ( m_timeLine.state() != QTimeLine::Running && !m_queuedPopup->isVisible() )
	{
		m_stages.removeFirst();

		if ( m_stages.size() )
			startAnimationStage();
		else
			emit animationDone();
	}
}


void Killbots::Scene::onNewGame( int rows, int columns, bool gameIncludesEnergy )
{
	if (    m_rows != rows
	     || m_columns != columns
	     || m_energyDisplay->isVisible() != gameIncludesEnergy
	   )
	{
		m_rows = rows;
		m_columns = columns;
		m_energyDisplay->setVisible( gameIncludesEnergy );
		doLayout();
	}
}


void Killbots::Scene::showNewGameMessage()
{
	showUnqueuedMessage( i18n("New game.") );
}


void Killbots::Scene::showRoundCompleteMessage()
{
	showQueuedMessage( i18n("Round complete.") );
}


void Killbots::Scene::showBoardFullMessage()
{
	showQueuedMessage( i18n("Board is full.\nResetting enemy counts.") );
}


void Killbots::Scene::showGameOverMessage()
{
	m_hero = 0;
	showUnqueuedMessage( i18n("Game over."), 15000 );
}


void Killbots::Scene::drawBackground( QPainter * painter, const QRectF & )
{
	painter->drawPixmap( sceneRect().topLeft(), Render::renderElement( "background", QSize( qRound( sceneRect().width() ), qRound( sceneRect().height() ) ) ) );

	QRect gridArea( -m_cellSize.width() / 2, -m_cellSize.height() / 2, m_columns * m_cellSize.width(), m_rows * m_cellSize.height() );
	painter->drawTiledPixmap( gridArea, Render::renderElement( "cell", m_cellSize ) );
}


void Killbots::Scene::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
	getMouseDirection( event );
	QGraphicsScene::mouseMoveEvent( event );
}


void Killbots::Scene::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
	HeroAction actionFromPosition = getMouseDirection( event );

	if ( actionFromPosition != NoAction )
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
			emit clicked( actionFromPosition );
		else if ( userAction == Settings::RepeatedStep )
			emit clicked( -actionFromPosition - 1 );
		else if ( userAction == Settings::Teleport )
			emit clicked( Teleport );
		else if ( userAction == Settings::TeleportSafely )
			emit clicked( TeleportSafely );
		else if ( userAction == Settings::TeleportSafelyIfPossible )
			emit clicked( TeleportSafelyIfPossible );
		else if ( userAction == Settings::WaitOutRound )
			emit clicked( WaitOutRound );
	}

	QGraphicsScene::mouseReleaseEvent( event );
}


Killbots::HeroAction Killbots::Scene::getMouseDirection( QGraphicsSceneMouseEvent * event )
{
	HeroAction result;
	QPointF cursorPosition = event->scenePos();

	bool heroOnScreen = m_hero && sceneRect().contains( m_hero->sceneBoundingRect() );

	bool popupUnderCursor = m_queuedPopup->sceneBoundingRect().contains( cursorPosition )
	                        || m_unqueuedPopup->sceneBoundingRect().contains( cursorPosition );

	if ( heroOnScreen && !popupUnderCursor )
	{
		if ( m_hero->sceneBoundingRect().contains( cursorPosition ) )
			result = Hold;
		else
		{
			const qreal piOver4 = 0.78539816339744830961566L;

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
		result = NoAction;
	}

	return result;
}


void Killbots::Scene::updateSpritePos( Sprite * sprite ) const
{
	sprite->setPos( QPointF( sprite->gridPos().x() * m_cellSize.width(), sprite->gridPos().y() * m_cellSize.height() ) );
}

#include "moc_scene.cpp"
