###### Installing Themes ######
SET( theme_files
    classic.desktop
    classic.png
    classic.svg
    default.desktop
    default.png
    default.svg
    mountainadventure.desktop
    mountainadventure.png
    mountainadventure.svg
    olddefault.svg
    olddefault.desktop
    olddefault.png
)

install( FILES ${theme_files} DESTINATION ${DATA_INSTALL_DIR}/killbots/themes )