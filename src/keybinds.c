/*
 * keybinds.c -- Keybindings definitions.
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

#include "keybinds.h"
#include "config.h"

#include <stdbool.h>

extern struct Action actions[];
extern size_t actions_count;

/* Macro for unbound keybinds. */
#define UNBOUND ((struct KeyBind){.code=SDLK_UNKNOWN})

/* Path to the global key config file. */
static char const *const GLOBAL_KEYCONF_PATH =\
    "/etc/"GIFVIEW_PROGRAM_NAME"/keys.conf";

/* Path to the local key config file (starting from $HOME). */
static char const *const LOCAL_KEYCONF_PATH =\
    "/.config/"GIFVIEW_PROGRAM_NAME"/keys.conf";

/* Helper struct used only for default_keybinds array. */
struct KeyDef
{
    char const *const name;
    struct KeyBind a,b,c;
};

/* Default keybinds. */
static struct KeyDef const default_keybinds[] = {
    {"zoom_in",     {SDLK_UP,         0}, {SDLK_KP_PLUS, 0},         UNBOUND},
    {"zoom_out",    {SDLK_DOWN,       0}, {SDLK_KP_MINUS,0},         UNBOUND},
    {"reset_zoom",  {SDLK_KP_MULTIPLY,0}, {SDLK_8,       KMOD_SHIFT},UNBOUND},
    {"shift_up",    {SDLK_KP_2,       0}, {SDLK_DOWN,    KMOD_CTRL}, UNBOUND},
    {"shift_down",  {SDLK_KP_8,       0}, {SDLK_UP,      KMOD_CTRL}, UNBOUND},
    {"shift_right", {SDLK_KP_6,       0}, {SDLK_RIGHT,   KMOD_CTRL}, UNBOUND},
    {"shift_left",  {SDLK_KP_4,       0}, {SDLK_LEFT,    KMOD_CTRL}, UNBOUND},
    {"quit",        {SDLK_ESCAPE,     0}, {SDLK_q,       0},         UNBOUND},
};

/* Number of items in the default_keybinds array. */
static size_t const default_keybinds_count = (
    sizeof(default_keybinds) / sizeof(*default_keybinds));


/* Parse an action string.  On success, PTR will be set to point to the
 * corresponding struct stored in the global `actions` array.  Returns 0 on
 * success, or 1 if there was an error. */
int parse_action(char const *action, struct Action **ptr)
{
    for (size_t i = 0; i < actions_count; ++i)
    {
        if (strcmp(actions[i].name, action) == 0)
        {
            *ptr = &actions[i];
            return 0;
        }
    }
    return 1;
}

/* Parse a keybind string.  On success, PTR will be set to the parsed keybind.
 * Returns 0 on success, 1 if the modifier is invalid, or 2 if the key name is
 * invalid.  If the key name is invalid, call SDL_GetError() for more
 * information. */
int parse_keybind(char const *keyname, struct KeyBind *bind)
{
    /* Parse modifier(s). */
    Uint16 modmask = KMOD_NONE;
    while (keyname[1] == '-')
    {
        switch (keyname[0])
        {
        case 'C':
            modmask |= KMOD_CTRL;
            break;
        case 'S':
            modmask |= KMOD_SHIFT;
            break;
        case 'A':
            modmask |= KMOD_ALT;
            break;
        case 'M':
            modmask |= KMOD_GUI;
            break;
        default:
            return 1;
            break;
        }
        keyname += 2;
    }
    /* Get the key code. */
    SDL_Keycode code = SDL_GetKeyFromName(keyname);
    if (code == SDLK_UNKNOWN)
    {
        return 2;
    }
    bind->code = code;
    bind->modmask = modmask;
    return 0;
}

/* Parse a keyconf file, updating the keybinds in the global `actions` array. */
void parse_keyconf(FILE *file)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t length = 0;

    size_t line_counter = 0;
    while ((length = getline(&line, &size, file)) != -1)
    {
        line_counter++;
        ssize_t i = 0;

        /* Skip leading whitespace. */
        while (i < length && isspace(line[i++]))
        {
        }
        if (i >= length)
        {
            /* Blank line. */
            continue;
        }

        /* Read the action. */
        ssize_t tok_start = i;
        while (i < length && !isspace(line[i++]))
        {
        }
        char *action_name = strndup(line+tok_start-1, i-tok_start);
        struct Action *action = NULL;
        if (parse_action(action_name, &action))
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid action '%s'\n",
                line_counter, tok_start, action_name);
        }
        free(action_name);

        /* Inter-token whitespace. */
        while (i < length && isspace(line[i++]))
        {
        }
        if (i >= length)
        {
            /* No keys, so the action is unbound. */
            set_keybind(action, UNBOUND, UNBOUND, UNBOUND);
            continue;
        }

        /* Read key 1. */
        tok_start = i;
        while (i < length && !isspace(line[i++]))
        {
        }
        char *key1 = strndup(line+tok_start-1, i-tok_start);
        struct KeyBind k1;
        int err = parse_keybind(key1, &k1);
        if (err == 1)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                line_counter, tok_start, key1);
        }
        else if (err == 2)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                line_counter, tok_start, key1, SDL_GetError());
        }
        free(key1);
        if (i >= length)
        {
            set_keybind(action, k1, UNBOUND, UNBOUND);
            continue;
        }

        /* Inter-token whitespace. */
        while (i < length && isspace(line[i++]))
        {
        }
        if (i >= length)
        {
            set_keybind(action, k1, UNBOUND, UNBOUND);
            continue;
        }

        /* Read key 2. */
        tok_start = i;
        while (i < length && !isspace(line[i++]))
        {
        }
        char *key2 = strndup(line+tok_start-1, i-tok_start);
        struct KeyBind k2;
        err = parse_keybind(key2, &k2);
        if (err == 1)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                line_counter, tok_start, key2);
        }
        else if (err == 2)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                line_counter, tok_start, key2, SDL_GetError());
        }
        free(key2);
        if (i >= length)
        {
            set_keybind(action, k1, k2, UNBOUND);
            continue;
        }

        /* Inter-token whitespace. */
        while (i < length && isspace(line[i++]))
        {
        }
        if (i >= length)
        {
            set_keybind(action, k1, k2, UNBOUND);
            continue;
        }

        /* Read key 3. */
        tok_start = i;
        while (i < length && !isspace(line[i++]))
        {
        }
        char *key3 = strndup(line+tok_start-1, i-tok_start);
        struct KeyBind k3;
        err = parse_keybind(key3, &k3);
        if (err == 1)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                line_counter, tok_start, key3);
        }
        else if (err == 2)
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                line_counter, tok_start, key3, SDL_GetError());
        }
        free(key3);
        set_keybind(action, k1, k2, k3);

        /* Make sure the rest of the line is blank. */
        while (i < length && isspace(line[i++]))
        {
        }
        if (i < length)
        {
            fprintf(stderr,
                "ERROR:%zu,%zu -- Trailing non-whitespace characters",
                    line_counter, i);
        }
    }
}


void set_keybind(
    struct Action *action, struct KeyBind primary, struct KeyBind secondary,
    struct KeyBind tertiary)
{
    if (action == NULL)
    {
        return;
    }
    action->primary = NULL;
    if (primary.code != SDLK_UNKNOWN)
    {
        action->primary = malloc(sizeof(primary));
        memcpy(action->primary, &primary, sizeof(primary));
    }
    action->secondary = NULL;
    if (secondary.code != SDLK_UNKNOWN)
    {
        action->secondary = malloc(sizeof(secondary));
        memcpy(action->secondary, &secondary, sizeof(secondary));
    }
    action->tertiary = NULL;
    if (tertiary.code != SDLK_UNKNOWN)
    {
        action->tertiary = malloc(sizeof(tertiary));
        memcpy(action->tertiary, &tertiary, sizeof(tertiary));
    }
}

void init_keybinds(void)
{
    /* Clear any previously set keybinds. */
    for (size_t i = 0; i < actions_count; ++i)
    {
        free(actions[i].primary);
        free(actions[i].secondary);
        free(actions[i].tertiary);
        actions[i].primary = NULL;
        actions[i].secondary = NULL;
        actions[i].tertiary = NULL;
    }

    /* Set default keybinds. */
    for (size_t i = 0; i < default_keybinds_count; ++i)
    {
        struct KeyDef const *const def = &default_keybinds[i];
        struct Action *action = NULL;
        if (parse_action(def->name, &action))
        {
            fprintf(stderr,
                "ERROR: init_keybinds -- default_keybinds contains an invalid"
                "action name '%s'\n",
                def->name);
            continue;
        }
        set_keybind(action, def->a, def->b, def->c);
    }

    /* Read keybindings from config files. First try the one at
     * ${HOME}/LOCAL_KEYCONF_PATH, otherwise falling back on the one at
     * GLOBAL_KEYCONF_PATH. */
    char const *home = getenv("HOME");
    char *confpath = NULL;
    if (home)
    {
        confpath = estrcat(home, LOCAL_KEYCONF_PATH);
    }

    FILE *conf = fopen(confpath, "r");
    if (conf == NULL)
    {
        conf = fopen(GLOBAL_KEYCONF_PATH, "r");
        if (conf == NULL)
        {
            /* No configs, use default keybinds. */
            return;
        }
    }
    free(confpath);

    parse_keyconf(conf);
    fclose(conf);
}
