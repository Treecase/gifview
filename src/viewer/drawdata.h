/*
 * drawdata.h -- DrawData struct.
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

#ifndef _GIFVIEW_DRAWDATA_H
#define _GIFVIEW_DRAWDATA_H


/** How to draw the image (scaling, offsets). */
struct DrawData
{
    double zoom;
    int offset_x, offset_y;
};


/**
 * Clamp DD's offsets such that an IMG_W x IMG_H sized image transformed by it
 * will be within a MAX_X x MAX_Y bounding-box.
 */
void drawdata_clamp(
    struct DrawData *dd, int img_w, int img_h, int max_x, int max_y);


#endif /* _GIFVIEW_DRAWDATA_H */
