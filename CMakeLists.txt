project( killbots )

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames 4.9.0 REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})
endif(NOT COMPILING_KDEGAMES)

###### Switching to Subdirectories ######
add_subdirectory( icons )
add_subdirectory( rulesets )
add_subdirectory( themes )
add_subdirectory( tests )
add_subdirectory( doc )

###### Building killbots executable ######
set( killbots_SRCS
    main.cpp
    coordinator.cpp
    engine.cpp
    mainwindow.cpp
    numericdisplayitem.cpp
    optionspage.cpp
    renderer.cpp
    ruleset.cpp
    rulesetdetailsdialog.cpp
    rulesetselector.cpp
    scene.cpp
    sprite.cpp
    view.cpp
)
kde4_add_kcfg_files( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
kde4_add_app_icon( killbots_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-killbots.png"})
kde4_add_executable( killbots ${killbots_SRCS} )
target_link_libraries( killbots ${KDE4_KDEUI_LIBS} kdegames ${QT_QTXML_LIBRARY})

###### Installing Files ######
install( TARGETS killbots ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES killbotsui.rc DESTINATION ${DATA_INSTALL_DIR}/killbots )
install( FILES killbots.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
