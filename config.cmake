chcore_config(CHCORE_CROSS_COMPILE STRING "" "Prefix for cross compiling toolchain")
chcore_config(CHCORE_PLAT STRING "" "Target hardware platform")
chcore_config(CHCORE_VERBOSE_BUILD BOOL OFF "Generate verbose build log?")
chcore_config(CHCORE_ROOT_PROGRAM STRING "procm.srv" "First user program to run")

chcore_config_include(kernel/config.cmake)
chcore_config_include(user/config.cmake)
