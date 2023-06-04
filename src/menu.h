/*
 * menu.h -- App Menu struct.
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

#ifndef GIFVIEW_MENU_H
#define GIFVIEW_MENU_H

#include <stdbool.h>

#include <SDL2/SDL.h>


struct MenuItemDef
{
    char *label;
    void (*callback)(void *);
    void *callback_data;
};

typedef struct MenuBuilder_ MenuBuilder;

MenuBuilder *menubuilder_new(void);
void menubuilder_free(MenuBuilder *mb);
void menubuilder_add_item(MenuBuilder *mb, struct MenuItemDef def);


/** App Right-click menu. */
typedef struct Menu_ Menu;

/** Allocate a new Menu. */
Menu *menu_new(MenuBuilder *def, SDL_Renderer *R);
/** Free a Menu. */
void menu_free(Menu *menu);
/** Draw the Menu. */
void menu_draw(Menu *menu);
/** Handle Menu input events. */
bool menu_handle_event(Menu *menu, SDL_Event event);
/** Move Menu's top-left corner to (x, y). */
void menu_move_to(Menu *menu, int x, int y);


#endif /* GIFVIEW_MENU_H */
