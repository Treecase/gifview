/*
 * sdlapp.h -- App struct.
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

#ifndef _GIFVIEW_SDLAPP_H
#define _GIFVIEW_SDLAPP_H

#include "sdlgif.h"
#include "viewer/viewer.h"
#include "fontrenderer.h"

#include <SDL2/SDL.h>


/**
 * SDL-specific app data.  Acts as a view/controller for a Viewer.
 */
struct App
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *bg_texture;
    struct TextRenderer *paused_text, *looping_text, *playback_speed_text;
    int width, height;
    struct Viewer view;
    GraphicList images, current_frame;
    /** Time since last frame (in 100ths of a second [centiseconds]). */
    double timer;
    /** Total length of the animation (in 100ths of a second). */
    double full_time;
    /** Is the state display text visible? */
    bool state_text_visible;
};


/** Create SDL data. */
struct App app_new(GIF const *gif);

/** Free SDL data. */
void app_free(struct App const *app);

/** Clear the screen. */
void app_clear_screen(struct App *app);

/** Increment the timer, returning true if we've moved to the next frame. */
bool app_timer_increment(struct App *app);

/**
 * Move to the next frame.  (Normally done automatically by timer_increment.
 * Use this if you want to change frames manually, eg. by user input.)
 */
void app_next_frame(struct App *app);

/** Move to the previous frame. */
void app_previous_frame(struct App *app);

/** Draw the screen. */
void app_draw(struct App *app);

/** Resize the screen. */
void app_resize(struct App *app, int width, int height);

/** Set app paused state. */
void app_set_paused(struct App *app, bool paused);

/** Set app looping state. */
void app_set_looping(struct App *app, bool looping);

/** Set app playback speed. */
void app_set_playback_speed(struct App *app, double playback_speed);


#endif /* _GIFVIEW_SDLAPP_H */
