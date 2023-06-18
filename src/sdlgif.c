/*
 * sdlgif.c -- SDL representation of GIF data.
 *
 * Copyright (C) 2022-2023 Trevor Last
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
#include "font.h"

#include <string.h>

#include <SDL_ttf.h>


#define MIN(a, b)   (a < b? a : b)


/**
 * Interstitial structure used to construct the full frames contained in the
 * SDLGraphic struct.  SDL representation of a GIF_Graphic.
 */
struct SurfaceGraphic
{
    SDL_Rect rect;
    SDL_Surface *surface;
};


SDL_Color
sdl_color_get_from_colortable(struct GIF_ColorTable const *table, size_t index)
{
    SDL_Color color = {.a=0xff};
    color.r = table->colors[(3 * index) + 0];
    color.g = table->colors[(3 * index) + 1];
    color.b = table->colors[(3 * index) + 2];
    return color;
}

/** Convert a GIF_ColorTable to a list of SDL_Colors. */
SDL_Color *sdl_color_list_from_colortable(struct GIF_ColorTable const *table)
{
    SDL_Color *const colors = malloc(sizeof(SDL_Color) * table->size);
    for (size_t i = 0; i < table->size; ++i)
        colors[i] = sdl_color_get_from_colortable(table, i);
    return colors;
}

/** Create a SurfaceGraphic from a GIF_Image. */
struct SurfaceGraphic *surfacegraphic_from_image(struct GIF_Image const *image)
{
    struct SurfaceGraphic *const out = malloc(sizeof(*out));
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
        error("SDL_CreateRGBSurfaceWithFormatFrom -- %s\n", SDL_GetError());
        free(out);
        return NULL;
    }

    struct GIF_ColorTable const *const table = image->color_table;
    if (!table)
    {
        warn("surfacegraphic_from_image -- Image has no palette!\n");
        return out;
    }

    SDL_Color *const colors = sdl_color_list_from_colortable(table);
    int const err = SDL_SetPaletteColors(
        out->surface->format->palette, colors, 0, table->size);
    if (err != 0)
        error("SDL_SetPaletteColors -- %s\n", SDL_GetError());
    free(colors);

    return out;
}

/** Fit the font to the given width/height. */
int fit_font_to_rect(int width, int height)
{
    static float const POINTS_PER_INCH = 72.0f;

    float hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, NULL, &hdpi, &vdpi) != 0)
    {
        error("SDL_GetDisplayDPI -- %s\n", SDL_GetError());
        hdpi = 72.0f;
        vdpi = 72.0f;
    }

    float const width_inches = (float)width / hdpi;
    float const height_inches = (float)height / vdpi;

    float const h_points = width_inches * POINTS_PER_INCH;
    float const v_points = height_inches * POINTS_PER_INCH;
    return MIN(v_points, h_points);
}

/** Create a SurfaceGraphic from a GIF_PlainTextExt. */
struct SurfaceGraphic *surfacegraphic_from_plaintext(
    struct GIF_PlainTextExt const *restrict plaintext,
    struct GIF_ColorTable const *restrict gct)
{
    struct SurfaceGraphic *out = malloc(sizeof(*out));
    out->rect.x = plaintext->tg_left;
    out->rect.y = plaintext->tg_top;
    out->rect.w = plaintext->tg_width;
    out->rect.h = plaintext->tg_height;

    SDL_Color fg = sdl_color_get_from_colortable(gct, plaintext->fg_idx);
    SDL_Color bg = sdl_color_get_from_colortable(gct, plaintext->bg_idx);

    int points = fit_font_to_rect(
        plaintext->cell_width, plaintext->cell_height);
    TTF_Font *font = TTF_OpenFont(DEFAULT_MONOSPACE_FONT_PATH, points);
    if (!font)
        error("TTF_OpenFont -- %s\n", TTF_GetError());

    /* TODO:
     * Sometimes draws boxes, maybe need to draw characters individually? */
    /* Using TTF_RenderUTF8_Solid_Wrapped here because we need to stick to the
     * given palette colors. */
    char *text = strndup(plaintext->data, plaintext->data_size);
    out->surface = TTF_RenderUTF8_Solid_Wrapped(
        font, plaintext->data, fg, out->rect.w);
    SDL_SetColorKey(out->surface, SDL_FALSE, 0);
    SDL_SetPaletteColors(out->surface->format->palette, &bg, 0, 1);
    free(text);
    if (!out->surface)
        error("TTF_RenderUTF8_Shaded_Wrapped -- %s\n", TTF_GetError());

    TTF_CloseFont(font);
    return out;
}

/** Create a SurfaceGraphic from a GIF_Graphic. */
struct SurfaceGraphic *surfacegraphic_from_graphic(
    struct GIF_Graphic const *restrict graphic,
    struct GIF_ColorTable const *restrict gct)
{
    struct SurfaceGraphic *out = NULL;
    if (graphic->is_img)
        out = surfacegraphic_from_image(&graphic->img);
    else
        out = surfacegraphic_from_plaintext(&graphic->plaintext, gct);
    if (!out)
        return NULL;

    /* Set transparency color. */
    if (graphic->extension && graphic->extension->transparent_color_flag)
    {
        SDL_SetColorKey(
            out->surface,
            SDL_TRUE,
            graphic->extension->transparent_color_idx);
    }
    return out;
}

/** Free a SurfaceGraphic. */
void surfacegraphic_free(struct SurfaceGraphic *sg)
{
    SDL_FreeSurface(sg->surface);
    free(sg);
}


/** Allocate a new SDLGraphic. */
struct SDLGraphic *graphic_new(void)
{
    struct SDLGraphic *graphic = malloc(sizeof(struct SDLGraphic));
    graphic->delay = 0;
    graphic->width = 0;
    graphic->height = 0;
    graphic->texture = NULL;
    return graphic;
}

/** Free an SDLGraphic. */
void graphic_free(struct SDLGraphic *graphic)
{
    SDL_DestroyTexture(graphic->texture);
    free(graphic);
}


/**
 * Construct a frame of a GIF.  START will be updated to point to the
 * last processed graphic.  NEXTFRAME will be updated to contain the basis for
 * the next frame.
 */
SDL_Surface *
_make_frame(
    LinkedList const **restrict start,
    SDL_Surface **restrict nextframe,
    GIF const *restrict gif)
{
    LinkedList const *const start_orig = *start;
    LinkedList *surfacegraphics = NULL;

    /* Step through graphics until we find a graphic with a nonzero delay time,
     * which marks the start of a new frame. */
    for (; (*start)->next != NULL; *start = (*start)->next)
    {
        struct GIF_Graphic const *const graphic = (*start)->data;
        struct SurfaceGraphic *g = surfacegraphic_from_graphic(
            graphic, gif->global_color_table);
        linkedlist_append(&surfacegraphics, linkedlist_new(g));
        if (graphic->extension && graphic->extension->delay_time != 0)
            break;
    }
    struct GIF_Graphic const *const graphic = (*start)->data;
    struct SurfaceGraphic *g = surfacegraphic_from_graphic(
        graphic, gif->global_color_table);
    linkedlist_append(&surfacegraphics, linkedlist_new(g));

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

GraphicList graphiclist_new_from_gif(SDL_Renderer *renderer, GIF gif)
{
    SDL_Surface *lastframe = SDL_CreateRGBSurfaceWithFormat(
        0, gif.width, gif.height, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(lastframe, NULL, SDL_MapRGBA(lastframe->format, 0, 0, 0, 0));

    GraphicList out = NULL;
    for (LinkedList const *node = gif.graphics; node; node = node->next)
    {
        SDL_Surface *frame = _make_frame(&node, &lastframe, &gif);

        struct GIF_Graphic *g = node->data;
        struct SDLGraphic *frame_g = graphic_new();
        frame_g->delay = g->extension? g->extension->delay_time : 0;
        frame_g->width = frame->w;
        frame_g->height = frame->h;
        frame_g->texture = SDL_CreateTextureFromSurface(renderer, frame);

        SDL_FreeSurface(frame);

        linkedlist_append(&out, linkedlist_new(frame_g));
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
        graphic_free(node->data);
        GraphicList next = node->next;
        free(node);
        if (node == graphics)
            break;
        node = next;
    }
}
