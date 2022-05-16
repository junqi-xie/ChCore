add_executable(badinst1.bin badinst1.c)
add_executable(badinst2.bin badinst2.c)
add_executable(fault.bin fault.c)
add_executable(hello.bin hello.c)
add_executable(putget.bin putget.c)

add_executable(user.bin user.c)
add_executable(ipc_client.bin ipc_client.c)
add_executable(yield_single.bin yield_single.c)
add_executable(yield_aff.bin yield_aff.c)
add_executable(yield_multi_aff.bin yield_multi_aff.c)
add_executable(yield_multi.bin yield_multi.c)
add_executable(yield_spin.bin yield_spin.c)

chcore_install_all_targets()

add_executable(helloworld.bin hello.c)

chcore_copy_all_targets_to_ramdisk()
chcore_copy_files_to_ramdisk(test.txt)
