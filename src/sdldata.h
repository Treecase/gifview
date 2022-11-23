/*
 * sdldata.h -- SDLData struct.
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

#ifndef _GIFVIEW_SDLDATA_H
#define _GIFVIEW_SDLDATA_H

#include <SDL2/SDL.h>


/** SDL data for the app. */
struct SDLData
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bg_texture;
    int width, height;
};


/** Size (in pixels) of background grid squares. */
static int const BACKGROUND_GRID_SIZE = 8;

/** Color for even-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_A[3] = {0x64, 0x64, 0x64};
/** Color for odd-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_B[3] = {0x90, 0x90, 0x90};


/** Create SDL data. */
struct SDLData sdldata_new(int window_width, int window_height);

/** Free SDL data. */
void sdldata_free(struct SDLData const *data);

/** Generate a background grid texture. */
void sdldata_generate_bg_grid(struct SDLData *data);

/** Clear the screen. */
void sdldata_clear_screen(struct SDLData *data);


#endif /* _GIFVIEW_SDLDATA_H */
