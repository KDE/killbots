PROJECT( killbots )

ADD_SUBDIRECTORY( cursors )
ADD_SUBDIRECTORY( rulesets )
ADD_SUBDIRECTORY( themes )

FIND_PACKAGE( KDE4 REQUIRED )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR}  )

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

KDE4_ADD_KCFG_FILES( killbots_SRCS settings.kcfgc )

KDE4_ADD_EXECUTABLE( killbots ${killbots_SRCS} )

TARGET_LINK_LIBRARIES( killbots ${KDE4_KDEUI_LIBS} ${KDEGAMES_LIBRARY} )

INSTALL( TARGETS killbots DESTINATION ${BIN_INSTALL_DIR} )

INSTALL( FILES killbotsui.rc DESTINATION ${DATA_INSTALL_DIR}/killbots )

INSTALL( FILES settings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )

INSTALL( FILES killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


