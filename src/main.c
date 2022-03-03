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
#include "args.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SDL_MAIN_HANDLED 1  // For compiling with MinGW.
#include <SDL2/SDL.h>
#undef SDL_MAIN_HANDLED


/* SDL data for a GIF graphic. */
struct Graphic
{
    SDL_Rect rect;
    SDL_Surface *surface;
    size_t delay;
};

/* SDL data for the app. */
struct sdldata
{
    SDL_Window *window;
    SDL_Surface *screen;
};


/* 1/100 second timer callback.  Used to trigger frame changes in animated
 * GIFs. */
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

/* Create SDL data. */
struct sdldata init_sdl(int window_width, int window_height)
{
    struct sdldata data;

    data.window = SDL_CreateWindow(
        "GIF View",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_RESIZABLE);
    if (data.window == NULL)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "Failed to create window -- %s\n",
            SDL_GetError());
        exit(EXIT_FAILURE);
    }

    data.screen = SDL_GetWindowSurface(data.window);
    if (data.screen == NULL)
    {
        SDL_LogCritical(
            SDL_LOG_CATEGORY_APPLICATION,
            "Failed to create screen -- %s\n",
            SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return data;
}

/* Free SDL data. */
void free_sdl(struct sdldata const *data)
{
    SDL_FreeSurface(data->screen);
    SDL_DestroyWindow(data->window);
}

/* Create a Graphic from a GIF_Graphic. */
struct Graphic *mk_SDLSurface_from_GIFImage(struct GIF_Graphic graphic)
{
    if (!graphic.is_img)
    {
        return NULL;
    }
    struct GIF_Image image = graphic.img;
    struct Graphic *out = malloc(sizeof(*out));
    out->rect.x = image.left;
    out->rect.y = image.right;
    out->rect.w = image.width;
    out->rect.h = image.height;
    out->surface = SDL_CreateRGBSurfaceWithFormatFrom(
        image.pixels,
        image.width, image.height,
        8,
        image.width,
        SDL_PIXELFORMAT_INDEX8);

    if (out->surface == NULL)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_CreateRGBSurfaceWithFormatFrom -- %s\n",
            SDL_GetError());
        free(out);
        return NULL;
    }

    if (graphic.extension)
    {
        out->delay = graphic.extension->delay_time;
        if (graphic.extension->transparent_color_flag)
        {
            SDL_SetColorKey(
                out->surface,
                SDL_TRUE,
                graphic.extension->transparent_color_idx);
        }
    }
    else
    {
        out->delay = 0;
    }

    struct GIF_ColorTable const *table = image.color_table;
    if (table == NULL)
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "mk_SDLSurface_from_GIFImage -- Image has no palette!\n");
        return out;
    }
    SDL_Color *colors = malloc(sizeof(SDL_Color) * table->size);
    for (size_t i = 0; i < table->size; ++i)
    {
        colors[i].r = table->colors[(3*i)+0];
        colors[i].g = table->colors[(3*i)+1];
        colors[i].b = table->colors[(3*i)+2];
        colors[i].a = 255;
    }

    if (SDL_SetPaletteColors(
            out->surface->format->palette, colors, 0, table->size)
        != 0)
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "SDL_SetPaletteColors -- %s\n",
            SDL_GetError());
    }
    free(colors);
    return out;
}

/* Generate a linked list of Graphics from a linked list of GIF_Graphics. */
LinkedList *graphics2surfaces(GIF gif)
{
    SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(
        0,
        gif.width, gif.height,
        32,
        SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(frame, NULL, SDL_MapRGBA(frame->format, 0, 0, 0, 0xff));

    LinkedList *images = NULL;
    for (LinkedList *node = gif.graphics; node != NULL; node = node->next)
    {
        struct GIF_Graphic *graphic = node->data;
        if (graphic->is_img)
        {
            struct Graphic *g = mk_SDLSurface_from_GIFImage(*graphic);
            SDL_BlitSurface(g->surface, NULL, frame, &g->rect);
            if (g->delay != 0)
            {
                struct Graphic *frameout = malloc(sizeof(*frameout));
                frameout->delay = g->delay;
                frameout->rect.h = gif.height;
                frameout->rect.w = gif.width;
                frameout->rect.x = 0;
                frameout->rect.y = 0;
                frameout->surface = frame;
                linkedlist_append(&images, linkedlist_new(frameout));
                if (node->next != NULL)
                {
                    frame = SDL_CreateRGBSurfaceWithFormat(
                        0,
                        gif.width, gif.height,
                        32,
                        SDL_PIXELFORMAT_RGBA32);
                    SDL_BlitSurface(frameout->surface, NULL, frame, NULL);
                }
                else
                {
                    frame = NULL;
                }
            }
            SDL_FreeSurface(g->surface);
            free(g);
        }
        else
        {
            /* TODO: PlainText rendering */
        }
    }
    if (frame != NULL)
    {
        struct Graphic *frameout = malloc(sizeof(*frameout));
        frameout->delay = 0;
        frameout->rect.h = gif.height;
        frameout->rect.w = gif.width;
        frameout->rect.x = 0;
        frameout->rect.y = 0;
        frameout->surface = frame;
        linkedlist_append(&images, linkedlist_new(frameout));
    }
    /* Make the list circular, for free looping. */
    for (LinkedList *g = images; g != NULL; g = g->next)
    {
        if (g->next == NULL)
        {
            g->next = images;
            break;
        }
    }
    return images;
}

/* Free a linked list of Graphics. */
void free_graphics(LinkedList *graphics)
{
    for (LinkedList *node = graphics->next; node != NULL;)
    {
        struct Graphic *graphic = node->data;
        SDL_FreeSurface(graphic->surface);
        LinkedList *next = node->next;
        free(node);
        if (next == graphics)
        {
            break;
        }
        node = next;
    }
}

/* Clear the screen. */
void clear_screen(struct sdldata G)
{
    static int const grid_size = 8;

    Uint32 const grid_a = SDL_MapRGB(G.screen->format, 0x90, 0x90, 0x90);
    Uint32 const grid_b = SDL_MapRGB(G.screen->format, 0x64, 0x64, 0x64);

    int w, h;
    SDL_GetWindowSize(G.window, &w, &h);

    SDL_FillRect(G.screen, NULL, grid_a);
    for (int y = 0; y < (h/grid_size+1); ++y)
    {
        for (int x = ((y % 2 == 1)? 0 : grid_size); x < w; x += grid_size*2)
        {
            SDL_Rect const rect = {
                .h = grid_size,
                .w = grid_size,
                .x = x,
                .y = y * grid_size};
            SDL_FillRect(G.screen, &rect, grid_b);
        }
    }
}

int main(int argc, char *argv[])
{
    char const *const filename = parse_args(argc, argv);
    GIF gif = load_gif_from_file(filename);

    for (LinkedList *node = gif.comments; node != NULL; node = node->next)
    {
        printf("Comment: '%s'\n", (char const *)node->data);
    }
    for (LinkedList *node = gif.app_extensions; node != NULL; node = node->next)
    {
        struct GIF_ApplicationExt const *ext = node->data;
        printf("App Extension: %.8s%.3s (%zu data bytes)\n",
            ext->appid, ext->auth_code, ext->data_size);
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    /* Generate SDL_Surfaces from the GIF's graphics. */
    LinkedList *images = graphics2surfaces(gif);

    /* G holds all the SDL stuff. */
    struct sdldata G = init_sdl(gif.width, gif.height);

    /* Clear the screen. */
    clear_screen(G);

    /* Current frame of the GIF's animation. */
    LinkedList *current_frame = images;

    /* Start the frame update timer. */
    SDL_TimerID timer = SDL_AddTimer(10, timer_callback, NULL);
    size_t timer_increment = 0;

    bool screen_dirty = true;
    bool running = true;
    while (running)
    {
        if (screen_dirty)
        {
            struct Graphic *img = current_frame->data;
            int w, h;
            SDL_GetWindowSize(G.window, &w, &h);
            SDL_Rect position = {
                .h = img->rect.h,
                .w = img->rect.w,
                .x = (w / 2) - (img->rect.w / 2),
                .y = (h / 2) - (img->rect.h / 2)
            };
            clear_screen(G);
            SDL_BlitSurface(img->surface, NULL, G.screen, &position);
            if (SDL_UpdateWindowSurface(G.window))
            {
                G.screen = SDL_GetWindowSurface(G.window);
                clear_screen(G);
                SDL_BlitSurface(img->surface, NULL, G.screen, &img->rect);
                if (SDL_UpdateWindowSurface(G.window))
                {
                    SDL_LogError(
                        SDL_LOG_CATEGORY_APPLICATION,
                        "SDL_UpdateWindowSurface -- %s\n",
                        SDL_GetError());
                }
            }
            screen_dirty = false;
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            running = false;
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
            break;
        }
    }

    free_graphics(images);

    SDL_RemoveTimer(timer);
    SDL_FreeSurface(G.screen);
    SDL_DestroyWindow(G.window);
    SDL_Quit();

    free_gif(gif);

    return EXIT_SUCCESS;
}
