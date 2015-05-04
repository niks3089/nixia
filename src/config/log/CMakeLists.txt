FILE(GLOB nixia_sources *.c)
FILE(GLOB nixia_headers *.h)
add_library(log ${nixia_sources} ${nixia_headers}) 
