#ifndef QTEST_ENGINETEST_H
#define QTEST_ENGINETEST_H

#include "coordinator.h"
#include "engine.h"
#include "scene.h"

#include "qtest.h"

#include <QtCore/QObject>

namespace Killbots
{
class EngineTest : public QObject
{
    Q_OBJECT

public:
    EngineTest(QObject *parent = 0);
    ~EngineTest();

private slots:
    void testValidCells_data();
    void testValidCells();

    void testRandomEmptyCell_data();
    void testRandomEmptyCell();

    void testMoveIsValid_data();
    void testMoveIsValid();

    void testMoveIsSafe_data();
    void testMoveIsSafe();

private:
    Coordinator *m_coordinator;
    Engine *m_engine;
    Scene *m_scene;
};
}
#endif

