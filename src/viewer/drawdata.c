/*
 * drawdata.c -- DrawData struct.
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

#include "drawdata.h"


void drawdata_clamp(
    struct DrawData *dd, int img_w, int img_h, int max_x, int max_y)
{
    int const scaled_img_w = img_w * dd->zoom,
              scaled_img_h = img_h * dd->zoom;

    /* When the image is smaller than the bounding box, the image's bounding
     * box should stay within the window bounding box.  When the image is
     * larger than the bounding box, the image's bounding box should contain
     * the window bounding box. */
    if (max_x >= scaled_img_w)
    {
        if (dd->offset_x < 0)
            dd->offset_x = 0;
        if (dd->offset_x > max_x - scaled_img_w)
            dd->offset_x = max_x - scaled_img_w;
    }
    else
    {
        if (dd->offset_x > 0)
            dd->offset_x = 0;
        if (dd->offset_x < max_x - scaled_img_w)
            dd->offset_x = max_x - scaled_img_w;
    }
    if (max_y >= scaled_img_h)
    {
        if (dd->offset_y < 0)
            dd->offset_y = 0;
        if (dd->offset_y > max_y - scaled_img_h)
            dd->offset_y = max_y - scaled_img_h;
    }
    else
    {
        if (dd->offset_y > 0)
            dd->offset_y = 0;
        if (dd->offset_y < max_y - scaled_img_h)
            dd->offset_y = max_y - scaled_img_h;
    }
}
