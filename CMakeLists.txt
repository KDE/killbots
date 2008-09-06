CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT( killbots )

FIND_PACKAGE( KDE4 REQUIRED )
FIND_PACKAGE( LibKDEGames REQUIRED )

INCLUDE( KDE4Defaults )

ADD_SUBDIRECTORY( rulesets )
ADD_SUBDIRECTORY( themes )

SET( killbots_SRCS
	engine.cpp
	gamestatusdisplayitem.cpp
	mainwindow.cpp
	main.cpp
	optionspage.cpp
	render.cpp
	ruleset.cpp
	rulesetselector.cpp
	scene.cpp
	sprite.cpp
	view.cpp
)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR} )
KDE4_ADD_KCFG_FILES( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
KDE4_ADD_EXECUTABLE( killbots ${killbots_SRCS} )
TARGET_LINK_LIBRARIES( killbots ${KDE4_KDEUI_LIBS} ${KDEGAMES_LIBRARY} )

INSTALL( TARGETS killbots ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES killbotsui.rc DESTINATION ${DATA_INSTALL_DIR}/killbots )
INSTALL( FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
INSTALL( FILES killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
