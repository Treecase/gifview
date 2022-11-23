/*
 * sdldata.c -- SDLData struct.
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

#include "sdldata.h"
#include "util.h"


struct SDLData sdldata_new(int window_width, int window_height)
{
    struct SDLData data;

    data.window = SDL_CreateWindow(
        "GIF View",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_RESIZABLE);
    if (data.window == NULL)
        fatal("Failed to create window: %s\n", SDL_GetError());

    data.renderer = SDL_CreateRenderer(
        data.window, -1, SDL_RENDERER_ACCELERATED);
    if (data.renderer == NULL)
        fatal("Failed to create renderer -- %s\n", SDL_GetError());

    SDL_GetWindowSize(data.window, &data.width, &data.height);
    sdldata_generate_bg_grid(&data);
    return data;
}

void sdldata_free(struct SDLData const *data)
{
    SDL_DestroyTexture(data->bg_texture);
    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);
}

void sdldata_generate_bg_grid(struct SDLData *data)
{
    SDL_Surface *grid_surf = SDL_CreateRGBSurfaceWithFormat(
        0, data->width, data->height, 32, SDL_PIXELFORMAT_RGBA32);

    Uint32 const grid_color_a = SDL_MapRGB(
        grid_surf->format,
        BACKGROUND_GRID_COLOR_A[0],
        BACKGROUND_GRID_COLOR_A[1],
        BACKGROUND_GRID_COLOR_A[2]);
    Uint32 const grid_color_b = SDL_MapRGB(
        grid_surf->format,
        BACKGROUND_GRID_COLOR_B[0],
        BACKGROUND_GRID_COLOR_B[1],
        BACKGROUND_GRID_COLOR_B[2]);

    SDL_FillRect(grid_surf, NULL, grid_color_a);
    for (int y = 0; y < (data->height/BACKGROUND_GRID_SIZE+1); ++y)
    {
        int const initial_x = (y % 2 == 1)? 0 : BACKGROUND_GRID_SIZE;
        for (int x = initial_x; x < data->width; x += BACKGROUND_GRID_SIZE*2)
        {
            SDL_Rect const rect = {
                .h = BACKGROUND_GRID_SIZE,
                .w = BACKGROUND_GRID_SIZE,
                .x = x,
                .y = y * BACKGROUND_GRID_SIZE
            };
            SDL_FillRect(grid_surf, &rect, grid_color_b);
        }
    }
    SDL_DestroyTexture(data->bg_texture);
    data->bg_texture = SDL_CreateTextureFromSurface(data->renderer, grid_surf);
    SDL_FreeSurface(grid_surf);
}

void sdldata_clear_screen(struct SDLData *data)
{
    SDL_RenderCopy(data->renderer, data->bg_texture, NULL, NULL);
}
