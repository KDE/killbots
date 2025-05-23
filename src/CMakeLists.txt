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
    CATEGORY_NAME killbots
    OLD_CATEGORY_NAMES org.kde.kdegames.killbots
    DESCRIPTION "Killbots game"
    EXPORT KILLBOTS
)

if(APPLE)
    set_target_properties(killbots PROPERTIES
           MACOSX_BUNDLE_DISPLAY_NAME "Killbots"
        MACOSX_BUNDLE_BUNDLE_NAME "Killbots"
        MACOSX_BUNDLE_LONG_VERSION_STRING "Killbots ${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.killbots"
        MACOSX_BUNDLE_COPYRIGHT "2024 The Killbots Developers")
endif()


kconfig_add_kcfg_files(killbots settings.kcfgc rulesetbase.kcfgc)

file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/icons/*-apps-killbots.png")
ecm_add_app_icon(killbots ICONS ${ICONS_SRCS})

target_link_libraries(killbots
    KDEGames6
    KF6::CoreAddons
    KF6::XmlGui
    KF6::DBusAddons
    KF6::Completion
    KF6::Crash
    KF6::I18n
    KF6::IconThemes
    Qt6::Widgets
    Qt6::Xml
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
