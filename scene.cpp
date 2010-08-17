/*
 *  Copyright 2007-2009  Parker Coates <parker.coates@kdemail.net>
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

#include "numericdisplayitem.h"
#include "renderer.h"
#include "settings.h"

#include <KGamePopupItem>

#include <KDE/KDebug>

#include <QtGui/QPainter>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>

#include <cmath>


Killbots::Scene::Scene( QObject * parent )
  : QGraphicsScene( parent ),
    m_hero( 0 ),
    m_rows( 0 ),
    m_columns( 0 )
{
	setItemIndexMethod( QGraphicsScene::NoIndex );
}


Killbots::Scene::~Scene()
{
}


void Killbots::Scene::addNumericDisplay( NumericDisplayItem * displayItem )
{
	addItem( displayItem );
	displayItem->setPos( -1000000, 0 );
	m_numericDisplays << displayItem;
}


void Killbots::Scene::setGridSize(int rows, int columns)
{
	if ( m_rows != rows || m_columns != columns )
	{
		m_rows = rows;
		m_columns = columns;
	}
}


void Killbots::Scene::forgetHero()
{
	m_hero = 0;
}


Killbots::Sprite * Killbots::Scene::createSprite( SpriteType type, QPoint position )
{
	Sprite * sprite = new Sprite();
	sprite->setSpriteType( type );
	sprite->setRenderSize( m_cellSize );
	sprite->enqueueGridPos( position );
	updateSpritePos( sprite, position );
	sprite->scale( 0, 0 );
	// A bit of a hack, but we use the sprite type for stacking order.
	sprite->setZValue( type );


	addItem( sprite );

	if ( type == Hero )
		m_hero = sprite;

	return sprite;
}


void Killbots::Scene::animateSprites( const QList<Sprite *> & newSprites,
                                      const QList<Sprite *> & slidingSprites,
                                      const QList<Sprite *> & teleportingSprites,
                                      const QList<Sprite *> & destroyedSprites,
                                      qreal value
                                    ) const
{
	static bool halfDone = false;

	if ( value == 0.0 )
	{
		halfDone = false;
	}
	else if ( value < 1.0 )
	{
		foreach ( Sprite * sprite, newSprites )
		{
			sprite->resetTransform();
			sprite->scale( value, value );
		}

		foreach ( Sprite * sprite, slidingSprites )
		{
			QPointF posInGridCoordinates = value * QPointF( sprite->nextGridPos() - sprite->currentGridPos() ) + sprite->currentGridPos();
			sprite->setPos( QPointF( posInGridCoordinates.x() * m_cellSize.width(), posInGridCoordinates.y() * m_cellSize.height() ) );
		}

		qreal scaleFactor = value < 0.5
		                  ? 1.0 - 2 * value
		                  : 2 * value - 1.0;

		if ( !halfDone && value >= 0.5 )
		{
			halfDone = true;
			foreach ( Sprite * sprite, teleportingSprites )
				updateSpritePos( sprite, sprite->nextGridPos() );
		}

		foreach ( Sprite * sprite, teleportingSprites )
		{
			sprite->resetTransform();
			sprite->scale( scaleFactor, scaleFactor );
		}

		foreach ( Sprite * sprite, destroyedSprites )
		{
			sprite->resetTransform();
			sprite->scale( 1 - value, 1 - value );
			sprite->rotate( value * 180 );
		}
	}
	else
	{
		foreach ( Sprite * sprite, newSprites + slidingSprites + teleportingSprites )
		{
			sprite->resetTransform();
			sprite->advanceGridPosQueue();
			updateSpritePos( sprite, sprite->currentGridPos() );
		}

		qDeleteAll( destroyedSprites );
	}
}


void Killbots::Scene::doLayout()
{
	QSize size = views().first()->size();

	// If no game has been started
	if ( m_rows == 0 || m_columns == 0 )
	{
		setSceneRect( QRectF( QPointF( 0, 0 ), size ) );
		return;
	}

	kDebug() << "Laying out scene at" << size;

	// Make certain layout properties proportional to the scene height,
	// but clamp them between reasonable values. There's probably room for more
	// tweaking here.
	const int baseDimension = qMin( size.width(), size.height() ) / 35;
	const int spacing = qBound( 5, baseDimension, 15 );
	const int newFontPixelSize = qBound( QFontInfo( QFont() ).pixelSize(), baseDimension, 25 );
	const qreal aspectRatio = Renderer::self()->aspectRatio();

	QSize displaySize;
	// If the font size has changed, resize all the displays (visible or not).
	if ( m_numericDisplays.first()->font().pixelSize() != newFontPixelSize )
	{
		QFont font;
		font.setPixelSize( newFontPixelSize  );

		foreach ( NumericDisplayItem * display, m_numericDisplays )
		{
			display->setFont( font );
			displaySize = displaySize.expandedTo( display->preferredSize() );
		}
		foreach ( NumericDisplayItem * display, m_numericDisplays )
			display->setRenderSize( displaySize );
	}
	else
	{
		displaySize = m_numericDisplays.first()->boundingRect().size().toSize();
	}

	// The rest of the function deals only with a list of visible displays.
	QList<NumericDisplayItem *> visibleDisplays;
	foreach ( NumericDisplayItem * display, m_numericDisplays )
	{
		if ( display->isVisible() )
			visibleDisplays << display;
	}

	// Determine the total width required to arrange the displays horizontally.
	const int widthOfDisplaysOnTop = visibleDisplays.size() * displaySize.width()
	                           + ( visibleDisplays.size() - 1 ) * spacing;

	// The displays can either be placed centred, across the top of the
	// scene or top-aligned, down the side of the scene. We first calculate
	// what the cell size would be for both options.
	int availableWidth = size.width() - 3 * spacing - displaySize.width();
	int availableHeight = size.height() - 2 * spacing;
	const qreal cellWidthSide = ( availableWidth / m_columns < availableHeight / m_rows * aspectRatio )
	                          ? availableWidth / m_columns
	                          : availableHeight / m_rows * aspectRatio;

	availableWidth = size.width() - 2 * spacing;
	availableHeight = size.height() - 3 * spacing - displaySize.height();
	const qreal cellWidthTop = ( availableWidth / m_columns < availableHeight / m_rows * aspectRatio )
	                         ? availableWidth / m_columns
	                         : availableHeight / m_rows * aspectRatio;

	// If placing the displays on top would result in larger cells, we take
	// that option, but only if the displays would actually fit.
	const bool displaysOnTop = ( cellWidthTop > cellWidthSide && size.width() > widthOfDisplaysOnTop );
	const qreal newCellWidth = displaysOnTop ? cellWidthTop : cellWidthSide;
	m_cellSize = QSize( qRound( newCellWidth ), qRound( newCellWidth / aspectRatio ) );

	foreach ( QGraphicsItem * item, items() )
	{
		Sprite * sprite = qgraphicsitem_cast<Sprite *>( item );
		if ( sprite )
		{
			sprite->setRenderSize( m_cellSize );
			updateSpritePos( sprite, sprite->currentGridPos() );
		}
	}

	if ( displaysOnTop )
	{
		// Set the sceneRect to centre the grid if possible, but ensure the display items are visible
		const qreal sceneRectXPos = -( size.width() - m_cellSize.width() * ( m_columns - 1 ) ) / 2.0;
		const qreal centeredYPos = - ( size.height() - m_cellSize.height() * ( m_rows - 1 ) ) / 2.0;
		const qreal indentedYPos = - ( m_cellSize.height() / 2.0 + 2 * spacing + displaySize.height() );
		const qreal sceneRectYPos = qMin( centeredYPos, indentedYPos );

		// Position the display items centered at the top of the scene
		const qreal displayYPos = ( sceneRectYPos - ( displaySize.height() + m_cellSize.height() / 2.0 ) ) / 2;

		int xPos = sceneRectXPos + ( size.width() - widthOfDisplaysOnTop ) / 2.0;
		foreach ( NumericDisplayItem * display, visibleDisplays )
		{
			display->setPos( xPos, displayYPos );
			xPos += displaySize.width() + spacing;
		}

		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );
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
		foreach ( NumericDisplayItem * display, visibleDisplays )
		{
			display->setPos( displayXPos, yPos );
			yPos += displaySize.height() + spacing;
		}

		setSceneRect( QRectF( sceneRectXPos, sceneRectYPos, size.width(), size.height() ) );
	}

	// Update the scene background
	QPainter p;
	QRect gridRect( -sceneRect().x() - m_cellSize.width() / 2,
	                -sceneRect().y() - m_cellSize.height() / 2,
	                m_columns * m_cellSize.width(),
	                m_rows * m_cellSize.height()
	              );

	QPixmap unrotated = Renderer::self()->spritePixmap( "background", size );
	p.begin( &unrotated );
	p.drawTiledPixmap( gridRect, Renderer::self()->spritePixmap( "cell", m_cellSize ) );
	p.end();

	// The background brush begins painting at 0,0 but our sceneRect doesn't
	// start at 0,0 so we have to "rotate" the pixmap so that it looks right
	// when tiled.
	QPixmap background( size );
	background.fill( Qt::transparent );
	p.begin( &background );
	p.drawTiledPixmap( background.rect(), unrotated, -sceneRect().topLeft().toPoint() );
	p.end();

	setBackgroundBrush( background );

	update();
}


void Killbots::Scene::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
	getMouseDirection( event->scenePos() );
	QGraphicsScene::mouseMoveEvent( event );
}


void Killbots::Scene::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
	HeroAction actionFromPosition = getMouseDirection( event->scenePos() );

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


Killbots::HeroAction Killbots::Scene::getMouseDirection( QPointF cursorPosition ) const
{
	HeroAction result;
	const bool heroOnScreen = m_hero && sceneRect().contains( m_hero->sceneBoundingRect() );

	if ( heroOnScreen && !popupAtPosition( cursorPosition ) )
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

		views().first()->setCursor( Renderer::self()->cursorFromAction( result ) );
	}
	else
	{
		views().first()->unsetCursor();
		result = NoAction;
	}

	return result;
}


bool Killbots::Scene::popupAtPosition( QPointF position ) const
{
	foreach ( QGraphicsItem * item, items( position ) )
	{
		if ( dynamic_cast<KGamePopupItem *>( item ) != 0 )
			return true;
	}
	return false;
}


void Killbots::Scene::updateSpritePos( Sprite * sprite, QPoint gridPosition ) const
{
	sprite->setPos( gridPosition.x() * m_cellSize.width(), gridPosition.y() * m_cellSize.height() );
}

#include "moc_scene.cpp"
