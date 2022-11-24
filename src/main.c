/*
 * gifview - A GIF file viewer.
 * main.c -- Entry point for GIFView.
 *
 * Copyright (C) 2022 Trevor Last
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "args.h"
#include "keybinds.h"
#include "sdldata.h"
#include "sdlgif.h"
#include "viewer/viewer.h"

#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>


#if _WIN32
#define MAIN SDL_main
#else
#define MAIN main
#endif


#pragma GCC diagnostic push
/* The viewer_* functions are void(Viewer *) but Action wants a void(void *).
 * void * can be safely converted to anything so it shouldn't be an issue. */
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
/** List of keybindable actions. */
struct Action actions[] = {
    {"zoom_in",     viewer_zoom_in,     NULL, NULL, NULL},
    {"zoom_out",    viewer_zoom_out,    NULL, NULL, NULL},
    {"reset_zoom",  viewer_zoom_reset,  NULL, NULL, NULL},
    {"shift_up",    viewer_shift_up,    NULL, NULL, NULL},
    {"shift_down",  viewer_shift_down,  NULL, NULL, NULL},
    {"shift_right", viewer_shift_right, NULL, NULL, NULL},
    {"shift_left",  viewer_shift_left,  NULL, NULL, NULL},
    {"quit",        viewer_quit,        NULL, NULL, NULL},
};
#pragma GCC diagnostic pop

/** Length of actions array. */
size_t actions_count = sizeof(actions) / sizeof(*actions);


/**
 * 1/100 second timer callback.  Used to trigger frame changes in animated
 * GIFs.
 */
Uint32 timer_callback(Uint32 interval, void *param)
{
    SDL_Event event = {
        .type = SDL_USEREVENT,
        .user = {
            .code = 0,
            .data1 = NULL,
            .data2 = NULL,
            .type = SDL_USEREVENT
        }
    };
    SDL_PushEvent(&event);
    return interval;
}


int MAIN(int argc, char *argv[])
{
    char const *const filename = parse_args(argc, argv);
    GIF gif = gif_from_file(filename);

    for (LinkedList *node = gif.comments; node != NULL; node = node->next)
        printf("Comment: '%s'\n", (char const *)node->data);

    for (LinkedList *node = gif.app_extensions; node != NULL; node = node->next)
    {
        struct GIF_ApplicationExt const *ext = node->data;
        printf("App Extension: %.8s%.3s (%zu data bytes)\n",
            ext->appid, ext->auth_code, ext->data_size);
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    struct SDLData G = sdldata_new(gif.width, gif.height);
    sdldata_clear_screen(&G);

    struct Viewer viewer = {
        .running = true,
        .shift_amount = 2.5 * BACKGROUND_GRID_SIZE,
        /* In feh, zooming in 3 times doubles the image's size.  Zooming is
         * equivalent to exponentiation (eg. 3 zoom ins gives
         * `n*2*2*2 = n * 2^3`).  Therefore our equation is `2 = m^3`.  Solving
         * for m gives us our multiplier */
        .zoom_change_multiplier = 1.259921049894873,
        .dd = {
            .offset_x = 0,
            .offset_y = 0,
            .zoom = 1.0,
        }
    };

    keybinds_init();

    GraphicList images = graphiclist_new(G, gif);
    GraphicList current_frame = images;

    /* Start the frame update timer. */
    SDL_TimerID timer = SDL_AddTimer(10, timer_callback, NULL);
    size_t timer_increment = 0;

    bool screen_dirty = true;
    while (viewer.running)
    {
        /* Redraw the screen if dirty. */
        if (screen_dirty)
        {
            struct Graphic const *const img = current_frame->data;
            int img_scaled_h = img->height * viewer.dd.zoom;
            int img_scaled_w = img->width * viewer.dd.zoom;
            SDL_Rect position = {
                .h = img_scaled_h,
                .w = img_scaled_w,
                .x = G.width / 2 - img_scaled_w / 2 + viewer.dd.offset_x,
                .y = G.height / 2 - img_scaled_h / 2 + viewer.dd.offset_y
            };
            sdldata_clear_screen(&G);
            SDL_RenderCopy(G.renderer, img->texture, NULL, &position);
            SDL_RenderPresent(G.renderer);
            screen_dirty = false;
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            viewer_quit(&viewer);
            break;

        case SDL_USEREVENT:
            struct Graphic const *image = current_frame->data;
            timer_increment++;
            if (timer_increment >= image->delay)
            {
                current_frame = current_frame->next;
                timer_increment = 0;
                screen_dirty = true;
            }
            break;

        case SDL_WINDOWEVENT:
            screen_dirty = true;
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                G.width  = event.window.data1;
                G.height = event.window.data2;
                /* Recenter image when window changes size. */
                viewer.dd.offset_x = 0;
                viewer.dd.offset_y = 0;
                sdldata_generate_bg_grid(&G);
            }
            break;

        case SDL_KEYDOWN:
            for (size_t i = 0; i < actions_count; ++i)
                if (action_ispressed(actions[i], event.key.keysym))
                    actions[i].action(&viewer);
            screen_dirty = true;
            break;

        case SDL_MOUSEMOTION:
            if (event.motion.state & SDL_BUTTON_LMASK)
            {
                viewer.dd.offset_x += event.motion.xrel;
                viewer.dd.offset_y += event.motion.yrel;
                screen_dirty = true;
            }
            break;
        }
        drawdata_clamp(&viewer.dd, gif.width, gif.height, G.width, G.height);
    }

    graphiclist_free(images);

    SDL_RemoveTimer(timer);
    sdldata_free(&G);
    SDL_Quit();

    gif_free(gif);

    return EXIT_SUCCESS;
}
