FILE(GLOB ruleset_files *.desktop)
INSTALL( FILES ${ruleset_files}  DESTINATION ${DATA_INSTALL_DIR}/killbots/rulesets )
