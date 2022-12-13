/*
 * imagetransform.c -- ImageTransform struct.
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

#include "imagetransform.h"


struct Rect
{
    int left, right, top, bottom;
};


void imagetransform_clamp(
    struct ImageTransform *transform,
    int img_w, int img_h,
    int max_x, int max_y)
{
    int const half_max_x = max_x / 2;
    int const half_max_y = max_y / 2;

    int const scaled_img_w = img_w * transform->zoom;
    int const scaled_img_h = img_h * transform->zoom;
    int const half_scaled_img_w = scaled_img_w / 2;
    int const half_scaled_img_h = scaled_img_h / 2;

    struct Rect const img_rect = {
        transform->offset_x + half_max_x - half_scaled_img_w,
        transform->offset_x + half_max_x + half_scaled_img_w,
        transform->offset_y + half_max_y - half_scaled_img_h,
        transform->offset_y + half_max_y + half_scaled_img_h
    };
    struct Rect const win_rect = {0, max_x, 0, max_y};

    /* When the image is smaller than the bounding box, the image's bounding
     * box should stay within the window bounding box.  When the image is
     * larger than the bounding box, the image's bounding box should contain
     * the window bounding box. */
    struct Rect const *big_x = max_x >= scaled_img_w? &win_rect : &img_rect;
    struct Rect const *lil_x = max_x >= scaled_img_w? &img_rect : &win_rect;
    struct Rect const *big_y = max_y >= scaled_img_h? &win_rect : &img_rect;
    struct Rect const *lil_y = max_y >= scaled_img_h? &img_rect : &win_rect;

    if (lil_x->left < big_x->left)
        transform->offset_x = win_rect.left - half_max_x + half_scaled_img_w;
    if (lil_x->right > big_x->right)
        transform->offset_x = win_rect.right - half_max_x - half_scaled_img_w;

    if (lil_y->top < big_y->top)
        transform->offset_y = win_rect.top - half_max_y + half_scaled_img_h;
    if (lil_y->bottom > big_y->bottom)
        transform->offset_y = win_rect.bottom - half_max_y - half_scaled_img_h;
}
