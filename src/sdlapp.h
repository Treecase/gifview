/*
 * sdlapp.h -- App struct.
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

#ifndef _GIFVIEW_SDLAPP_H
#define _GIFVIEW_SDLAPP_H

#include "sdlgif.h"
#include "viewer/viewer.h"

#include <SDL2/SDL.h>


/** SDL data for app. */
struct App
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bg_texture;
    int width, height;
    struct Viewer view;
    GraphicList images, current_frame;
    size_t timer;
};


/** Size (in pixels) of background grid squares. */
static int const BACKGROUND_GRID_SIZE = 8;

/** Color for even-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_A[3] = {0x64, 0x64, 0x64};
/** Color for odd-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_B[3] = {0x90, 0x90, 0x90};


/** Create SDL data. */
struct App app_new(GIF const *gif);

/** Free SDL data. */
void app_free(struct App const *app);

/** Clear the screen. */
void app_clear_screen(struct App *app);

/** Increment the timer, returning true if we've moved to the next frame. */
bool app_timer_increment(struct App *app);

/** Draw the screen. */
void app_draw(struct App *app);

/** Resize the screen. */
void app_resize(struct App *app, int width, int height);


#endif /* _GIFVIEW_SDLAPP_H */
