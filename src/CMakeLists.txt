ecm_setup_version(${RELEASE_SERVICE_VERSION}
    VARIABLE_PREFIX KILLBOTS
    VERSION_HEADER killbots_version.h
)

add_executable(killbots)

target_sources(killbots PRIVATE
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
    KF5::XmlGui
    KF5::DBusAddons
    KF5::Completion
    KF5::Crash
    KF5::I18n
    Qt5::Widgets
    Qt5::Xml
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
