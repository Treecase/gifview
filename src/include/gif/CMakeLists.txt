
add_library(gif STATIC
    gif.c
    gif-load.c
    lzw.c
)
target_link_libraries(gif
    PUBLIC
        linkedlist
    PRIVATE
        util
)
