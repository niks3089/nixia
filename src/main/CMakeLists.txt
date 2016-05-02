FILE(GLOB nixia_sources *.c)
FILE(GLOB nixia_headers *.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(nixia ${nixia_sources}) 
target_link_libraries(nixia 
        log common http csperf common
        libevent rt m pthread
        libevent libzlog libevent_pthreads libevhtp libhiredis libncurses)

# Copy configuration files  
install(FILES ${LOG_CONFIG} DESTINATION ${CONFIG_DIR}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_WRITE GROUP_READ
    WORLD_EXECUTE WORLD_WRITE WORLD_READ)

install(TARGETS nixia RUNTIME DESTINATION /usr/local/bin)
