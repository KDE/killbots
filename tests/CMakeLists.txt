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
    ../renderer.cpp
    ../ruleset.cpp
    ../scene.cpp
    ../sprite.cpp
)
kconfig_add_kcfg_files( enginetest_SRCS ../settings.kcfgc ../rulesetbase.kcfgc )
add_executable( enginetest ${enginetest_SRCS} )
add_test(NAME enginetest COMMAND enginetest)
ecm_mark_as_test(enginetest)
target_link_libraries( enginetest
    KF5::KDELibs4Support
    Qt5::Test
    KF5KDEGames
)
