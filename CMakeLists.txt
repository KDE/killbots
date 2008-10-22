PROJECT( killbots )


###### Compile Time Options ######
OPTION( WITH_OPENGL "Use OpenGL for game area" FALSE )


###### Building the Killbots Executable ######
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDE_DIR} )
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/config-killbots.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-killbots.h )
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
KDE4_ADD_KCFG_FILES( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
KDE4_ADD_EXECUTABLE( killbots ${killbots_SRCS} )
SET( killbots_LINKLIBRARIES
	${KDE4_KDEUI_LIBS}
	kdegames
)
IF( WITH_OPENGL AND OPENGL_SUPPORT )
	SET( killbots_LINKLIBRARIES ${killbots_LINKLIBRARIES} ${QT_QTOPENGL_LIBRARY} )
ENDIF( WITH_OPENGL AND OPENGL_SUPPORT )
TARGET_LINK_LIBRARIES( killbots ${killbots_LINKLIBRARIES} )


###### Installing Files ######
INSTALL( TARGETS killbots ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL( FILES killbotsui.rc DESTINATION ${DATA_INSTALL_DIR}/killbots )
INSTALL( FILES killbots.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


###### Switching to Subdirectories #####
ADD_SUBDIRECTORY( icons )
ADD_SUBDIRECTORY( rulesets )
ADD_SUBDIRECTORY( themes )
