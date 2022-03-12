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
    GIFVIEW_GLOBAL_CONFIG_DIR"/keys.conf";

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

/* keys.conf parser data. */
struct Parser
{
    size_t line_count;
    char *line;
    ssize_t i;
    ssize_t length;
};

/* Consume characters from P until a non-whitespace character is found.
 * P will be left pointing to said character. */
void skip_whitespace(struct Parser *p)
{
    for (; p->i < p->length && isspace(p->line[p->i]); p->i++)
    {
    }
}

/* Consume characters from P until a whitespace character is found, or if P
 * points to a " character, until another " is found. P will be left pointing
 * to the whitespace character, or the next character after the ". */
int read_key(struct Parser *p, char **key)
{
    int err = 0;
    ssize_t tok_start = p->i;
    if (p->line[p->i] == '"')
    {
        tok_start = ++p->i;
        while (p->i < p->length && p->line[p->i++] != '"')
        {
        }
        while (!isspace(p->line[p->i]))
        {
            p->i++;
            err = 1;
        }
    }
    else
    {
        while (p->i < p->length && !isspace(p->line[p->i++]))
        {
        }
    }
    *key = strndup(p->line+tok_start, p->i-tok_start-1);
    return err;
}

/* Parse a keyconf file, updating the keybinds in the global `actions` array. */
void parse_keyconf(FILE *file)
{
    struct Parser p = {
        .line_count = 0,
        .line = NULL,
        .i = 0,
        .length = 0
    };

    size_t size = 0;
    while ((p.length = getline(&p.line, &size, file)) != -1)
    {
        p.line_count++;
        p.i = 0;

        /* Skip leading whitespace. */
        skip_whitespace(&p);
        if (p.i >= p.length)
        {
            /* Blank line. */
            continue;
        }

        /* Skip comment lines. */
        if (p.line[p.i] == '#')
        {
            continue;
        }

        /* Read the action. */
        ssize_t tok_start = p.i;
        while (p.i < p.length && !isspace(p.line[p.i++]))
        {
        }
        char *action_name = strndup(p.line+tok_start, p.i-tok_start-1);
        struct Action *action = NULL;
        if (parse_action(action_name, &action))
        {
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid action '%s'\n",
                p.line_count, tok_start, action_name);
        }
        free(action_name);

        /* Inter-token whitespace. */
        skip_whitespace(&p);
        if (p.i >= p.length)
        {
            /* No keys, so the action is unbound. */
            set_keybind(action, UNBOUND, UNBOUND, UNBOUND);
            continue;
        }

        /* Read key 1. */
        char *key1 = NULL;
        struct KeyBind k1;
        int err = read_key(&p, &key1);
        err = parse_keybind(key1, &k1);
        switch (err)
        {
        case 1:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                p.line_count, tok_start, key1);
            break;
        case 2:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                p.line_count, tok_start, key1, SDL_GetError());
            break;
        }
        if (p.i >= p.length)
        {
            set_keybind(action, k1, UNBOUND, UNBOUND);
            continue;
        }
        free(key1);

        /* Inter-token whitespace. */
        skip_whitespace(&p);
        if (p.i >= p.length)
        {
            set_keybind(action, k1, UNBOUND, UNBOUND);
            continue;
        }

        /* Read key 2. */
        char *key2 = NULL;
        struct KeyBind k2;
        err = read_key(&p, &key2);
        err = parse_keybind(key2, &k2);
        switch (err)
        {
        case 1:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                p.line_count, tok_start, key2);
            break;
        case 2:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                p.line_count, tok_start, key2, SDL_GetError());
            break;
        }
        free(key2);
        if (p.i >= p.length)
        {
            set_keybind(action, k1, k2, UNBOUND);
            continue;
        }

        /* Inter-token whitespace. */
        skip_whitespace(&p);
        if (p.i >= p.length)
        {
            set_keybind(action, k1, k2, UNBOUND);
            continue;
        }

        /* Read key 3. */
        char *key3 = NULL;
        struct KeyBind k3;
        err = read_key(&p, &key3);
        err = parse_keybind(key3, &k3);
        switch (err)
        {
        case 1:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid modifier '%s'\n",
                p.line_count, tok_start, key3);
            break;
        case 2:
            fprintf(stderr, "ERROR:%zu,%zu -- Invalid keyname '%s' (%s)\n",
                p.line_count, tok_start, key3, SDL_GetError());
            break;
        }
        free(key3);
        set_keybind(action, k1, k2, k3);

        /* Make sure the rest of the line is blank. */
        skip_whitespace(&p);
        if (p.i < p.length)
        {
            fprintf(stderr,
                "ERROR:%zu,%zu -- Trailing non-whitespace characters",
                    p.line_count, p.i);
        }
    }
    free(p.line);
}


void set_keybind(
    struct Action *action, struct KeyBind primary, struct KeyBind secondary,
    struct KeyBind tertiary)
{
    if (action == NULL)
    {
        return;
    }
    free(action->primary);
    action->primary = NULL;
    if (primary.code != SDLK_UNKNOWN)
    {
        action->primary = malloc(sizeof(primary));
        memcpy(action->primary, &primary, sizeof(primary));
    }
    free(action->secondary);
    action->secondary = NULL;
    if (secondary.code != SDLK_UNKNOWN)
    {
        action->secondary = malloc(sizeof(secondary));
        memcpy(action->secondary, &secondary, sizeof(secondary));
    }
    free(action->tertiary);
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

    /* Start with global config... */
    FILE *conf = fopen(GLOBAL_KEYCONF_PATH, "r");
    if (conf)
    {
        parse_keyconf(conf);
        fclose(conf);
    }

    /* ..And now the local config. */
    char const *home = getenv("HOME");
    if (home)
    {
        char *confpath = estrcat(home, LOCAL_KEYCONF_PATH);
        conf = fopen(confpath, "r");
        if (conf)
        {
            parse_keyconf(conf);
            fclose(conf);
        }
        free(confpath);
    }
}
