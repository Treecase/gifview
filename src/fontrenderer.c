/*
 * fontrenderer.h -- SDL2 font drawing.
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

#include "fontrenderer.h"


struct TextRenderer *textrenderer_new(char const *file, int ptsize)
{
    struct TextRenderer *text = malloc(sizeof(struct TextRenderer));
    text->font = TTF_OpenFont(file, ptsize);
    text->surface = NULL;
    text->texture = NULL;
    return text;
}

void textrenderer_free(struct TextRenderer *text)
{
    TTF_CloseFont(text->font);
    SDL_FreeSurface(text->surface);
    SDL_DestroyTexture(text->texture);
    free(text);
}

void textrenderer_set_text(
    struct TextRenderer *text, SDL_Renderer *renderer, char const *utf8text)
{
    static SDL_Color const WHITE = {0xff, 0xff, 0xff, 0xff};
    static SDL_Color const BLACK = {0x00, 0x00, 0x00, 0xff};
    static int const OUTLINE = 2;

    TTF_SetFontOutline(text->font, OUTLINE);
    SDL_Surface *outlined = TTF_RenderUTF8_Blended(text->font, utf8text, BLACK);
    TTF_SetFontOutline(text->font, 0);
    SDL_Surface *base = TTF_RenderUTF8_Blended(text->font, utf8text, WHITE);

    SDL_Rect rect = base->clip_rect;
    rect.x += OUTLINE;
    rect.y += OUTLINE;
    SDL_BlitSurface(base, NULL, outlined, &rect);

    text->surface = outlined;
    text->texture = SDL_CreateTextureFromSurface(renderer, text->surface);
    text->rect.h = text->surface->h;
    text->rect.w = text->surface->w;
    text->rect.x = 0;
    text->rect.y = 0;
}
