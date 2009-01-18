project( killbots )

###### Switching to Subdirectories ######
add_subdirectory( icons )
add_subdirectory( rulesets )
add_subdirectory( themes )

###### Building killbots executable ######
set( killbots_SRCS
    engine.cpp
    gamestatusdisplayitem.cpp
    mainwindow.cpp
    main.cpp
    optionspage.cpp
    render.cpp
    ruleset.cpp
    rulesetdetailsdialog.cpp
    rulesetselector.cpp
    scene.cpp
    sprite.cpp
    view.cpp
)
kde4_add_kcfg_files( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
kde4_add_executable( killbots ${killbots_SRCS} )
target_link_libraries( killbots ${KDE4_KDEUI_LIBS} kdegames ${QT_QTXML_LIBRARY})

###### Installing Files ######
install( TARGETS killbots ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES killbotsui.rc DESTINATION ${DATA_INSTALL_DIR}/killbots )
install( FILES killbots.kcfg DESTINATION ${KCFG_INSTALL_DIR} )



