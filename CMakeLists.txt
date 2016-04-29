cmake_minimum_required(VERSION 2.8)

project(nixia)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

#Our variables
# Target directory
#This is setting in /usr/local/opt/nixia
set(CONFIG_DIR "/etc/nixia")
set(MAN_PAGE_DIR "share/man/man1")

set(LOG_CONFIG ${PROJECT_SOURCE_DIR}/src/config/log/log.conf)
set(MAN_PAGE   ${PROJECT_SOURCE_DIR}/docs/nixia.1)

set(CMAKE_C_FLAGS "-g -Wall -Werror -Wfatal-errors")

# set up include-directories
include_directories(
    "${PROJECT_SOURCE_DIR}/src/common"
    "${PROJECT_SOURCE_DIR}/src/config/log"
    "${PROJECT_SOURCE_DIR}/src/http"
    "${PROJECT_SOURCE_DIR}/src/csperf"
    "${PROJECT_SOURCE_DIR}/src/main"
    "${PROJECT_BINARY_DIR}")
    
set(PATCHES_DIR ${PROJECT_SOURCE_DIR}/patches)

include(ExternalProject)

#URL https://github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz 
# Build libevent
ExternalProject_Add(libevent2
        URL ${PROJECT_SOURCE_DIR}/packages/libevent-2.0.21-stable.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/libevent2
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> 
        UPDATE_COMMAND ""
)
ExternalProject_Get_Property(libevent2 install_dir)
add_library(libevent STATIC IMPORTED)
set_property(TARGET libevent PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libevent.a)
add_dependencies(libevent libevent2)
include_directories(${install_dir}/include)

add_library(libevent_pthreads STATIC IMPORTED)
set_property(TARGET libevent_pthreads PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libevent_pthreads.a)

# Find event_pthreads 
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${install_dir}/lib)
#find_library(libevent_pthreads libevent_pthreads PATHS ${install_dir}/lib)

# Build libevhtp
ExternalProject_Add(evhtp
        DEPENDS libevent2
        URL ${PROJECT_SOURCE_DIR}/packages/libevhtp.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/evhtp
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE:STRING=Debug
        -DLIBEVENT_LIBRARY:FILEPATH=${CMAKE_BINARY_DIR}/external/libevent2/lib
        -DLIBEVENT_EXTRA_LIBRARY:FILEPATH=${CMAKE_BINARY_DIR}/external/libevent2/lib
        -DLIBEVENT_CORE_LIBRARY:FILEPATH=${CMAKE_BINARY_DIR}/external/libevent2/lib
        -DLIBEVENT_PTHREADS_LIBRARY:FILEPATH=${CMAKE_BINARY_DIR}/external/libevent2/lib
        -DLIBEVENT_INCLUDE_DIR:PATH=${CMAKE_BINARY_DIR}/external/libevent2/include
        -DEVHTP_DISABLE_SSL:STRING=ON <SOURCE_DIR> && 
        make -C <SOURCE_DIR>/../evhtp-build
        INSTALL_COMMAND make install 
)
ExternalProject_Get_Property(evhtp install_dir)
add_library(libevhtp STATIC IMPORTED)
set_property(TARGET libevhtp PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libevhtp.a)
add_dependencies(libevhtp evhtp)
include_directories(${install_dir}/include/evhtp)

# Build zlog
ExternalProject_Add(zlog
        URL ${PROJECT_SOURCE_DIR}/packages/zlog.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/zlog
        CONFIGURE_COMMAND ""
        PATCH_COMMAND patch -p1 < ${PROJECT_SOURCE_DIR}/patches/zlog_error.patch
        UPDATE_COMMAND ""
        BUILD_COMMAND make -C <SOURCE_DIR> 
        INSTALL_COMMAND make -C <SOURCE_DIR> install PREFIX=<INSTALL_DIR> 
)
ExternalProject_Get_Property(zlog install_dir)
add_library(libzlog STATIC IMPORTED)
set_property(TARGET libzlog PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libzlog.a)
add_dependencies(libzlog zlog)
include_directories(${install_dir}/include)

# Build hiredis
ExternalProject_Add(hiredis
        URL ${PROJECT_SOURCE_DIR}/packages/hiredis.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/hiredis
        CONFIGURE_COMMAND ""
        UPDATE_COMMAND ""
        BUILD_COMMAND make -C <SOURCE_DIR> 
        INSTALL_COMMAND make -C <SOURCE_DIR> install PREFIX=<INSTALL_DIR> 
)
ExternalProject_Get_Property(hiredis install_dir)
add_library(libhiredis STATIC IMPORTED)
set_property(TARGET libhiredis PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libhiredis.a)
add_dependencies(libhiredis hiredis)
include_directories(${install_dir}/include/hiredis)

# Build ncurses 
ExternalProject_Add(ncurses
        URL ${PROJECT_SOURCE_DIR}/packages/ncurses.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/ncurses
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> 
        PATCH_COMMAND patch -p1 < ${PROJECT_SOURCE_DIR}/patches/ncurses.patch
        UPDATE_COMMAND ""
)
ExternalProject_Get_Property(ncurses install_dir)
add_library(libncurses STATIC IMPORTED)
set_property(TARGET libncurses PROPERTY IMPORTED_LOCATION
        ${install_dir}/lib/libncurses.a)
add_dependencies(libncurses ncurses)
include_directories(${install_dir}/include)

# Add sub-directories
    add_subdirectory(src/config/log)
    add_subdirectory(src/common)
    add_subdirectory(src/http)
    add_subdirectory(src/csperf)
    add_subdirectory(src/main)

install(FILES ${MAN_PAGE} DESTINATION ${MAN_PAGE_DIR}
    PERMISSIONS OWNER_WRITE OWNER_READ
    GROUP_READ WORLD_READ)

include(CTest)
enable_testing()

#Valgrind settings
find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS --show-reachable=yes --leak-check=full
    --log-file=${CMAKE_BINARY_DIR}/valgrind_output.txt)

# Regression
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/examples/configurations/*.json")
foreach(file ${files})
    get_filename_component(test_name ${file} NAME)
    add_test(${test_name} nixia -f ${file})
    add_test(memcheck_${test_name} ${MEMORYCHECK_COMMAND}
        ${MEMORYCHECK_COMMAND_OPTIONS} nixia -f ${file}) 
endforeach()

