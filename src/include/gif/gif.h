/*
 * gif.h -- GIF functions and struct.
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

#include "linkedlist/linkedlist.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/** GIF Versions. */
enum GIF_Version
{
    GIF_Version_Unknown,
    GIF_Version_87a,
    GIF_Version_89a,
};

/** GIF Color Table */
struct GIF_ColorTable
{
    /** If true, the table has been sorted, to assist decoders with limited
     * color palettes. */
    bool sorted;
    /** Number of colors in the table. */
    size_t size;
    /** Color table RGB triples. */
    uint8_t *colors;
};

/** Image Descriptor. */
struct GIF_Image
{
    /** Position of the top left corner of the image. */
    uint16_t left, top;
    /** Size of the image. */
    uint16_t width, height;
    /** Whether the image is interlaced or not. */
    bool interlace_flag;

    /**
     * Image's local color table, or the global color table if the image
     * doesn't have a local one.
     */
    struct GIF_ColorTable *color_table;

    /** Size of PIXELS in bytes. */
    size_t size;
    /** Decompressed image data. */
    uint8_t *pixels;
};

/** Graphic extension. */
struct GIF_GraphicExt
{
    /** Graphic disposal method. */
    enum DisposalMethod
    {
        /* Using 8 as undefined value because valid disposal method range is
         * 3 bits (ie. 0-7) */
        GIF_DisposalMethod_Undefined = 8,
        GIF_DisposalMethod_None = 0,
        GIF_DisposalMethod_DoNotDispose = 1,
        GIF_DisposalMethod_RestoreBackground = 2,
        GIF_DisposalMethod_RestorePrevious = 3,
    } disposal_method;
    /** Whether user input is needed to continue. */
    bool user_input_flag;
    /** If true, transparent_color_idx will have a value. */
    bool transparent_color_flag;
    /** Number of 100ths of a second to wait before drawing the next graphic. */
    uint16_t delay_time;
    /** Index into color table for a transparent color. */
    uint8_t transparent_color_idx;
};

/** Plain Text extension. */
struct GIF_PlainTextExt
{
    /** Text grid description. */
    uint16_t tg_left, tg_top, tg_width, tg_height;
    /** Character cell size. */
    uint8_t cell_width, cell_height;
    /** Text foreground/background colors. */
    uint8_t fg_idx, bg_idx;
    /** No. of bytes in DATA. */
    size_t data_size;
    /** Comment text. */
    uint8_t *data;
};

/** Application extension. */
struct GIF_ApplicationExt
{
    /** Application identifier. */
    char appid[8];
    /** Application authentication code. */
    char auth_code[3];
    /** No. of bytes in DATA. */
    size_t data_size;
    /** App extension data. */
    uint8_t *data;
};

/** Graphic Block. */
struct GIF_Graphic
{
    /** Attached Graphic Extension, or NULL if there isn't one. */
    struct GIF_GraphicExt *extension;
    /** If true, the graphic is an image, if false, it's a plaintext. */
    bool is_img;
    union
    {
        /** Valid only if IS_IMG is true. */
        struct GIF_Image img;
        /** Valid only if IS_IMG is false. */
        struct GIF_PlainTextExt plaintext;
    };
};

/** Container for GIF data. */
typedef struct GIF
{
    /** GIF version number. */
    enum GIF_Version version;

    /** Image dimensions, as specified in the Logical Screen Descriptor. */
    uint16_t width, height;
    /** If `global_color_table` is not NULL, index into GCT of background
     * color. */
    uint8_t bg_color_index;

    /** Bits per color minus 1 in the *original image*, NOT the GIF itself. */
    uint8_t color_resolution;
    /** Approximation of pixel aspect ratio of original image. */
    uint8_t pixel_aspect_ratio;

    /** Pointer to global color table, or NULL if there isn't one. */
    struct GIF_ColorTable *global_color_table;

    /** A list of GIF_Graphics in the GIF. */
    LinkedList *graphics;
    /** A list of comments (char *) in the GIF. */
    LinkedList *comments;
    /** A list of GIF_ApplicationExts in the GIF. */
    LinkedList *app_extensions;
} GIF;


/* Load a GIF from a file. */
GIF gif_from_file(char const *filename);

/* Deallocate GIF data. */
void gif_free(GIF gif);


#endif /* GIFVIEW_GIF_H */
