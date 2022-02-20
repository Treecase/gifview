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

#include "gif.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SDL_MAIN_HANDLED    1
#include <SDL2/SDL.h>
#undef SDL_MAIN_HANDLED


/* Create an SDL_Surface based on GIF data. */
SDL_Surface *mk_SDLSurface_from_GIFImage(struct GIF gif)
{
    struct GIF_Image image = gif.graphics->img;
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
        image.data.image,
        image.width, image.height,
        8,
        image.width,
        SDL_PIXELFORMAT_INDEX8);

    size_t colorcount = gif.lsd.gct_size;
    uint8_t *table = gif.lsd.color_table;
    if (image.lct_flag)
    {
        colorcount = image.lct_size;
        table = image.color_table;
    }

    SDL_Color *colors = malloc(sizeof(SDL_Color) * colorcount);
    for (size_t i = 0; i < colorcount; ++i)
    {
        colors[i].r = table[(3*i)+0];
        colors[i].g = table[(3*i)+1];
        colors[i].b = table[(3*i)+2];
        colors[i].a = 255;
    }
    if (SDL_SetPaletteColors(surface->format->palette, colors, 0, colorcount) != 0)
    {
        fputs("palette set fail", stderr);
        exit(EXIT_FAILURE);
    }
    free(colors);
    return surface;
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "No filename given!\n");
        exit(EXIT_FAILURE);
    }
    char const *const filename = argv[1];

    GIF gif = load_gif_from_file(filename);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface *gifimage = mk_SDLSurface_from_GIFImage(gif);

    SDL_Window *window = SDL_CreateWindow(
        "GIF View",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        SDL_LogCritical(0, "Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    bool running = true;
    while (running)
    {
        SDL_Surface *screen = SDL_GetWindowSurface(window);
        Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
        if (SDL_FillRect(screen, NULL, color) != 0)
        {
            SDL_LogError(0, "ERROR SDL_FillRect: %s\n", SDL_GetError());
        }
        SDL_BlitSurface(gifimage, NULL, screen, NULL);
        SDL_FreeSurface(screen);
        if (SDL_UpdateWindowSurface(window) != 0)
        {
            SDL_LogError(
                0, "ERROR SDL_UpdateWindowSurface: %s\n", SDL_GetError());
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_FreeSurface(gifimage);
    SDL_Quit();

    return EXIT_SUCCESS;
}
