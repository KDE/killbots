###### Installing Killbots Rulesets ######
SET( ruleset_files
    classic.desktop
    daleks.desktop
    default.desktop
    easy.desktop
)

install( FILES ${ruleset_files} DESTINATION ${DATA_INSTALL_DIR}/killbots/rulesets )
