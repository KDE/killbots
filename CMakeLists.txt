cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "20")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(killbots VERSION ${RELEASE_SERVICE_VERSION})

set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.30.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    Config
    Crash
    WidgetsAddons
    Config
    I18n
    ConfigWidgets
    XmlGui
    Completion
    DocTools
    DBusAddons
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMAddTests)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)

add_definitions(
    -DQT_NO_FOREACH
)

###### Switching to Subdirectories ######
add_subdirectory(icons)
add_subdirectory(rulesets)
add_subdirectory(themes)
#Reactivate
#add_subdirectory(tests)
add_subdirectory(doc)

###### Building killbots executable ######
set(killbots_SRCS
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
ecm_qt_declare_logging_category(killbots_SRCS HEADER killbots_debug.h IDENTIFIER KILLBOTS_LOG CATEGORY_NAME org.kde.kdegames.killbots)
qt5_add_resources(killbots_SRCS killbots.qrc)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KILLBOTS VERSION_HEADER killbots_version.h)

kconfig_add_kcfg_files( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-killbots.png")
ecm_add_app_icon(killbots_SRCS ICONS ${ICONS_SRCS})
add_executable(killbots ${killbots_SRCS})
target_link_libraries(killbots
    KF5KDEGames
    Qt5::Xml
    KF5::XmlGui
    Qt5::Widgets
    KF5::DBusAddons
    KF5::Completion
    KF5::Crash
    KF5::I18n
)

###### Installing Files ######
install(TARGETS killbots ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.killbots.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.killbots.appdata.xml DESTINATION  ${KDE_INSTALL_METAINFODIR})
install(FILES killbots.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES killbots.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR} )
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
