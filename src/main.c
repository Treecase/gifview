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
#include "sdlapp.h"
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


/* ===[ Action Callbacks ]=== */
/* General */
void quit(struct App *G)
{
    viewer_quit(&G->view);
}
void fullscreen_toggle(struct App *G)
{
    if (SDL_GetWindowFlags(G->window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(G->window, 0);
    else
        SDL_SetWindowFullscreen(G->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

/* Zoom */
void zoom_in(struct App *G)
{
    viewer_zoom_in(&G->view);
}
void zoom_out(struct App *G)
{
    viewer_zoom_out(&G->view);
}
void zoom_default(struct App *G)
{
    viewer_zoom_reset(&G->view);
}

/* Scroll */
void scroll_up(struct App *G)
{
    viewer_shift_up(&G->view);
}
void scroll_down(struct App *G)
{
    viewer_shift_down(&G->view);
}
void scroll_right(struct App *G)
{
    viewer_shift_right(&G->view);
}
void scroll_left(struct App *G)
{
    viewer_shift_left(&G->view);
}

/* Playback */
void pause_toggle(struct App *G)
{
    G->paused = !G->paused;
}
void loop_toggle(struct App *G)
{
    puts("loop toggle");
    G->looping = !G->looping;
}
void speed_down(struct App *G)
{
    G->playback_speed *= 0.9;
}
void speed_up(struct App *G)
{
    G->playback_speed *= 1.1;
}
void speed_half(struct App *G)
{
    G->playback_speed *= 0.5;
}
void speed_double(struct App *G)
{
    G->playback_speed *= 2.0;
}
void speed_reset(struct App *G)
{
    G->playback_speed = 1.0;
}
void step_next(struct App *G)
{
    G->paused = true;
    app_next_frame(G);
}
void step_previous(struct App *G)
{
    G->paused = true;
    app_previous_frame(G);
}

#pragma GCC diagnostic push
/* The viewer_* functions are void(Viewer *) but Action wants a void(void *).
 * void * can be safely converted to anything so it shouldn't be an issue. */
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
/** List of keybindable actions. */
struct Action actions[] = {
    /* General */
    {"quit", quit, NULL, NULL, NULL},
    {"fullscreen_toggle", fullscreen_toggle, NULL, NULL, NULL},
    /* Zoom */
    {"zoom_in", zoom_in, NULL, NULL, NULL},
    {"zoom_out", zoom_out, NULL, NULL, NULL},
    {"zoom_default", zoom_default, NULL, NULL, NULL},
    /* Scroll */
    {"scroll_up", scroll_up, NULL, NULL, NULL},
    {"scroll_down", scroll_down, NULL, NULL, NULL},
    {"scroll_right", scroll_right, NULL, NULL, NULL},
    {"scroll_left", scroll_left, NULL, NULL, NULL},
    /* Playback */
    {"pause_toggle", pause_toggle, NULL, NULL, NULL},
    {"loop_toggle", loop_toggle, NULL, NULL, NULL},
    {"speed_down", speed_down, NULL, NULL, NULL},
    {"speed_up", speed_up, NULL, NULL, NULL},
    {"speed_half", speed_half, NULL, NULL, NULL},
    {"speed_double", speed_double, NULL, NULL, NULL},
    {"speed_reset", speed_reset, NULL, NULL, NULL},
    {"step_next", step_next, NULL, NULL, NULL},
    {"step_previous", step_previous, NULL, NULL, NULL},
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
    struct App G = app_new(&gif);

    keybinds_init();

    /* Start the frame update timer. */
    SDL_TimerID timer = SDL_AddTimer(10, timer_callback, NULL);

    bool screen_dirty = true;
    while (G.view.running)
    {
        /* Redraw the screen if dirty. */
        if (screen_dirty)
        {
            imagetransform_clamp(
                &G.view.transform, gif.width, gif.height, G.width, G.height);
            app_clear_screen(&G);
            app_draw(&G);
            screen_dirty = false;
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            viewer_quit(&G.view);
            break;

        case SDL_USEREVENT:
            if (app_timer_increment(&G))
                screen_dirty = true;
            break;

        case SDL_WINDOWEVENT:
            screen_dirty = true;
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                app_resize(&G, event.window.data1, event.window.data2);
            break;

        case SDL_KEYDOWN:
            screen_dirty = true;
            for (size_t i = 0; i < actions_count; ++i)
                if (action_ispressed(actions[i], event.key.keysym))
                    actions[i].action(&G);
            break;

        case SDL_MOUSEMOTION:
            if (event.motion.state & SDL_BUTTON_LMASK)
            {
                viewer_translate(&G.view, event.motion.xrel, event.motion.yrel);
                screen_dirty = true;
            }
            break;
        }
    }

    SDL_RemoveTimer(timer);
    app_free(&G);
    SDL_Quit();

    gif_free(gif);

    return EXIT_SUCCESS;
}
