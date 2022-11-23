/*
 * viewer.c-- Viewer struct.
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

#include "viewer.h"


void viewer_zoom_in(struct Viewer *v)
{
    v->dd.zoom *= v->zoom_change_multiplier;
}

void viewer_zoom_out(struct Viewer *v)
{
    v->dd.zoom /= v->zoom_change_multiplier;
}

void viewer_zoom_reset(struct Viewer *v)
{
    v->dd.zoom = 1.0;
}

void viewer_shift_up(struct Viewer *v)
{
    v->dd.offset_y -= v->shift_amount;
}

void viewer_shift_down(struct Viewer *v)
{
    v->dd.offset_y += v->shift_amount;
}

void viewer_shift_right(struct Viewer *v)
{
    v->dd.offset_x -= v->shift_amount;
}

void viewer_shift_left(struct Viewer *v)
{
    v->dd.offset_x += v->shift_amount;
}

void viewer_quit(struct Viewer *v)
{
    v->running = false;
}
