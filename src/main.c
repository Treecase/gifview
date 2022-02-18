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

#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>


int main(int argc, char const *argv[])
{
#if 0
    SDL_Init(SDL_INIT_VIDEO);

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
    SDL_Quit();
#endif // 0

    if (argc != 2)
    {
        fprintf(stderr, "No filename given!\n");
        exit(EXIT_FAILURE);
    }
    char const *const filename = argv[1];

    GIF gif = load_gif_from_file(filename);

    switch (gif.version)
    {
    case GIF_Version_87a:
        puts("GIF Version 87a");
        break;
    case GIF_Version_89a:
        puts("GIF Version 89a");
        break;

    default:
        puts("Unknown GIF Version");
        break;
    }

    printf("%dx%d\n", gif.lsd.width, gif.lsd.height);
    printf("GCT? %s\n", gif.lsd.gct_flag? "yes" : "no");
    printf("color resolution: %d\n", gif.lsd.color_resolution);
    printf("sorted? %s\n", gif.lsd.sort_flag? "yes" : "no");
    printf("GCT size: %lu\n", gif.lsd.gct_size);
    puts("");

    for (
        struct GIF_Graphic *graphic = gif.graphics;
        graphic != NULL;
        graphic = graphic->next)
    {
        puts("graphic:");
        if (graphic->has_extension)
        {
            struct GIF_GraphicExt const *const ext = &graphic->extension;
            puts(" ext:");
            char const *disposal_str = "???";
            switch (ext->disposal_method)
            {
            case GIF_DisposalMethod_DoNotDispose:
                disposal_str = "do not dispose";
                break;
            case GIF_DisposalMethod_None:
                disposal_str = "none";
                break;
            case GIF_DisposalMethod_RestoreBackground:
                disposal_str = "restore background";
                break;
            case GIF_DisposalMethod_RestorePrevious:
                disposal_str = "restore previous";
                break;
            case GIF_DisposalMethod_Undefined:
                disposal_str = "undefined";
                break;
            }
            printf("  disposal method: %s\n", disposal_str);
            printf("  user input? %s\n", ext->user_input_flag? "yes" : "no");
            printf("  transparent color? %s\n", ext->transparent_color_flag? "yes" : "no");
            printf("  delay time: %d\n", ext->delay_time);
            printf("  transparent color idx: %d\n", ext->transparent_color_idx);
        }
        if (graphic->is_img)
        {
            struct GIF_Image const *const i = &graphic->img;
            printf(" %d,%d  %dx%d\n", i->left, i->right, i->width, i->height);
            printf(" LCT? %s\n", i->lct_flag? "yes" : "no");
            printf(" interlaced? %s\n", i->interlace_flag? "yes" : "no");
            printf(" sorted? %s\n", i->sort_flag? "yes" : "no");
            printf(" LCT size: %lu\n", i->lct_size);
        }
        else
        {
            puts(" plaintext:");
        }
    }

    return EXIT_SUCCESS;
}
