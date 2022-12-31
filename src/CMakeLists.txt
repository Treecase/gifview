
# Use fontconfig to get path to default font
exec_program(fc-match ARGS --format=%{file} OUTPUT_VARIABLE DEFAULT_FONT_PATH)

configure_file(config.h.in "${PROJECT_BINARY_DIR}/include/config.h")
configure_file(font.h.in "${PROJECT_BINARY_DIR}/include/font.h")

target_sources(gifview PRIVATE
    main.c
    args.c
    fontrenderer.c
    keybinds.c
    sdlapp.c
    sdlgif.c
)

add_subdirectory(include)
add_subdirectory(viewer)

target_link_libraries(gifview PRIVATE gif linkedlist util viewer)
