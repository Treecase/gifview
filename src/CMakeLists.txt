
configure_file(config.h.in "${PROJECT_BINARY_DIR}/include/config.h")

target_sources(gifview PRIVATE
    main.c
    gif.c
    util.c
    lzw.c
    args.c
    keybinds.c
)
