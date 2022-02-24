/*
 * gif.h -- GIF reading declarations.
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

#ifndef GIFVIEW_GIF_H
#define GIFVIEW_GIF_H

#include "util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* GIF Versions */
enum GIF_Version
{
    GIF_Version_Unknown,
    GIF_Version_87a,
    GIF_Version_89a,
};

/* GIF Color Table */
struct GIF_ColorTable
{
    bool sorted;
    size_t size;
    uint8_t *colors;
};

/* Image Descriptor */
struct GIF_Image
{
    uint16_t left, right, width, height;
    bool interlace_flag;

    struct GIF_ColorTable *color_table;

    size_t size;
    uint8_t *pixels;
};

/* Graphic extension */
struct GIF_GraphicExt
{
    enum DisposalMethod
    {
        GIF_DisposalMethod_Undefined,
        GIF_DisposalMethod_None,
        GIF_DisposalMethod_DoNotDispose,
        GIF_DisposalMethod_RestoreBackground,
        GIF_DisposalMethod_RestorePrevious,
    } disposal_method;
    bool user_input_flag;
    bool transparent_color_flag;
    uint16_t delay_time;
    uint8_t transparent_color_idx;
};

/* Plain Text extension */
struct GIF_PlainTextExt
{
    uint16_t tg_left, tg_top, tg_width, tg_height;
    uint8_t cell_width, cell_height;
    uint8_t fg_idx, bg_idx;
    size_t data_size;
    uint8_t *data;
};

/* Application extension */
struct GIF_ApplicationExt
{
    char appid[8];
    char auth_code[3];
    size_t data_size;
    uint8_t *data;
};

/* Graphic Block */
struct GIF_Graphic
{
    struct GIF_GraphicExt *extension;
    bool is_img;
    union
    {
        struct GIF_Image img;
        struct GIF_PlainTextExt plaintext;
    };
};

/* Container for GIF data. */
typedef struct GIF
{
    enum GIF_Version version;

    uint16_t width, height;
    uint8_t bg_color_index;

    uint8_t color_resolution;
    uint8_t pixel_aspect_ratio;

    struct GIF_ColorTable *global_color_table;

    LinkedList *graphics;
    LinkedList *comments;
    LinkedList *app_extensions;
} GIF;

/* Load a GIF from a file. */
GIF load_gif_from_file(char const *filename);

/* Deallocate GIF data. */
void free_gif(GIF gif);

#endif  // GIFVIEW_GIF_H
