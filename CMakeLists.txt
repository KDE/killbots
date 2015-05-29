project( killbots )

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.3.0")

find_package(ECM 1.3.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Test)
find_package(KF5 REQUIRED COMPONENTS 
    CoreAddons 
    Config 
    WidgetsAddons 
    Config 
    I18n 
    GuiAddons 
    ConfigWidgets 
    ItemViews 
    IconThemes 
    XmlGui
    KIO
    Completion
    NotifyConfig
    )
    
find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMAddTests)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
###### Switching to Subdirectories ######
add_subdirectory( icons )
add_subdirectory( rulesets )
add_subdirectory( themes )
#Reactivate
#add_subdirectory( tests )
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
kconfig_add_kcfg_files( killbots_SRCS settings.kcfgc rulesetbase.kcfgc )
#kde4_add_app_icon( killbots_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-apps-killbots.png")
add_executable( killbots ${killbots_SRCS} )
target_link_libraries( killbots  KF5KDEGames Qt5::Xml KF5::XmlGui Qt5::Widgets KF5::DBusAddons KF5::Completion KF5::I18n)

###### Installing Files ######
install( TARGETS killbots ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS org.kde.killbots.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES killbotsui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/killbots )
install( FILES killbots.kcfg DESTINATION ${KCFG_INSTALL_DIR} )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
