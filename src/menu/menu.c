/*
 * menu.c -- App Menu struct.
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

#include "menu.h"
#include "font.h"
#include "menubutton.h"
#include "util.h"

#include <SDL2/SDL_ttf.h>

#include <stdlib.h>
#include <string.h>


static int const BORDER = 1;
static int const PADDING = 2;
static SDL_Color const OUTLINE_COLOR = {.r=0x00, .g=0x00, .b=0x00, .a=0xFF};
static SDL_Color const FILL_COLOR = {.r=0xFF, .g=0xFF, .b=0xFF, .a=0xFF};


struct Menu
{
    SDL_Rect rect;
    SDL_Renderer *renderer;
    size_t items_count, items_size;
    struct MenuButton **items;
    bool is_visible;
};

void _on_changed(Menu *);



Menu *menu_new(SDL_Renderer *R)
{
    Menu *menu = malloc(sizeof(Menu));
    menu->rect = (SDL_Rect){
        .x=0,
        .y=0,
        .w=2 * PADDING + 2 * BORDER,
        .h=2 * PADDING + 2 * BORDER,
    };
    menu->renderer = R;
    menu->items_count = 0;
    menu->items_size = 0;
    menu->items = NULL;
    menu->is_visible = false;
    return menu;
}


void menu_free(Menu *menu)
{
    for (size_t i = 0; i < menu->items_count; ++i)
        menubutton_free(menu->items[i]);
    free(menu->items);
    free(menu);
}


void menu_draw(Menu *menu)
{
    if (!menu->is_visible)
        return;

    SDL_SetRenderDrawColor(
        menu->renderer,
        FILL_COLOR.r, FILL_COLOR.g, FILL_COLOR.b, FILL_COLOR.a);
    SDL_RenderFillRect(menu->renderer, &menu->rect);

    SDL_SetRenderDrawColor(
        menu->renderer,
        OUTLINE_COLOR.r, OUTLINE_COLOR.g, OUTLINE_COLOR.b, OUTLINE_COLOR.a);
    SDL_RenderDrawRect(menu->renderer, &menu->rect);

    for (size_t i = 0; i < menu->items_count; ++i)
        menubutton_draw(menu->items[i]);
}


bool menu_handle_event(Menu *menu, SDL_Event event)
{
    static bool is_show_click = false;
    bool handled = false;

    if (!menu->is_visible)
    {
        switch (event.type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                menu_move_to(menu, event.button.x, event.button.y);
                menu->is_visible = true;
                handled = true;
                is_show_click = true;
            }
            break;
        }
    }
    else
    {
        for (size_t i = 0; i < menu->items_count; ++i)
            handled = menubutton_handle_event(menu->items[i], event) || handled;

        if (!handled)
        {
            switch (event.type)
            {
            case SDL_MOUSEBUTTONUP:
                if (!is_show_click)
                {
                    menu->is_visible = false;
                    handled = true;
                }
                else
                    is_show_click = false;
                break;
            }
        }
    }

    return handled;
}


void menu_add_button(Menu *menu, MenuButton *button)
{
    if (menu->items_count + 1 >= menu->items_size)
    {
        menu->items_size += 10;
        menu->items = realloc(menu->items, menu->items_size);
    }
    menu->items[menu->items_count++] = button;
    signal_connect(menubutton_signal_changed(button), bind(_on_changed, menu));
    _on_changed(menu);
}


void menu_move_to(Menu *menu, int x, int y)
{
    int dx = x - menu->rect.x;
    int dy = y - menu->rect.y;

    menu->rect.x += dx;
    menu->rect.y += dy;
    for (size_t i = 0; i < menu->items_count; ++i)
        menubutton_translate(menu->items[i], dx, dy);
}



/** Recalculate menu rect. */
void _on_changed(Menu *menu)
{
    SDL_Rect textrect = {.x=0, .y=0, .w=0, .h=0};

    int yoff[menu->items_count];

    for (size_t i = 0; i < menu->items_count; ++i)
    {
        yoff[i] = textrect.h;
        SDL_Rect btn_rect = menubutton_get_rect(menu->items[i]);
        textrect.w = btn_rect.w > textrect.w? btn_rect.w : textrect.w;
        textrect.h += btn_rect.h;
    }

    for (size_t i = 0; i < menu->items_count; ++i)
    {
        SDL_Rect newrect = menubutton_get_rect(menu->items[i]);
        newrect.x = menu->rect.x + PADDING + BORDER;
        newrect.y = menu->rect.y + yoff[i] + PADDING + BORDER;
        newrect.w = textrect.w;
        menubutton_set_rect(menu->items[i], newrect);
    }

    menu->rect.w = textrect.w + 2 * PADDING + 2 * BORDER;
    menu->rect.h = textrect.h + 2 * PADDING + 2 * BORDER;
}
