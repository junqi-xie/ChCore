cmake_minimum_required(VERSION 3.14)
project(ChCore)

set(_cmake_script_dir ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${_cmake_script_dir}/Modules)

include(SubProject)
include(CommonTools)

chcore_dump_chcore_vars()

chcore_get_nproc(_nproc)

if(CHCORE_VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

set(_common_args
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DCHCORE_PROJECT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE})

# Construct cache args list for subprojects (kernel, libchcore, etc)
macro(chcore_config _config_name _config_type _default _description)
    if(NOT DEFINED ${_config_name})
        message(
            FATAL_ERROR
                "Do not run CMake command directly, use `./chbuild` instead")
    endif()
    list(APPEND _cache_args
         -D${_config_name}:${_config_type}=${${_config_name}})
endmacro()
include(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)

# --- LibChCore ---

set(_libchcore_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/libchcore)
set(_libchcore_build_dir ${_libchcore_source_dir}/_build)
set(_libchcore_install_dir ${_libchcore_source_dir}/_install)

# Main targets for LibChCore
chcore_add_subproject(
    libchcore
    SOURCE_DIR ${_libchcore_source_dir}
    BINARY_DIR ${_libchcore_build_dir}
    INSTALL_DIR ${_libchcore_install_dir}
    CMAKE_ARGS
        ${_common_args}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_TOOLCHAIN_FILE=${_cmake_script_dir}/Toolchains/user.cmake
    CMAKE_CACHE_ARGS ${_cache_args}
    BUILD_ALWAYS TRUE)

# Clean target for LibChCore
add_custom_target(
    libchcore-clean
    COMMAND /bin/rm -rf ${_libchcore_build_dir}
    COMMAND /bin/rm -rf ${_libchcore_install_dir})

# --- User ---

set(_user_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/user)
set(_user_build_dir ${_user_source_dir}/_build)
set(_user_install_dir ${_user_source_dir}/_install)

# Target to force re-including cpio binaries
add_custom_target(
    user-clean-incbin
    COMMAND
        [ -d ${_user_build_dir} ]
        && find ${_user_build_dir} -type f -name "incbin_*.S.*" | xargs rm -f
        || true)

# Main targets for user stuffs
chcore_add_subproject(
    user
    SOURCE_DIR ${_user_source_dir}
    BINARY_DIR ${_user_build_dir}
    INSTALL_DIR ${_user_install_dir}
    CMAKE_ARGS
        ${_common_args}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_TOOLCHAIN_FILE=${_cmake_script_dir}/Toolchains/user.cmake
    CMAKE_CACHE_ARGS ${_cache_args}
    DEPENDS libchcore user-clean-incbin
    BUILD_ALWAYS TRUE)

# Clean target for user stuffs
add_custom_target(
    user-clean
    COMMAND /bin/rm -rf ${_user_build_dir}
    COMMAND /bin/rm -rf ${_user_install_dir})

# --- Kernel ---

set(_kernel_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/kernel)
set(_kernel_build_dir ${_kernel_source_dir}/_build)
set(_kernel_install_dir ${CMAKE_CURRENT_BINARY_DIR})

# Target to force re-including cpio binaries
add_custom_target(
    kernel-clean-incbin
    COMMAND
        [ -d ${_kernel_build_dir} ]
        && find ${_kernel_build_dir} -type f -name "incbin_*.S.*" | xargs rm -f
        || true)

# Main targets for kernel
chcore_add_subproject(
    kernel
    SOURCE_DIR ${_kernel_source_dir}
    BINARY_DIR ${_kernel_build_dir}
    INSTALL_DIR ${_kernel_install_dir}
    CMAKE_ARGS
        ${_common_args}
        -DCHCORE_USER_INSTALL_DIR=${_user_install_dir} # used by kernel/CMakeLists.txt to incbin cpio files
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_TOOLCHAIN_FILE=${_cmake_script_dir}/Toolchains/kernel.cmake
    CMAKE_CACHE_ARGS ${_cache_args}
    DEPENDS user kernel-clean-incbin
    BUILD_ALWAYS TRUE)

# Clean target for kernel
add_custom_target(
    kernel-clean
    COMMAND /bin/rm -rf ${_kernel_build_dir}
    COMMAND [ -f ${_kernel_install_dir}/install_manifest.txt ] && cat
            ${_kernel_install_dir}/install_manifest.txt | xargs rm -rf || true)

# --- Clean All ---

add_custom_target(
    clean-all
    COMMAND ${CMAKE_COMMAND} --build . --target kernel-clean
    COMMAND ${CMAKE_COMMAND} --build . --target user-clean
    COMMAND ${CMAKE_COMMAND} --build . --target libchcore-clean)
