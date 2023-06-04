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
#include "util.h"

#include <SDL2/SDL_ttf.h>

#include <stdlib.h>
#include <string.h>


struct MenuBuilder_
{
    struct MenuItemDef **items;
    size_t items_size;
    size_t items_count;
};

struct MenuItem
{
    SDL_Rect rect, visrect;
    SDL_Texture *normal;
    SDL_Texture *hovered;
    bool is_hovered;
    void (*on_click)(void *);
    void *on_click_data;
};

struct Menu_
{
    SDL_Rect rect;
    SDL_Texture *background;
    SDL_Renderer *renderer;
    size_t items_count;
    struct MenuItem **items;
    bool is_visible;
};


/* ===[ MenuItem ]=== */
struct MenuItem *menuitem_new(
    struct MenuItemDef *def, TTF_Font *font, SDL_Renderer *R)
{
    static int const inner_padding = 3;
    static SDL_Color const text_color = {.r=0x00, .g=0x00, .b=0x00, .a=0xFF};
    static SDL_Color const hovered_color = {.r=0x7F, .g=0x7F, .b=0x7F, .a=0xFF};

    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, def->label, text_color);
    SDL_Rect textrect = {
        .x=inner_padding,
        .y=inner_padding,
        .w=surface->w,
        .h=surface->h,
    };

    SDL_Surface *normal = SDL_CreateRGBSurface(
        0,
        surface->w + 2 * inner_padding,
        surface->h + 2 * inner_padding,
        32, 0, 0, 0, 0);
    SDL_FillRect(
        normal, NULL, SDL_MapRGBA(normal->format, 0xFF, 0xFF, 0xFF, 0x00));
    SDL_BlitSurface(surface, NULL, normal, &textrect);

    SDL_Surface *hovered = SDL_CreateRGBSurface(
        0, normal->w, normal->h, 32, 0, 0, 0, 0);
    SDL_FillRect(
        hovered,
        NULL,
        SDL_MapRGBA(
            hovered->format,
            hovered_color.r,
            hovered_color.g,
            hovered_color.b,
            hovered_color.a));
    SDL_BlitSurface(surface, NULL, hovered, &textrect);

    struct MenuItem *item = malloc(sizeof(struct MenuItem));
    item->rect = normal->clip_rect;
    item->visrect = normal->clip_rect;
    item->normal = SDL_CreateTextureFromSurface(R, normal);
    item->hovered = SDL_CreateTextureFromSurface(R, hovered);
    item->is_hovered = false;
    item->on_click = def->callback;
    item->on_click_data = def->callback_data;

    SDL_FreeSurface(surface);
    SDL_FreeSurface(normal);
    SDL_FreeSurface(hovered);
    return item;
}

void menuitem_free(struct MenuItem *item)
{
    SDL_DestroyTexture(item->normal);
    SDL_DestroyTexture(item->hovered);
    free(item);
}

void menuitem_draw(struct MenuItem *item, SDL_Renderer *R)
{
    SDL_Texture *texture = item->is_hovered? item->hovered : item->normal;
    SDL_RenderCopy(R, texture, NULL, &item->visrect);
}

bool menuitem_handle_event(struct MenuItem *item, SDL_Event event)
{
    switch (event.type)
    {
    case SDL_MOUSEMOTION:{
        SDL_Point cursor = {.x=event.motion.x, .y=event.motion.y};
        bool is_hovered = SDL_PointInRect(&cursor, &item->rect);
        if (is_hovered != item->is_hovered)
        {
            item->is_hovered = is_hovered;
            return true;
        }
        break;}

    case SDL_MOUSEBUTTONUP:{
        SDL_Point cursor = {.x=event.button.x, .y=event.button.y};
        if (SDL_PointInRect(&cursor, &item->rect))
        {
            if (item->on_click)
                item->on_click(item->on_click_data);
            return true;
        }
        break;}
    }
    return false;
}


/* ===[ MenuBuilder ]=== */
MenuBuilder *menubuilder_new(void)
{
    MenuBuilder *mb = malloc(sizeof(MenuBuilder));
    mb->items = NULL;
    mb->items_size = 0;
    mb->items_count = 0;
    return mb;
}

void menubuilder_free(MenuBuilder *mb)
{
    for (size_t i = 0; i < mb->items_count; ++i)
        free(mb->items[i]->label);
    free(mb->items);
    free(mb);
}

void menubuilder_add_item(MenuBuilder *mb, struct MenuItemDef def)
{
    if (mb->items == NULL)
    {
        mb->items_size = 1;
        mb->items = malloc(mb->items_size * sizeof(*mb->items));
    }
    if (mb->items_count + 1 > mb->items_size)
    {
        mb->items_size += 10;
        mb->items = realloc(mb->items, mb->items_size * sizeof(*mb->items));
    }
    struct MenuItemDef *def2 = malloc(sizeof(struct MenuItemDef));
    def2->label = strdup(def.label);
    def2->callback = def.callback;
    def2->callback_data = def.callback_data;
    mb->items[mb->items_count++] = def2;
}


/* ===[ Menu ]=== */
Menu *menu_new(MenuBuilder *def, SDL_Renderer *R)
{
    static int const border = 1;
    static int const padding = 2;
    static SDL_Color const outline_color = {.r=0x00, .g=0x00, .b=0x00, .a=0xFF};
    static SDL_Color const fill_color = {.r=0xFF, .g=0xFF, .b=0xFF, .a=0xFF};

    Menu *menu = malloc(sizeof(Menu));
    menu->renderer = R;
    menu->items_count = def->items_count;
    menu->items = malloc(menu->items_count * sizeof(struct MenuItem));
    menu->is_visible = false;

    /* Create MenuItems. */
    SDL_Rect textrect = {.x=0, .y=0, .w=0, .h=0};
    TTF_Font *font = TTF_OpenFont(DEFAULT_FONT_PATH, DEFAULT_FONT_SIZE);
    for (size_t i = 0; i < def->items_count; ++i)
    {
        struct MenuItem *item = menuitem_new(def->items[i], font, R);
        item->rect.x = border + padding;
        item->rect.y = textrect.h + border + padding;
        item->visrect.x = border + padding;
        item->visrect.y = textrect.h + border + padding;
        textrect.w = item->rect.w > textrect.w? item->rect.w : textrect.w;
        textrect.h += item->rect.h;
        menu->items[i] = item;
    }
    TTF_CloseFont(font);

    for (size_t i = 0; i < menu->items_count; ++i)
        menu->items[i]->rect.w = textrect.w;

    /* Draw background image. */
    SDL_Surface *menu_surf = SDL_CreateRGBSurface(
        0,
        textrect.w + 2*padding + 2*border,
        textrect.h + 2*padding + 2*border,
        32, 0, 0, 0, 0);
    /* Outline */
    SDL_FillRect(
        menu_surf,
        NULL,
        SDL_MapRGB(
            menu_surf->format,
            outline_color.r, outline_color.g, outline_color.b));
    /* Fill */
    SDL_Rect fill_rect = {
        .x=border, .y=border,
        .w=menu_surf->w - 2 * border, .h=menu_surf->h - 2 * border};
    SDL_FillRect(
        menu_surf,
        &fill_rect,
        SDL_MapRGB(
            menu_surf->format, fill_color.r, fill_color.g, fill_color.b));

    menu->background = SDL_CreateTextureFromSurface(R, menu_surf);
    menu->rect = menu_surf->clip_rect;
    return menu;
}

void menu_free(Menu *menu)
{
    SDL_DestroyTexture(menu->background);
    for (size_t i = 0; i < menu->items_count; ++i)
        menuitem_free(menu->items[i]);
    free(menu->items);
    free(menu);
}

void menu_draw(Menu *menu)
{
    if (!menu->is_visible)
        return;
    SDL_Rect rect = menu->rect;
    SDL_RenderCopy(menu->renderer, menu->background, NULL, &rect);
    for (size_t i = 0; i < menu->items_count; ++i)
        menuitem_draw(menu->items[i], menu->renderer);
}

bool menu_handle_event(Menu *menu, SDL_Event event)
{
    bool handled = false;

    switch (event.type)
    {
    case SDL_MOUSEBUTTONDOWN:
        if (!menu->is_visible)
        {
            menu_move_to(menu, event.button.x, event.button.y);
            menu->is_visible = true;
            handled = true;
        }
        else
        {
            menu->is_visible = false;
            handled = true;
        }
        break;
    }

    for (size_t i = 0; i < menu->items_count; ++i)
        handled = menuitem_handle_event(menu->items[i], event) || handled;

    return handled;
}

void menu_move_to(Menu *menu, int x, int y)
{
    int dx = x - menu->rect.x;
    int dy = y - menu->rect.y;

    menu->rect.x += dx;
    menu->rect.y += dy;
    for (size_t i = 0; i < menu->items_count; ++i)
    {
        menu->items[i]->rect.x += dx;
        menu->items[i]->rect.y += dy;
        menu->items[i]->visrect.x += dx;
        menu->items[i]->visrect.y += dy;
    }
}
