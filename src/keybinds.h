/*
 * keybinds.h -- Key bindings declarations.
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

#ifndef GIFVIEW_KEYBINDS_H
#define GIFVIEW_KEYBINDS_H

#include "util.h"

#include <stdbool.h>

#include <SDL2/SDL_keyboard.h>


/** Keybind data. */
struct KeyBind
{
    SDL_Keycode code;
    SDL_Keymod modmask;
};

/** Key actions. */
struct Action
{
    char const *name;
    void (*action)(void *);
    struct KeyBind *primary;
    struct KeyBind *secondary;
    struct KeyBind *tertiary;
};


/** Reset default keybinds, read keyconf files. */
void keybinds_init(void);

/** Set ACTION's keybinds. */
void action_set_keybinds(
    struct Action *action, struct KeyBind primary, struct KeyBind secondary,
    struct KeyBind tertiary);

/** Return true if any of ACTION's KeyBinds matches EVENT. */
bool action_ispressed(struct Action action, SDL_Keysym event);


#endif /* GIFVIEW_KEYBINDS_H */
