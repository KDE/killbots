set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

set( enginetest_SRCS
    enginetest.cpp
    ../coordinator.cpp
    ../engine.cpp
    ../numericdisplayitem.cpp
    ../render.cpp
    ../ruleset.cpp
    ../scene.cpp
    ../sprite.cpp
)
kde4_add_kcfg_files( enginetest_SRCS ../settings.kcfgc ../rulesetbase.kcfgc )
kde4_add_unit_test( enginetest ${enginetest_SRCS} )
target_link_libraries( enginetest
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    kdegames
)
