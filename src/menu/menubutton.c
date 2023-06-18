/*
 * menubutton.c -- MenuButton struct.
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

#include "menubutton.h"
#include "font.h"
#include "util.h"

#include <SDL2/SDL_ttf.h>


static int const INNER_PADDING = 3;
static SDL_Color const TEXT_COLOR = {.r=0x00, .g=0x00, .b=0x00, .a=0xFF};
static SDL_Color const HOVERED_COLOR = {.r=0x7F, .g=0x7F, .b=0x7F, .a=0xFF};


struct MenuButton
{
    /* Rect for overlap detection. */
    SDL_Rect rect;
    /* Rect for pasting the texture. x & y are synced with RECT, offset by
     * INNER_PADDING */
    SDL_Rect visrect;
    SDL_Renderer *renderer;
    SDL_Texture *text;
    bool is_hovered;
    BoundFunction on_click;
    Signal *signal_changed;
};

void settext(MenuButton *btn, char const *label);


MenuButton *
menubutton_new(char const *label, BoundFunction on_click, SDL_Renderer *R)
{
    MenuButton *btn = malloc(sizeof(MenuButton));
    btn->rect = (SDL_Rect){
        .x=0, .y=0,
        .w=2 * INNER_PADDING, .h=2 * INNER_PADDING
    };
    btn->visrect = (SDL_Rect){.x=INNER_PADDING, .y=INNER_PADDING, .w=0, .h=0};
    btn->renderer = R;
    btn->text = NULL;
    btn->is_hovered = false;
    btn->on_click = on_click;
    btn->signal_changed = signal_new();

    menubutton_set_label(btn, label);
    return btn;
}


void menubutton_free(MenuButton *btn)
{
    SDL_DestroyTexture(btn->text);
    free(btn);
}


void menubutton_draw(MenuButton *btn)
{
    if (btn->is_hovered)
    {
        SDL_SetRenderDrawColor(
            btn->renderer,
            HOVERED_COLOR.r, HOVERED_COLOR.g, HOVERED_COLOR.b, HOVERED_COLOR.a);
        SDL_RenderFillRect(btn->renderer, &btn->rect);
    }
    SDL_RenderCopy(btn->renderer, btn->text, NULL, &btn->visrect);
}


bool menubutton_handle_event(MenuButton *btn, SDL_Event event)
{
    switch (event.type)
    {
    case SDL_MOUSEMOTION:{
        SDL_Point cursor = {.x=event.motion.x, .y=event.motion.y};
        bool is_hovered = SDL_PointInRect(&cursor, &btn->rect);
        if (is_hovered != btn->is_hovered)
        {
            btn->is_hovered = is_hovered;
            return true;
        }
        break;}

    case SDL_MOUSEBUTTONUP:{
        SDL_Point cursor = {.x=event.button.x, .y=event.button.y};
        if (SDL_PointInRect(&cursor, &btn->rect))
        {
            boundfunction_invoke(btn->on_click);
            return true;
        }
        break;}
    }
    return false;
}


void menubutton_translate(MenuButton *btn, int dx, int dy)
{
    btn->rect.x += dx;
    btn->rect.y += dy;
    btn->visrect.x += dx;
    btn->visrect.y += dy;
}


void menubutton_set_rect(MenuButton *btn, SDL_Rect rect)
{
    btn->rect = rect;
    btn->visrect.x = btn->rect.x + INNER_PADDING;
    btn->visrect.y = btn->rect.y + INNER_PADDING;
}

SDL_Rect menubutton_get_rect(MenuButton *btn)
{
    return (SDL_Rect){
        .x=0,
        .y=0,
        .w=btn->visrect.w + 2 * INNER_PADDING,
        .h=btn->visrect.h + 2 * INNER_PADDING,
    };
}


void menubutton_set_label(MenuButton *btn, char const *label)
{
    TTF_Font *font = TTF_OpenFont(DEFAULT_FONT_PATH, DEFAULT_FONT_SIZE);
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, label, TEXT_COLOR);
    if (!surface)
        error("TTF_RenderUTF8_Blended -- %s\n", TTF_GetError());
    TTF_CloseFont(font);

    int width = surface? surface->w : 0;
    int height = surface? surface->h : 0;

    btn->rect.w = width + 2 * INNER_PADDING;
    btn->rect.h = height + 2 * INNER_PADDING;
    btn->visrect.w = width;
    btn->visrect.h = height;
    btn->text = SDL_CreateTextureFromSurface(btn->renderer, surface);
    if (!btn->text)
        error("SDL_CreateTextureFromSurface -- %s\n", SDL_GetError());

    SDL_FreeSurface(surface);
    signal_emit(btn->signal_changed);
}


Signal *menubutton_signal_changed(MenuButton *btn)
{
    return btn->signal_changed;
}
