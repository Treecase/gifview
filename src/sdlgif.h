/*
 * sdlgif.h -- SDL representations of GIF data.
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

#ifndef GIFVIEW_SDLGIF_H
#define GIFVIEW_SDLGIF_H

#include "util.h"
#include "gif/gif.h"

#include <SDL2/SDL.h>


/** SDL data for a GIF graphic.  Represents a complete frame of a GIF. */
struct SDLGraphic
{
    SDL_Texture *texture;
    int width, height;
    size_t delay;
};


/** LinkedList<SDLGraphic>. */
typedef LinkedList *GraphicList;


/** Generate a linked list of Graphics from a linked list of GIF_Graphics. */
GraphicList graphiclist_new_from_gif(SDL_Renderer *renderer, GIF gif);

/** Free a linked list of Graphics. */
void graphiclist_free(GraphicList graphics);


#endif /* GIFVIEW_SDLGIF_H */
