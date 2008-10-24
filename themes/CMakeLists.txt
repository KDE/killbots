###### Installing Themes ######
file( GLOB theme_files *.desktop *.svg *.png )
install( FILES ${theme_files} DESTINATION ${DATA_INSTALL_DIR}/killbots/themes )