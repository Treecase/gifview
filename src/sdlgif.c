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


/**
 * Interstitial structure used to construct the full frames contained in the
 * Graphic struct.  SDL representation of a GIF_Graphic.
 */
struct SurfaceGraphic
{
    SDL_Rect rect;
    SDL_Surface *surface;
};


/** Convert a GIF_ColorTable to an SDL color table. */
SDL_Color *_sdl_color_from_gif_colortable(struct GIF_ColorTable const *table)
{
    SDL_Color *colors = malloc(sizeof(SDL_Color) * table->size);
    for (size_t i = 0; i < table->size; ++i)
    {
        colors[i].r = table->colors[(3 * i) + 0];
        colors[i].g = table->colors[(3 * i) + 1];
        colors[i].b = table->colors[(3 * i) + 2];
        colors[i].a = 255;
    }
    return colors;
}

/** Create a SurfaceGraphic from a GIF_Image. */
struct SurfaceGraphic *surfacegraphic_from_image(struct GIF_Image const *image)
{
    struct SurfaceGraphic *out = malloc(sizeof(*out));
    out->rect.x = image->left;
    out->rect.y = image->top;
    out->rect.w = image->width;
    out->rect.h = image->height;
    out->surface = SDL_CreateRGBSurfaceWithFormatFrom(
        image->pixels,
        image->width, image->height,
        8,
        image->width,
        SDL_PIXELFORMAT_INDEX8);

    if (out->surface == NULL)
    {
        error("SDL_CreateRGBSurfaceWithFormatFrom: %s\n", SDL_GetError());
        free(out);
        return NULL;
    }
    return out;
}

/** Create a SurfaceGraphic from a GIF_Graphic. */
struct SurfaceGraphic *surfacegraphic_from_graphic(struct GIF_Graphic const *graphic)
{
    if (!graphic->is_img)
        return NULL;

    struct GIF_Image const *const image = &graphic->img;
    struct SurfaceGraphic *out = surfacegraphic_from_image(&graphic->img);
    if (!out)
        return NULL;

    /* Set transparency color. */
    if (graphic->extension && graphic->extension->transparent_color_flag)
    {
        SDL_SetColorKey(
            out->surface, SDL_TRUE, graphic->extension->transparent_color_idx);
    }

    /* SurfaceGraphic uses indexed color so we need a color table. */
    if (!image->color_table)
    {
        warn("surfacegraphic_from_graphic: Image has no palette!\n");
        return out;
    }
    struct GIF_ColorTable const *const table = image->color_table;
    SDL_Color *colors = _sdl_color_from_gif_colortable(table);
    int err = SDL_SetPaletteColors(
        out->surface->format->palette, colors, 0, table->size);
    if (err != 0)
        error("SDL_SetPaletteColors -- %s\n", SDL_GetError());
    free(colors);

    return out;
}

/** Free a SurfaceGraphic. */
void surfacegraphic_free(struct SurfaceGraphic *sg)
{
    SDL_FreeSurface(sg->surface);
    free(sg);
}


/** Create a new Graphic. */
struct Graphic *graphic_new(void)
{
    struct Graphic *graphic = malloc(sizeof(struct Graphic));
    graphic->delay = 0;
    graphic->width = 0;
    graphic->height = 0;
    graphic->texture = NULL;
    return graphic;
}


/**
 * Construct a frame of a GIF.  START will be updated to point to the
 * last processed graphic.  NEXTFRAME will be updated to contain the basis for
 * the next frame.
 */
SDL_Surface *_make_frame(LinkedList const **start, SDL_Surface **nextframe, GIF const *gif)
{
    LinkedList const *const start_orig = *start;
    LinkedList *surfacegraphics = NULL;

    /* Step through graphics until we find a graphic with a nonzero delay time,
     * which marks the start of a new frame. */
    for (; (*start)->next != NULL; *start = (*start)->next)
    {
        struct GIF_Graphic const *const graphic = (*start)->data;
        /* TODO: PlainText graphics are skipped for now. */
        if (!graphic->is_img)
            continue;
        struct SurfaceGraphic *g = surfacegraphic_from_graphic(graphic);
        linkedlist_append(&surfacegraphics, linkedlist_new(g));
        if (graphic->extension && graphic->extension->delay_time != 0)
            break;
    }
    struct GIF_Graphic const *const graphic = (*start)->data;
    /* TODO: PlainText graphics are skipped for now. */
    if (graphic->is_img)
    {
        struct SurfaceGraphic *g = surfacegraphic_from_graphic(graphic);
        linkedlist_append(&surfacegraphics, linkedlist_new(g));
    }

    /* Create the current frame, copying over data from the previous frame. */
    SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(
        0, (*nextframe)->w, (*nextframe)->h, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurface(*nextframe, NULL, frame, NULL);

    LinkedList const *gcurr = start_orig;
    LinkedList *sgcurr = surfacegraphics;
    for (; sgcurr != NULL; gcurr = gcurr->next)
    {
        struct GIF_Graphic const *const g = gcurr->data;
        struct SurfaceGraphic *const sg = sgcurr->data;

        struct GIF_GraphicExt const *const extension = g->extension;

        /* Apply the graphic to the next frame according to its disposal
         * method. */
        enum DisposalMethod const dm =\
            extension? extension->disposal_method : GIF_DisposalMethod_None;
        switch (dm)
        {
        case GIF_DisposalMethod_RestorePrevious:
            /* NEXTFRAME is the same as previous so don't draw the graphic. */
            break;
        case GIF_DisposalMethod_RestoreBackground:
            uint8_t bg[4] = {0, 0, 0, 0};
            struct GIF_ColorTable const * const gct = gif->global_color_table;
            if (gct)
            {
                uint8_t const index = gif->bg_color_index;
                memcpy(bg, gct->colors + index * 3, 3);
                bool const bg_is_transparent = (
                    extension->transparent_color_flag
                    && extension->transparent_color_idx == index);
                if (bg_is_transparent)
                    bg[3] = 0;
                else
                    bg[3] = 255;
            }
            SDL_FillRect(
                *nextframe,
                &sg->rect,
                SDL_MapRGBA((*nextframe)->format, bg[0], bg[1], bg[2], bg[3]));
            break;
        default:
            SDL_BlitSurface(sg->surface, NULL, *nextframe, &sg->rect);
            break;
        }

        SDL_BlitSurface(sg->surface, NULL, frame, &sg->rect);

        /* Free the list behind us. */
        LinkedList *old = sgcurr;
        sgcurr = sgcurr->next;
        surfacegraphic_free(old->data);
        free(old);
    }

    return frame;
}

GraphicList graphiclist_new(SDL_Renderer *renderer, GIF gif)
{
    SDL_Surface *lastframe = SDL_CreateRGBSurfaceWithFormat(
        0, gif.width, gif.height, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(lastframe, NULL, SDL_MapRGBA(lastframe->format, 0, 0, 0, 0));

    GraphicList out = NULL;

    LinkedList const *node = gif.graphics;
    while (node)
    {
        SDL_Surface *frame = _make_frame(&node, &lastframe, &gif);

        struct GIF_Graphic *g = node->data;
        struct Graphic *frame_g = graphic_new();
        frame_g->delay = g->extension? g->extension->delay_time : 0;
        frame_g->width = frame->w;
        frame_g->height = frame->h;
        frame_g->texture = SDL_CreateTextureFromSurface(renderer, frame);

        SDL_FreeSurface(frame);

        linkedlist_append(&out, linkedlist_new(frame_g));
        node = node->next;
    }

    /* Make the list circular, for free looping. */
    for (GraphicList g = out; g != NULL; g = g->next)
    {
        if (g->next == NULL)
        {
            g->next = out;
            break;
        }
    }

    return out;
}

void graphiclist_free(GraphicList graphics)
{
    for (GraphicList node = graphics->next; node != NULL;)
    {
        struct Graphic *graphic = node->data;
        SDL_DestroyTexture(graphic->texture);
        free(graphic);
        GraphicList next = node->next;
        free(node);
        if (node == graphics)
            break;
        node = next;
    }
}
