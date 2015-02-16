#include "enginetest.h"

#include "renderer.h"

#include <QDebug>
#include <KGlobal>
#include <KStandardDirs>

#include <QtTest>

Killbots::EngineTest::EngineTest( QObject * parent )
  : QObject( parent )
{
	KGlobal::dirs()->addResourceDir("ruleset", KDESRCDIR "/../rulesets/");
	KGlobal::dirs()->addResourceDir("appdata", KDESRCDIR "/../");

	m_coordinator = new Coordinator( this );
	m_coordinator->setAnimationSpeed( 0 );

	m_engine = new Engine( m_coordinator, this );
	m_engine->setRuleset( Ruleset::load( "default.desktop" ) );
	m_coordinator->setEngine( m_engine );

	m_scene = new Scene( this );
	m_coordinator->setScene( m_scene );
}

Killbots::EngineTest::~EngineTest()
{
	Killbots::Renderer::cleanup();
}

void Killbots::EngineTest::testValidCells_data()
{
	QTest::addColumn<int>("row");
	QTest::addColumn<int>("column");
	QTest::addColumn<bool>("valid");

	int rowCount = m_engine->ruleset()->rows();
	int columnCount = m_engine->ruleset()->columns();

	for ( int r = 0; r < rowCount; ++r )
	{
		QTest::newRow("Cell to left of grid") << r << -1 << false;
		for ( int c = 0; c < columnCount; ++c )
		{
			const QString description = QString("Row %1. column %2").arg(r).arg(c);
			QTest::newRow("Cell inside grid") << r << c << true;
		}
		QTest::newRow("Cell to right of grid") << r << columnCount << false;
	}

	for ( int c = -1; c <= columnCount; ++c )
	{
		QTest::newRow("Cell above grid") << -1 << c << false;
		QTest::newRow("Cell below grid") << rowCount << c << false;
	}
}


void Killbots::EngineTest::testValidCells()
{
	QFETCH( int, row );
	QFETCH( int, column );

	QTEST( m_engine->cellIsValid( QPoint( column, row ) ), "valid" );
}


void Killbots::EngineTest::testRandomEmptyCell_data()
{
	m_engine->startNewGame();
}


void Killbots::EngineTest::testRandomEmptyCell()
{
	for ( int i = 0; i < 100; ++i )
	{
		QPoint point = m_engine->randomEmptyCell();
		QVERIFY( m_engine->cellIsValid( point ) );
		QVERIFY( m_engine->spriteTypeAt( point ) == NoSprite );
	}
}


void Killbots::EngineTest::testMoveIsValid_data()
{
	                //0123456789012345
	QString layout = "  j             \n" // 0
	                 " r      j     jj\n" // 1
	                 "  f    j        \n" // 2
	                 "rjjj            \n" // 3
	                 "                \n" // 4
	                 "                \n" // 5
	                 "                \n" // 6
	                 "                \n" // 7
	                 "                \n" // 8
	                 "                \n" // 9
	                 "                \n" //10
	                 "                \n" //11
	                 "                \n" //12
	                 "                \n" //13
	                 "                \n" //14
	                 "                ";  //15
	m_engine->startNewRound( false, layout );
	const Ruleset * rules = m_engine->ruleset();

	QTest::addColumn<int>("row");
	QTest::addColumn<int>("column");
	QTest::addColumn<int>("action");
	QTest::addColumn<bool>("valid");

	QTest::newRow("Leaving the grid by the corner") << -1 <<  -1 << int(UpRight)  << false;
	QTest::newRow("Leaving the grid by the edge")   << -1 <<   8 << int(Up)       << false;
	QTest::newRow("Leaving the grid by teleport")   << 25 << -12 << int(Teleport) << false;

	QTest::newRow("Moving onto a robot")   << 1 << 1 << int(Right) << false;
	QTest::newRow("Moving onto a fastbot") << 2 << 2 << int(Down)  << false;

	QTest::newRow("Pushing a junkheap")                    << 0 <<  2 << int(Left)     << (rules->pushableJunkheaps() != Ruleset::None);
	QTest::newRow("Pushing two junkheaps")                 << 1 <<  8 << int(DownLeft) << (rules->pushableJunkheaps() == Ruleset::Many);
	QTest::newRow("Pushing a junkheap out of the grid")    << 0 <<  2 << int(Up)       << false;
	QTest::newRow("Pushing two junkheaps out of the grid") << 1 << 14 << int(Right)    << false;
	QTest::newRow("Pushing a junkheap onto a fastbot")     << 3 <<  2 << int(Up)       << (rules->pushableJunkheaps() != Ruleset::None && rules->squaskKillsEnabled());
	QTest::newRow("Pushing three junkheaps onto a robot")  << 3 <<  3 << int(Left)     << (rules->pushableJunkheaps() == Ruleset::Many && rules->squaskKillsEnabled());
}


void Killbots::EngineTest::testMoveIsValid()
{
	QFETCH( int, row );
	QFETCH( int, column );
	QFETCH( int, action );

	QTEST( m_engine->moveIsValid( QPoint( column, row ), HeroAction(action) ), "valid" );
}


void Killbots::EngineTest::testMoveIsSafe_data()
{
	                //0123456789012345
	QString layout = "----- -------   \n" // 0
	                 "-!!!- -!!!!!-   \n" // 1
	                 "-!r!- -!!!!!-   \n" // 2
	                 "-!!!- -!!f!!-   \n" // 3
	                 "----- -!!!!!-   \n" // 4
	                 "      -!!!!!-   \n" // 5
	                 "      -------   \n" // 6
	                 "frj    j        \n" // 7
	                 "      r         \n" // 8
	                 "     f     j    \n" // 9
	                 "                \n" //10
	                 " f f  ff  f     \n" //11
	                 "r   r  jjj      \n" //12
	                 "  - f fj-j      \n" //13
	                 "f   r  jjjf     \n" //14
	                 " rf   f   f     ";  //15
	m_engine->startNewRound( false, layout );

	QTest::addColumn<int>("row");
	QTest::addColumn<int>("column");
	QTest::addColumn<int>("action");
	QTest::addColumn<bool>("safe");

	// Proximity of a robot
	QTest::newRow("Cell above above right right of robot") << 0 << 0 << int(Hold) << true;
	QTest::newRow("Cell above above right of robot")       << 0 << 1 << int(Hold) << true;
	QTest::newRow("Cell above above robot")                << 0 << 2 << int(Hold) << true;
	QTest::newRow("Cell above above left of robot")        << 0 << 3 << int(Hold) << true;
	QTest::newRow("Cell above above left left of robot")   << 0 << 4 << int(Hold) << true;
	QTest::newRow("Cell above right right of robot")       << 1 << 0 << int(Hold) << true;
	QTest::newRow("Cell above right of robot")             << 1 << 1 << int(Hold) << false;
	QTest::newRow("Cell above robot")                      << 1 << 2 << int(Hold) << false;
	QTest::newRow("Cell above left of robot")              << 1 << 3 << int(Hold) << false;
	QTest::newRow("Cell above left left of robot")         << 1 << 4 << int(Hold) << true;
	QTest::newRow("Cell right right of robot")             << 2 << 0 << int(Hold) << true;
	QTest::newRow("Cell right of robot")                   << 2 << 1 << int(Hold) << false;
	QTest::newRow("Cell left of robot")                    << 2 << 3 << int(Hold) << false;
	QTest::newRow("Cell left left robot")                  << 2 << 4 << int(Hold) << true;
	QTest::newRow("Cell below right right of robot")       << 3 << 0 << int(Hold) << true;
	QTest::newRow("Cell below right of robot")             << 3 << 1 << int(Hold) << false;
	QTest::newRow("Cell below robot")                      << 3 << 2 << int(Hold) << false;
	QTest::newRow("Cell below left of robot")              << 3 << 3 << int(Hold) << false;
	QTest::newRow("Cell below left left of robot")         << 3 << 4 << int(Hold) << true;
	QTest::newRow("Cell below below right right of robot") << 4 << 0 << int(Hold) << true;
	QTest::newRow("Cell below below right of robot")       << 4 << 1 << int(Hold) << true;
	QTest::newRow("Cell below below robot")                << 4 << 2 << int(Hold) << true;
	QTest::newRow("Cell below below left of robot")        << 4 << 3 << int(Hold) << true;
	QTest::newRow("Cell below below left left of robot")   << 4 << 4 << int(Hold) << true;

	// Proximity of a fastbot
	QTest::newRow("Cell above above above right right right of fastbot") << 0 <<  6 << int(Hold) << true;
	QTest::newRow("Cell above above above right right of fastbot")       << 0 <<  7 << int(Hold) << true;
	QTest::newRow("Cell above above above right of fastbot")             << 0 <<  8 << int(Hold) << true;
	QTest::newRow("Cell above above above fastbot")                      << 0 <<  9 << int(Hold) << true;
	QTest::newRow("Cell above above above left of fastbot")              << 0 << 10 << int(Hold) << true;
	QTest::newRow("Cell above above above left left of fastbot")         << 0 << 11 << int(Hold) << true;
	QTest::newRow("Cell above above above left left left of fastbot")    << 0 << 12 << int(Hold) << true;
	QTest::newRow("Cell above above right right right of fastbot")       << 1 <<  6 << int(Hold) << true;
	QTest::newRow("Cell above above right right of fastbot")             << 1 <<  7 << int(Hold) << false;
	QTest::newRow("Cell above above right of fastbot")                   << 1 <<  8 << int(Hold) << false;
	QTest::newRow("Cell above above fastbot")                            << 1 <<  9 << int(Hold) << false;
	QTest::newRow("Cell above above left of fastbot")                    << 1 << 10 << int(Hold) << false;
	QTest::newRow("Cell above above left left of fastbot")               << 1 << 11 << int(Hold) << false;
	QTest::newRow("Cell above above left left left of fastbot")          << 1 << 12 << int(Hold) << true;
	QTest::newRow("Cell above right right right of fastbot")             << 2 <<  6 << int(Hold) << true;
	QTest::newRow("Cell above right right of fastbot")                   << 2 <<  7 << int(Hold) << false;
	QTest::newRow("Cell above right of fastbot")                         << 2 <<  8 << int(Hold) << false;
	QTest::newRow("Cell above fastbot")                                  << 2 <<  9 << int(Hold) << false;
	QTest::newRow("Cell above left of fastbot")                          << 2 << 10 << int(Hold) << false;
	QTest::newRow("Cell above left left of fastbot")                     << 2 << 11 << int(Hold) << false;
	QTest::newRow("Cell above left left left of fastbot")                << 2 << 12 << int(Hold) << true;
	QTest::newRow("Cell right right right of fastbot")                   << 3 <<  6 << int(Hold) << true;
	QTest::newRow("Cell right right of fastbot")                         << 3 <<  7 << int(Hold) << false;
	QTest::newRow("Cell right of fastbot")                               << 3 <<  8 << int(Hold) << false;
	QTest::newRow("Cell left of fastbot")                                << 3 << 10 << int(Hold) << false;
	QTest::newRow("Cell left left of fastbot")                           << 3 << 11 << int(Hold) << false;
	QTest::newRow("Cell left left left of fastbot")                      << 3 << 12 << int(Hold) << true;
	QTest::newRow("Cell below right right right of fastbot")             << 4 <<  6 << int(Hold) << true;
	QTest::newRow("Cell below right right of fastbot")                   << 4 <<  7 << int(Hold) << false;
	QTest::newRow("Cell below right of fastbot")                         << 4 <<  8 << int(Hold) << false;
	QTest::newRow("Cell below fastbot")                                  << 4 <<  9 << int(Hold) << false;
	QTest::newRow("Cell below left of fastbot")                          << 4 << 10 << int(Hold) << false;
	QTest::newRow("Cell below left left of fastbot")                     << 4 << 11 << int(Hold) << false;
	QTest::newRow("Cell below left left left of fastbot")                << 4 << 12 << int(Hold) << true;
	QTest::newRow("Cell below below right right right of fastbot")       << 5 <<  6 << int(Hold) << true;
	QTest::newRow("Cell below below right right of fastbot")             << 5 <<  7 << int(Hold) << false;
	QTest::newRow("Cell below below right of fastbot")                   << 5 <<  8 << int(Hold) << false;
	QTest::newRow("Cell below below fastbot")                            << 5 <<  9 << int(Hold) << false;
	QTest::newRow("Cell below below left of fastbot")                    << 5 << 10 << int(Hold) << false;
	QTest::newRow("Cell below below left left of fastbot")               << 5 << 11 << int(Hold) << false;
	QTest::newRow("Cell below below left left left of fastbot")          << 5 << 12 << int(Hold) << true;
	QTest::newRow("Cell below below below right right right of fastbot") << 6 <<  6 << int(Hold) << true;
	QTest::newRow("Cell below below below right right of fastbot")       << 6 <<  7 << int(Hold) << true;
	QTest::newRow("Cell below below below right of fastbot")             << 6 <<  8 << int(Hold) << true;
	QTest::newRow("Cell below below below fastbot")                      << 6 <<  9 << int(Hold) << true;
	QTest::newRow("Cell below below below left of fastbot")              << 6 << 10 << int(Hold) << true;
	QTest::newRow("Cell below below below left left of fastbot")         << 6 << 11 << int(Hold) << true;
	QTest::newRow("Cell below below below left left left of fastbot")    << 6 << 12 << int(Hold) << true;

	QTest::newRow("Cell with fastbots blocked by junkheaps") << 13 << 2 << int(Hold) << true;

	QTest::newRow("Cell with fastbots that will collide with other bots") << 13 << 8 << int(Hold) << true;

	QTest::newRow("Pushing junkheap horizontally as a blocker") << 7 <<  2 << int(Left)     << true;
	QTest::newRow("Pushing junkheap diagonally as a blocker")   << 7 <<  7 << int(DownLeft) << true;
	QTest::newRow("Pushing junkheap vertically as a blocker")   << 9 << 11 << int(Down)     << true;

	QTest::newRow("Pushing junkheap diagonally without blocking")   << 7 <<  2 << int(UpLeft)   << false;
	QTest::newRow("Pushing junkheap diagonally without blocking")   << 9 << 11 << int(DownLeft) << false;
	QTest::newRow("Pushing junkheap horizontally without blocking") << 7 <<  7 << int(Left)     << false;
	QTest::newRow("Pushing junkheap vertically without blocking")   << 7 <<  7 << int(Down)     << false;
}

void Killbots::EngineTest::testMoveIsSafe()
{
	QFETCH( int, row );
	QFETCH( int, column );
	QFETCH( int, action );

	QTEST( m_engine->moveIsSafe( QPoint( column, row ), HeroAction(action) ), "safe" );
}


QTEST_MAIN( Killbots::EngineTest )

#include "enginetest.moc"
