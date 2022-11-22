/*
 * gifview - A GIF file viewer.
 * main.c -- Entry point for GIFView.
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

#include "args.h"
#include "gif.h"
#include "keybinds.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>


#if _WIN32
#define MAIN SDL_main
#else
#define MAIN main
#endif


/* SDL data for a GIF graphic. */
struct Graphic
{
    SDL_Texture *texture;
    int width, height;
    size_t delay;
};

/* SurfaceGraphic */
struct SurfaceGraphic
{
    SDL_Rect rect;
    SDL_Surface *surface;
};

/* SDL data for the app. */
struct SDLData
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bg_texture;
    int width, height;
};

/* Image draw data. */
struct DrawingData
{
    double zoom;
    int offset_x, offset_y;
};

/* Global app data. */
struct GlobalData
{
    bool running;
};


/* Keybind action functions. */
void zoom_in(void);
void zoom_out(void);
void reset_zoom(void);
void shift_up(void);
void shift_down(void);
void shift_right(void);
void shift_left(void);
void quit(void);

/* Size (in pixels) of background grid squares. */
static int const BACKGROUND_GRID_SIZE = 8;

/* Color for even-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_A[3] = {0x64, 0x64, 0x64};
/* Color for odd-numbered background grid squares. */
static uint8_t const BACKGROUND_GRID_COLOR_B[3] = {0x90, 0x90, 0x90};

/* Number of pixels to shift the image when shifting it using arrow keys. */
static int const SHIFT_AMOUNT = 2.5 * BACKGROUND_GRID_SIZE;

/* How much to zoom in/out when +/- are pressed. */
static int const ZOOM_CHANGE_MULTIPLIER = 2;

/* dd holds stuff for how/where to draw the image. */
static struct DrawingData dd = {
    .offset_x = 0,
    .offset_y = 0,
    .zoom = 1.0
};

/* GD holds global running data. */
static struct GlobalData GD = {
    .running = true
};

/* List of keybindable actions. */
struct Action actions[] = {
    {"zoom_in",     zoom_in,     NULL, NULL, NULL},
    {"zoom_out",    zoom_out,    NULL, NULL, NULL},
    {"reset_zoom",  reset_zoom,  NULL, NULL, NULL},
    {"shift_up",    shift_up,    NULL, NULL, NULL},
    {"shift_down",  shift_down,  NULL, NULL, NULL},
    {"shift_right", shift_right, NULL, NULL, NULL},
    {"shift_left",  shift_left,  NULL, NULL, NULL},
    {"quit",        quit,        NULL, NULL, NULL},
};

/* Length of actions array. */
size_t actions_count = sizeof(actions) / sizeof(*actions);


/* Generate a background grid texture. */
void generate_bg_grid(struct SDLData *G)
{
    SDL_Surface *grid_surf = SDL_CreateRGBSurfaceWithFormat(
        0,
        G->width, G->height,
        32,
        SDL_PIXELFORMAT_RGBA32);

    Uint32 const grid_color_a = SDL_MapRGB(
        grid_surf->format,
        BACKGROUND_GRID_COLOR_A[0],
        BACKGROUND_GRID_COLOR_A[1],
        BACKGROUND_GRID_COLOR_A[2]);
    Uint32 const grid_color_b = SDL_MapRGB(
        grid_surf->format,
        BACKGROUND_GRID_COLOR_B[0],
        BACKGROUND_GRID_COLOR_B[1],
        BACKGROUND_GRID_COLOR_B[2]);

    SDL_FillRect(grid_surf, NULL, grid_color_a);
    for (int y = 0; y < (G->height/BACKGROUND_GRID_SIZE+1); ++y)
    {
        int const initial_x = (y % 2 == 1)? 0 : BACKGROUND_GRID_SIZE;
        for (int x = initial_x; x < G->width; x += BACKGROUND_GRID_SIZE*2)
        {
            SDL_Rect const rect = {
                .h = BACKGROUND_GRID_SIZE,
                .w = BACKGROUND_GRID_SIZE,
                .x = x,
                .y = y * BACKGROUND_GRID_SIZE};
            SDL_FillRect(grid_surf, &rect, grid_color_b);
        }
    }
    SDL_DestroyTexture(G->bg_texture);
    G->bg_texture = SDL_CreateTextureFromSurface(G->renderer, grid_surf);
    SDL_FreeSurface(grid_surf);
}

/* 1/100 second timer callback.  Used to trigger frame changes in animated
 * GIFs. */
Uint32 timer_callback(Uint32 interval, void *param)
{
    SDL_Event event = {
        .type = SDL_USEREVENT,
        .user = {
            .code = 0,
            .data1 = NULL,
            .data2 = NULL,
            .type = SDL_USEREVENT}};
    SDL_PushEvent(&event);
    return interval;
}

/* Create SDL data. */
struct SDLData init_sdl(int window_width, int window_height)
{
    struct SDLData data;

    data.window = SDL_CreateWindow(
        "GIF View",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_RESIZABLE);
    if (data.window == NULL)
    {
        fatal("Failed to create window: %s\n", SDL_GetError());
    }

    data.renderer = SDL_CreateRenderer(
        data.window, -1, SDL_RENDERER_ACCELERATED);
    if (data.renderer == NULL)
    {
        fatal("Failed to create renderer -- %s\n", SDL_GetError());
    }
    SDL_GetWindowSize(data.window, &data.width, &data.height);
    generate_bg_grid(&data);
    return data;
}

/* Free SDL data. */
void free_sdl(struct SDLData const *data)
{
    SDL_DestroyTexture(data->bg_texture);
    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);
}

/* Create a SurfaceGraphic from a GIF_Graphic. */
struct SurfaceGraphic *mk_SDLSurface_from_GIFImage(struct GIF_Graphic graphic)
{
    if (!graphic.is_img)
    {
        return NULL;
    }
    struct GIF_Image image = graphic.img;
    struct SurfaceGraphic *out = malloc(sizeof(*out));
    out->rect.x = image.left;
    out->rect.y = image.right;
    out->rect.w = image.width;
    out->rect.h = image.height;
    out->surface = SDL_CreateRGBSurfaceWithFormatFrom(
        image.pixels,
        image.width, image.height,
        8,
        image.width,
        SDL_PIXELFORMAT_INDEX8);

    if (out->surface == NULL)
    {
        error("SDL_CreateRGBSurfaceWithFormatFrom: %s\n", SDL_GetError());
        free(out);
        return NULL;
    }

    if (graphic.extension)
    {
        if (graphic.extension->transparent_color_flag)
        {
            SDL_SetColorKey(
                out->surface,
                SDL_TRUE,
                graphic.extension->transparent_color_idx);
        }
    }

    struct GIF_ColorTable const *table = image.color_table;
    if (table == NULL)
    {
        warn("mk_SDLSurface_from_GIFImage: Image has no palette!\n");
        return out;
    }
    SDL_Color *colors = malloc(sizeof(SDL_Color) * table->size);
    for (size_t i = 0; i < table->size; ++i)
    {
        colors[i].r = table->colors[(3*i)+0];
        colors[i].g = table->colors[(3*i)+1];
        colors[i].b = table->colors[(3*i)+2];
        colors[i].a = 255;
    }

    if (SDL_SetPaletteColors(
            out->surface->format->palette, colors, 0, table->size)
        != 0)
    {
        error("SDL_SetPaletteColors -- %s\n", SDL_GetError());
    }
    free(colors);
    return out;
}

/* Generate a linked list of Graphics from a linked list of GIF_Graphics. */
LinkedList *make_graphics(struct SDLData G, GIF gif)
{
    SDL_Surface *frame = SDL_CreateRGBSurfaceWithFormat(
        0,
        gif.width, gif.height,
        32,
        SDL_PIXELFORMAT_RGBA32);
    SDL_FillRect(frame, NULL, SDL_MapRGBA(frame->format, 0, 0, 0, 0xff));

    LinkedList *images = NULL;
    for (LinkedList *node = gif.graphics; node != NULL; node = node->next)
    {
        struct GIF_Graphic *graphic = node->data;
        if (graphic->is_img)
        {
            struct SurfaceGraphic *g = mk_SDLSurface_from_GIFImage(*graphic);
            SDL_BlitSurface(g->surface, NULL, frame, &g->rect);
            if (graphic->extension)
            {
                struct Graphic *frameout = malloc(sizeof(*frameout));
                frameout->delay = graphic->extension->delay_time;
                frameout->width = gif.width;
                frameout->height = gif.height;
                frameout->texture = SDL_CreateTextureFromSurface(
                    G.renderer, frame);
                linkedlist_append(&images, linkedlist_new(frameout));
                if (node->next != NULL)
                {
                    SDL_Surface *newframe = SDL_CreateRGBSurfaceWithFormat(
                        0,
                        gif.width, gif.height,
                        32,
                        SDL_PIXELFORMAT_RGBA32);
                    SDL_BlitSurface(frame, NULL, newframe, NULL);
                    SDL_FreeSurface(frame);
                    frame = newframe;
                }
                else
                {
                    frame = NULL;
                }
            }
            SDL_FreeSurface(g->surface);
            free(g);
        }
        else
        {
            /* TODO: PlainText rendering */
        }
    }
    if (frame != NULL)
    {
        struct Graphic *frameout = malloc(sizeof(*frameout));
        frameout->delay = 0;
        frameout->texture = SDL_CreateTextureFromSurface(G.renderer, frame);
        linkedlist_append(&images, linkedlist_new(frameout));
        SDL_FreeSurface(frame);
    }
    /* Make the list circular, for free looping. */
    for (LinkedList *g = images; g != NULL; g = g->next)
    {
        if (g->next == NULL)
        {
            g->next = images;
            break;
        }
    }
    return images;
}

/* Free a linked list of Graphics. */
void free_graphics(LinkedList *graphics)
{
    for (LinkedList *node = graphics->next; node != NULL;)
    {
        struct Graphic *graphic = node->data;
        SDL_DestroyTexture(graphic->texture);
        LinkedList *next = node->next;
        free(graphic);
        free(node);
        if (node == graphics)
        {
            break;
        }
        node = next;
    }
}

/* Clear the screen. */
void clear_screen(struct SDLData G)
{
    SDL_RenderCopy(G.renderer, G.bg_texture, NULL, NULL);
}

/* Draw IMG to the screen. */
void draw_img(struct SDLData G, struct Graphic const *img)
{
    int imgw = img->width  * dd.zoom,
        imgh = img->height * dd.zoom;

    SDL_Rect position = {
        .h = imgh,
        .w = imgw,
        .x = dd.offset_x,
        .y = dd.offset_y};

    SDL_RenderCopy(G.renderer, img->texture, NULL, &position);
}

/* Ensure DD's offset_* fields don't put the image off the screen. */
void bounds_check_offsets(struct SDLData G, int img_w, int img_h)
{
    int const scaled_img_w = img_w * dd.zoom,
              scaled_img_h = img_h * dd.zoom;
    if (G.width >= scaled_img_w)
    {
        if (dd.offset_x < 0)
            dd.offset_x = 0;
        if (dd.offset_x > G.width - scaled_img_w)
            dd.offset_x = G.width - scaled_img_w;
    }
    else
    {
        if (dd.offset_x > 0)
            dd.offset_x = 0;
        if (dd.offset_x < G.width - scaled_img_w)
            dd.offset_x = G.width - scaled_img_w;
    }
    if (G.height >= scaled_img_h)
    {
        if (dd.offset_y < 0)
            dd.offset_y = 0;
        if (dd.offset_y > G.height - scaled_img_h)
            dd.offset_y = G.height - scaled_img_h;
    }
    else
    {
        if (dd.offset_y > 0)
            dd.offset_y = 0;
        if (dd.offset_y < G.height - scaled_img_h)
            dd.offset_y = G.height - scaled_img_h;
    }
}

/* Increase zoom level. */
void zoom_in(void)
{
    dd.zoom *= ZOOM_CHANGE_MULTIPLIER;
}

/* Decrease zoom level. */
void zoom_out(void)
{
    dd.zoom /= ZOOM_CHANGE_MULTIPLIER;
}

/* Reset zoom level. */
void reset_zoom(void)
{
    dd.zoom = 1.0;
}

/* Shift camera up. */
void shift_up(void)
{
    dd.offset_y -= SHIFT_AMOUNT;
}

/* Shift camera down. */
void shift_down(void)
{
    dd.offset_y += SHIFT_AMOUNT;
}

/* Shift camera right. */
void shift_right(void)
{
    dd.offset_x -= SHIFT_AMOUNT;
}

/* Shift camera left. */
void shift_left(void)
{
    dd.offset_x += SHIFT_AMOUNT;
}

/* Quit the app. */
void quit(void)
{
    GD.running = false;
}

/* Return true if BIND matches EVENT. */
bool keybind_ispressed(struct KeyBind const *bind, SDL_Keysym event)
{
    if (bind == NULL || bind->code != event.sym)
    {
        return false;
    }

    if (bind->modmask == KMOD_NONE)
    {
        return event.mod == KMOD_NONE;
    }

    SDL_Keymod const othermask = KMOD_NUM | KMOD_CAPS | KMOD_MODE | KMOD_SCROLL;

#define FILTERMASK(b, e, filter) ((b & filter) == (e & filter))
    /* We can't just check if the masks are equal, because this would require
     * the user to press the left AND right modifier keys.  This is probably not
     * what the event specifiction actually wants, so we have to do some extra
     * checks to handle this correctly.  Here we do the naive behaviour, which
     * works fine if the event specifies that only left/right side should work.
     */
    bool shift = FILTERMASK(bind->modmask, event.mod, KMOD_SHIFT);
    bool ctrl = FILTERMASK(bind->modmask, event.mod, KMOD_CTRL);
    bool alt = FILTERMASK(bind->modmask, event.mod, KMOD_ALT);
    bool meta = FILTERMASK(bind->modmask, event.mod, KMOD_GUI);
    bool other = FILTERMASK(bind->modmask, event.mod, othermask);
#undef FILTERMASK

    /* The above booleans will require BOTH sides to be pressed if the event
     * says that either side is fine, which is not what we want.  To do the
     * Right Thing, we check here if the event spec says either side is fine,
     * and check only that *one of* the sides is pressed. */
    if ((bind->modmask & KMOD_SHIFT) == KMOD_SHIFT)
    {
        shift = event.mod & KMOD_SHIFT;
    }
    if ((bind->modmask & KMOD_CTRL) == KMOD_CTRL)
    {
        ctrl = event.mod & KMOD_CTRL;
    }
    if ((bind->modmask & KMOD_ALT) == KMOD_ALT)
    {
        alt = event.mod & KMOD_ALT;
    }
    if ((bind->modmask & KMOD_GUI) == KMOD_GUI)
    {
        meta = event.mod & KMOD_GUI;
    }

    return shift && ctrl && alt && meta && other;
}

/* Return true if any of ACTION's KeyBinds matches EVENT. */
bool action_ispressed(struct Action action, SDL_Keysym event)
{
    return (
        keybind_ispressed(action.primary, event)
        || keybind_ispressed(action.secondary, event)
        || keybind_ispressed(action.tertiary, event));
}


int MAIN(int argc, char *argv[])
{
    char const *const filename = parse_args(argc, argv);
    GIF gif = load_gif_from_file(filename);

    for (LinkedList *node = gif.comments; node != NULL; node = node->next)
    {
        printf("Comment: '%s'\n", (char const *)node->data);
    }
    for (LinkedList *node = gif.app_extensions; node != NULL; node = node->next)
    {
        struct GIF_ApplicationExt const *ext = node->data;
        printf("App Extension: %.8s%.3s (%zu data bytes)\n",
            ext->appid, ext->auth_code, ext->data_size);
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    /* G holds all the SDL stuff. */
    struct SDLData G = init_sdl(gif.width, gif.height);

    init_keybinds();

    /* Generate SDL_Surfaces from the GIF's graphics. */
    LinkedList *images = make_graphics(G, gif);

    /* Set initial dd settings. */
    dd.offset_x = G.width /2 - gif.width /2;
    dd.offset_y = G.height/2 - gif.height/2;
    dd.zoom = 1.0;

    /* Clear the screen. */
    clear_screen(G);

    /* Current frame of the GIF's animation. */
    LinkedList *current_frame = images;

    /* Start the frame update timer. */
    SDL_TimerID timer = SDL_AddTimer(10, timer_callback, NULL);
    size_t timer_increment = 0;

    bool screen_dirty = true;
    while (GD.running)
    {
        if (screen_dirty)
        {
            struct Graphic *img = current_frame->data;
            clear_screen(G);
            draw_img(G, img);
            SDL_RenderPresent(G.renderer);
            screen_dirty = false;
        }

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            quit();
            break;

        case SDL_USEREVENT:
            struct Graphic const *image = current_frame->data;
            timer_increment++;
            if (timer_increment >= image->delay)
            {
                current_frame = current_frame->next;
                timer_increment = 0;
                screen_dirty = true;
            }
            break;

        case SDL_WINDOWEVENT:
            screen_dirty = true;
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                G.width  = event.window.data1;
                G.height = event.window.data2;
                /* Recenter image when window changes size. */
                dd.offset_x = G.width /2 - dd.zoom*gif.width /2;
                dd.offset_y = G.height/2 - dd.zoom*gif.height/2;
                generate_bg_grid(&G);
            }
            break;

        case SDL_KEYDOWN:
            for (size_t i = 0; i < actions_count; ++i)
            {
                if (action_ispressed(actions[i], event.key.keysym))
                {
                    actions[i].action();
                }
            }
            screen_dirty = true;
            break;

        case SDL_MOUSEMOTION:
            if (event.motion.state & SDL_BUTTON_LMASK)
            {
                dd.offset_x += event.motion.xrel;
                dd.offset_y += event.motion.yrel;
                screen_dirty = true;
            }
            break;
        }
        bounds_check_offsets(G, gif.width, gif.height);
    }

    free_graphics(images);

    SDL_RemoveTimer(timer);
    free_sdl(&G);
    SDL_Quit();

    free_gif(gif);

    return EXIT_SUCCESS;
}
