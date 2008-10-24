###### Installing Killbots Rulesets ######
file( GLOB ruleset_files *.desktop )
install( FILES ${ruleset_files} DESTINATION ${DATA_INSTALL_DIR}/killbots/rulesets )
