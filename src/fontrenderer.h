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

#ifndef _GIFVIEW_FONTRENDERER_H
#define _GIFVIEW_FONTRENDERER_H

#include <SDL2/SDL.h>
#include <SDL_ttf.h>


/** SDL_ttf-based text renderer. */
struct TextRenderer
{
    TTF_Font *font;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;
};


/** Create a new textrenderer. */
struct TextRenderer *textrenderer_new(char const *file, int ptsize);

/** Free a textrenderer. */
void textrenderer_free(struct TextRenderer *text);

/** Set the textrenderer's text. */
void textrenderer_set_text(
    struct TextRenderer *text, SDL_Renderer *renderer, char const *utf8text);


#endif /* _GIFVIEW_FONTRENDERER_H */
