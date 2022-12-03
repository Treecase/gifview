/*
 * sdlgif.c -- SDL representation of GIF data.
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

#include "sdlgif.h"


/** Create a SurfaceGraphic from a GIF_Graphic. */
struct SurfaceGraphic *surfacegraphic_from_graphic(struct GIF_Graphic graphic)
{
    if (!graphic.is_img)
        return NULL;

    struct GIF_Image image = graphic.img;
    struct SurfaceGraphic *out = malloc(sizeof(*out));
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
        error("SDL_CreateRGBSurfaceWithFormatFrom: %s\n", SDL_GetError());
        free(out);
        return NULL;
    }

    if (graphic.extension)
    {
        if (graphic.extension->transparent_color_flag)
        {
            SDL_SetColorKey(
                out->surface,
                SDL_TRUE,
                graphic.extension->transparent_color_idx);
        }
    }

    struct GIF_ColorTable const *table = image.color_table;
    if (table == NULL)
    {
        warn("surfacegraphic_from_graphic: Image has no palette!\n");
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
        error("SDL_SetPaletteColors -- %s\n", SDL_GetError());
    }
    free(colors);
    return out;
}


GraphicList graphiclist_new(SDL_Renderer *renderer, GIF gif)
{
    SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(
        0, gif.width, gif.height, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(frame, NULL, SDL_MapRGBA(frame->format, 0, 0, 0, 0xff));

    GraphicList images = NULL;
    for (GraphicList node = gif.graphics; node != NULL; node = node->next)
    {
        struct GIF_Graphic *graphic = node->data;
        if (graphic->is_img)
        {
            struct SurfaceGraphic *g = surfacegraphic_from_graphic(*graphic);
            SDL_BlitSurface(g->surface, NULL, frame, &g->rect);
            if (graphic->extension)
            {
                struct Graphic *frameout = malloc(sizeof(*frameout));
                frameout->delay = graphic->extension->delay_time;
                frameout->width = gif.width;
                frameout->height = gif.height;
                frameout->texture = SDL_CreateTextureFromSurface(
                    renderer, frame);
                linkedlist_append(&images, linkedlist_new(frameout));
                if (node->next != NULL)
                {
                    SDL_Surface *newframe = SDL_CreateRGBSurfaceWithFormat(
                        0, gif.width, gif.height, 32, SDL_PIXELFORMAT_RGBA32);
                    SDL_BlitSurface(frame, NULL, newframe, NULL);
                    SDL_FreeSurface(frame);
                    frame = newframe;
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
            /* TODO: PlainText rendering. */
        }
    }
    if (frame != NULL)
    {
        struct Graphic *frameout = malloc(sizeof(*frameout));
        frameout->delay = 0;
        frameout->texture = SDL_CreateTextureFromSurface(renderer, frame);
        linkedlist_append(&images, linkedlist_new(frameout));
        SDL_FreeSurface(frame);
    }
    /* Make the list circular, for free looping. */
    for (GraphicList g = images; g != NULL; g = g->next)
    {
        if (g->next == NULL)
        {
            g->next = images;
            break;
        }
    }
    return images;
}

void graphiclist_free(GraphicList graphics)
{
    for (GraphicList node = graphics->next; node != NULL;)
    {
        struct Graphic *graphic = node->data;
        SDL_DestroyTexture(graphic->texture);
        GraphicList next = node->next;
        free(graphic);
        free(node);
        if (node == graphics)
            break;
        node = next;
    }
}
