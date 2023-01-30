add_executable(procm.srv main.c elf.c launch.c spawn.c)

macro(_incbin _binary_name _dep_target _binary_path)
    set(binary_name ${_binary_name})
    set(binary_path ${_binary_path})
    configure_file(incbin.tpl.S incbin_${_binary_name}.S)
    unset(binary_name)
    unset(binary_path)
    target_sources(procm.srv PRIVATE incbin_${_binary_name}.S)
    add_dependencies(procm.srv ${_dep_target})
endmacro()

_incbin(tmpfs_elf tmpfs.srv ${CMAKE_CURRENT_BINARY_DIR}/../tmpfs/tmpfs.srv)
_incbin(shell_elf shell.srv ${CMAKE_CURRENT_BINARY_DIR}/../shell/shell.srv)

chcore_install_target(procm.srv)
