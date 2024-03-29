/*
 * viewer.h -- Viewer struct.
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

#ifndef GIFVIEW_VIEWER_H
#define GIFVIEW_VIEWER_H

#include "imagetransform.h"

#include <stdbool.h>


/**
 * Abstract viewer struct.  Should be independent of any windowing/drawing
 * libraries.
 */
struct Viewer
{
    bool running;
    /**
     * Number of pixels to shift the image when shifting it using arrow keys.
     */
    int shift_amount;
    /** How much to zoom in/out when +/- are pressed. */
    double zoom_change_multiplier;
    /** Image positioning. */
    struct ImageTransform transform;
    /** Is playback paused? */
    bool paused;
    /** Is playback looping? */
    bool looping;
    /** Playback speed multiplier. */
    double playback_speed;
};


/** Increase zoom level. */
void viewer_zoom_in(struct Viewer *v);

/** Decrease zoom level. */
void viewer_zoom_out(struct Viewer *v);

/** Reset zoom level. */
void viewer_zoom_reset(struct Viewer *v);

/** Reset camera transform. */
void viewer_transform_reset(struct Viewer *v);

/** Translate camera by DX,DY pixels. */
void viewer_translate(struct Viewer *v, int dx, int dy);

/** Shift camera up. */
void viewer_shift_up(struct Viewer *v);

/** Shift camera down. */
void viewer_shift_down(struct Viewer *v);

/** Shift camera right. */
void viewer_shift_right(struct Viewer *v);

/** Shift camera left. */
void viewer_shift_left(struct Viewer *v);

/** Quit the app. */
void viewer_quit(struct Viewer *v);

/** Return true if the timer should be allowed to increment, false otherwise. */
bool viewer_should_timer_increment(struct Viewer *v);


#endif /* GIFVIEW_VIEWER_H */
