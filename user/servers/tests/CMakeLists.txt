add_executable(tests.bin main.c elf.c launch.c spawn.c)

macro(_incbin _binary_name _dep_target _binary_path)
    set(binary_name ${_binary_name})
    set(binary_path ${_binary_path})
    configure_file(incbin.tpl.S incbin_${_binary_name}.S)
    unset(binary_name)
    unset(binary_path)
    target_sources(tests.bin PRIVATE incbin_${_binary_name}.S)
    add_dependencies(tests.bin ${_dep_target})
endmacro()

_incbin(userproc_elf user.bin ${CMAKE_CURRENT_BINARY_DIR}/../../apps/user.bin)
_incbin(ipc_client_elf ipc_client.bin ${CMAKE_CURRENT_BINARY_DIR}/../../apps/ipc_client.bin)

chcore_install_target(tests.bin)
