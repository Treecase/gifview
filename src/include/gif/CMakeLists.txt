
add_library(gif OBJECT
    gif.c
    lzw.c
)
target_link_libraries(gif PUBLIC linkedlist)
target_link_libraries(gif PRIVATE util)
