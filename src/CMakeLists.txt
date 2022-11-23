
configure_file(config.h.in "${PROJECT_BINARY_DIR}/include/config.h")

target_sources(gifview PRIVATE
    main.c
    args.c
    keybinds.c
    sdldata.c
    sdlgif.c
    util.c
)

add_subdirectory(gif)
add_subdirectory(viewer)

target_link_libraries(gifview PRIVATE gif viewer)
