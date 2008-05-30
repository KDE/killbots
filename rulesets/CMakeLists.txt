SET( ruleset_files
	classic.desktop
	default.desktop
	dense.desktop
	easy.desktop
	full.desktop
)

INSTALL( FILES ${ruleset_files}  DESTINATION ${DATA_INSTALL_DIR}/killbots/rulesets )