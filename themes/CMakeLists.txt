###### Installing Themes ######
set( theme_files
    classic.desktop
    classic.png
    classic.svgz
    mountainadventure.desktop
    mountainadventure.png
    mountainadventure.svgz
    mummymadness.desktop
    mummymadness.png
    mummymadness.svgz
    robotkill.desktop
    robotkill.png
    robotkill.svgz
)

install( FILES ${theme_files} DESTINATION ${DATA_INSTALL_DIR}/killbots/themes )
