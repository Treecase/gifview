/*
 * menubutton.h -- MenuButton struct.
 *
 * Copyright (C) 2023 Trevor Last
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

#ifndef GIFVIEW_MENUBUTTON_H
#define GIFVIEW_MENUBUTTON_H

#include "boundfunction.h"
#include "signal.h"

#include <stdbool.h>

#include <SDL2/SDL.h>


typedef struct MenuButton MenuButton;


MenuButton *
menubutton_new(char const *label, BoundFunction on_click, SDL_Renderer *R);
void menubutton_free(MenuButton *btn);

void menubutton_draw(MenuButton *btn);
bool menubutton_handle_event(MenuButton *btn, SDL_Event event);

void menubutton_translate(MenuButton *btn, int dx, int dy);

void menubutton_set_rect(MenuButton *btn, SDL_Rect rect);
SDL_Rect menubutton_get_rect(MenuButton *btn);
void menubutton_set_label(MenuButton *btn, char const *label);

Signal *menubutton_signal_changed(MenuButton *btn);


#endif /* GIFVIEW_MENUBUTTON_H */
