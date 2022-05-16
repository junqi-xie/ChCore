add_executable(tmpfs.srv main.c tmpfs_ops.c tmpfs.c cpio.c wrapper_ops.c wrapper.c vnode.c tmpfs_test.c)

add_dependencies(tmpfs.srv ramdisk.cpio)
set(binary_path ${CMAKE_BINARY_DIR}/ramdisk.cpio)
configure_file(incbin_ramdisk.tpl.S incbin_ramdisk.S)
target_sources(tmpfs.srv PRIVATE incbin_ramdisk.S)

if(CHCORE_TMPFS_TEST)
    target_compile_options(tmpfs.srv PRIVATE "-DTMPFS_TEST")
endif()
