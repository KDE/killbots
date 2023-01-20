set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(enginetest)
target_include_directories(enginetest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_source(enginetest PRIVATE
    enginetest.cpp enginetest.h
    ../coordinator.cpp
    ../engine.cpp
    ../numericdisplayitem.cpp
    ../renderer.cpp
    ../ruleset.cpp
    ../scene.cpp
    ../sprite.cpp
)

kconfig_add_kcfg_files(enginetest ../settings.kcfgc ../rulesetbase.kcfgc)

target_link_libraries(enginetest
    KF5KDEGames
    KF5::KDELibs4Support
    Qt::Test
)

add_test(NAME enginetest COMMAND enginetest)
ecm_mark_as_test(enginetest)
