ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX KILLBOTS
    VERSION_HEADER killbots_version.h
)

add_executable(killbots)

target_sources(killbots PRIVATE
    actions.h
    coordinator.cpp
    coordinator.h
    engine.cpp
    engine.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    numericdisplayitem.cpp
    numericdisplayitem.h
    optionspage.cpp
    optionspage.h
    renderer.cpp
    renderer.h
    ruleset.cpp
    rulesetdetailsdialog.cpp
    rulesetdetailsdialog.h
    ruleset.h
    rulesetselector.cpp
    rulesetselector.h
    scene.cpp
    scene.h
    sprite.cpp
    sprite.h
    view.cpp
    view.h

    killbots.qrc
)

ecm_qt_declare_logging_category(killbots
    HEADER killbots_debug.h
    IDENTIFIER KILLBOTS_LOG
    CATEGORY_NAME org.kde.kdegames.killbots
    DESCRIPTION "Killbots"
    EXPORT KILLBOTS
)

kconfig_add_kcfg_files(killbots settings.kcfgc rulesetbase.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/icons/*-apps-killbots.png")
ecm_add_app_icon(killbots ICONS ${ICONS_SRCS})

target_link_libraries(killbots
    KF5KDEGames
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::XmlGui
    KF${KF_MAJOR_VERSION}::DBusAddons
    KF${KF_MAJOR_VERSION}::Completion
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::I18n
    Qt::Widgets
    Qt::Xml
)

install(TARGETS killbots ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.killbots.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.killbots.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR})
install(FILES killbots.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_qt_install_logging_categories(
    EXPORT KILLBOTS
    FILE killbots.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
