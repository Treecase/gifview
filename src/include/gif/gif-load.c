/*
 * gif-load.c -- Load a GIF data stream.
 *
 * Note that this parser implements a relaxed verison of the GIF 89a standard.
 * Firstly, the version number does not affect loading, so blocks added in 89a
 * can appear in an 87a versioned file.  Second, other blocks can appear between
 * a graphic control extension and the associated image, contrary to the BNF
 * grammar given in the spec.  This is intentional, done to handle real-world
 * data created with similarly lax encoders.
 *
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

#include "gif.h"
#include "lzw.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Parser;

/**
 * GIF Parser state.
 *
 * FN returns the next state to move to.  NAME identifies the state for error
 * messages.
 */
typedef struct ParseState
{
    struct ParseState (*fn)(struct Parser *);
    char *name;
} ParseState;

/**
 * GIF Parser state machine.
 *
 * Reads characters from STREAM according to STATE, building the RESULT as it
 * goes.  PUSHED_GEXT is used to store the last encountered Graphic Control
 * Extension, as blocks can appear between it and the Graphic it controls.  It
 * is an error to set this if it is not NULL.  When 'popping' the value, it
 * must be reset to NULL.
 *
 * TODO: Probably want to enforce the aforementioned PUSHED_GEXT contract
 * through methods!
 */
typedef struct Parser
{
    FILE *stream;
    ParseState state;
    struct GIF_GraphicExt *pushed_gext;
    GIF result;
} Parser;

struct GenericExtension
{
    uint8_t label;
    size_t data_size;
    uint8_t *data;
};

enum GIF_BlockIdentifiers
{
    GIF_ExtensionIntroducer = 0x21,
    GIF_ImageSeparator = 0x2C,
    GIF_Trailer = 0x3B,
};

enum GIF_ExtensionBlockLabels
{
    GIF_Ext_PlainText = 0x01,
    GIF_Ext_GraphicControl = 0xF9,
    GIF_Ext_Comment = 0xFE,
    GIF_Ext_ApplicationExtension = 0xFF
};


ParseState state_header(Parser *);
ParseState state_logical_screen_descriptor(Parser *p);
ParseState state_data(Parser *p);
ParseState state_extension(Parser *p);
ParseState state_image(Parser *p);
ParseState state_trailer(Parser *p);

static ParseState const
    STATE_HEADER = {state_header, "Header"},
    STATE_LSD = {state_logical_screen_descriptor, "Logical Screen Descriptor"},
    STATE_DATA = {state_data, "Data"},
    STATE_EXTENSION = {state_extension, "Extension"},
    STATE_IMAGE = {state_image, "Image"},
    STATE_TRAILER = {state_trailer, "Trailer"},
    STATE_FINISHED = {NULL, "Finished"};


/* ===[ Parser Methods ]=== */
/** Print parser error message and exit. */
void parser_error(
    Parser const *restrict p, char const *restrict fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *fmt2 = NULL;
    sprintfa(&fmt2, "GIF Parse Error: %s -- %s\n", p->state.name, fmt);
    vfprintf(stderr, fmt2, ap);
    free(fmt2);
    va_end(ap);
    exit(EXIT_FAILURE);
}

/** Read a byte from P's stream and return it. */
uint8_t parser_next(Parser *p)
{
    uint8_t byte;
    efread(&byte, 1, 1, p->stream);
    return byte;
}

/** Read a byte from P's stream, unget it, then return it. */
uint8_t parser_peek(Parser *p)
{
    uint8_t byte = parser_next(p);
    ungetc(byte, p->stream);
    return byte;
}

/** Read N bytes from P's stream into OUT. */
void parser_read(Parser *restrict p, void *restrict out, size_t n)
{
    efread(out, 1, n, p->stream);
}


/* ===[ Add extensions to the GIF ]=== */
void add_application_extension(Parser *p, struct GenericExtension ext)
{
    struct GIF_ApplicationExt *appext = malloc(sizeof(*appext));
    memcpy(appext->appid, ext.data, 8);
    memcpy(appext->auth_code, ext.data + 8, 3);
    appext->data_size = ext.data_size - 11;
    appext->data = malloc(appext->data_size);
    memcpy(appext->data, ext.data + 11, appext->data_size);
    linkedlist_append(&p->result.app_extensions, linkedlist_new(appext));
}

void add_comment_extension(Parser *p, struct GenericExtension ext)
{
    char *comment = calloc(ext.data_size + 1, 1);
    memcpy(comment, ext.data, ext.data_size);
    linkedlist_append(&p->result.comments, linkedlist_new(comment));
}

void add_graphic_control_extension(Parser *p, struct GenericExtension ext)
{
    struct GIF_GraphicExt *gext = malloc(sizeof(*gext));
    uint8_t fields;
    memcpy(&fields, ext.data+0, 1);
    memcpy(&gext->delay_time, ext.data+1, 2);
    memcpy(&gext->transparent_color_idx, ext.data+3, 1);
    gext->transparent_color_flag = fields & 1;
    gext->user_input_flag = (fields >> 1) & 1;
    gext->disposal_method = (fields >> 2) & 7;
    if (p->pushed_gext)
    {
        parser_error(
            p, "Graphic Control Extension appears without a matching graphic");
    }
    p->pushed_gext = gext;
}

void add_plain_text_extension(Parser *p, struct GenericExtension ext)
{
    struct GIF_PlainTextExt ptext;
    memcpy(&ptext.tg_left    , ext.data+ 0, 2);
    memcpy(&ptext.tg_top     , ext.data+ 2, 2);
    memcpy(&ptext.tg_width   , ext.data+ 4, 2);
    memcpy(&ptext.tg_height  , ext.data+ 6, 2);
    memcpy(&ptext.cell_width , ext.data+ 8, 1);
    memcpy(&ptext.cell_height, ext.data+ 9, 1);
    memcpy(&ptext.fg_idx     , ext.data+10, 1);
    memcpy(&ptext.bg_idx     , ext.data+11, 1);
    ptext.data_size = ext.data_size - 12;
    ptext.data = malloc(ptext.data_size);
    memcpy(ptext.data, ext.data + 12, ptext.data_size);
    struct GIF_Graphic *graphic = malloc(sizeof(*graphic));
    graphic->extension = p->pushed_gext;
    graphic->is_img = false;
    graphic->plaintext = ptext;
    p->pushed_gext = NULL;
    linkedlist_append(&p->result.graphics, linkedlist_new(graphic));
}

void add_extension(Parser *p, struct GenericExtension ext)
{
    switch (ext.label)
    {
    case GIF_Ext_ApplicationExtension:
        add_application_extension(p, ext);
        break;
    case GIF_Ext_Comment:
        add_comment_extension(p, ext);
        break;
    case GIF_Ext_GraphicControl:
        add_graphic_control_extension(p, ext);
        break;
    case GIF_Ext_PlainText:
        add_plain_text_extension(p, ext);
        break;
    default:
        parser_error(p, "Invalid extension label 0x%.2hhx", ext.label);
        break;
    }
}


/**
 * Read data sub-blocks from FILE, store these in DATA, and stop when a block
 * terminator is reached.  DATA_SIZE will be filled with the number of bytes
 * read.  Memory pointed to by data must be freed.
 */
void read_data_sub_blocks(FILE *file, size_t *data_size, uint8_t **data)
{
    *data_size = 0;
    *data = NULL;
    for(;;)
    {
        uint8_t block_size = 0;

        /* Get the number of bytes in the next block. */
        efread(&block_size, 1, 1, file);

        /* A block_size of 0 means we're done. */
        if (block_size == 0)
            return;

        /* Resize the data buffer to fit the block. */
        errno = 0;
        *data = realloc(*data, (*data_size) + block_size);
        if (*data == NULL)
            fatal("realloc: %s\n", strerror(errno));

        /* Read the data block. */
        efread(*data + *data_size, 1, block_size, file);
        *data_size += block_size;
    }
}

/**
 * Read SIZE*3 bytes of Color Table data from FILE, storing it in TABLE.
 * Memory pointed to by TABLE must be freed.
 */
struct GIF_ColorTable *read_color_table(FILE *file, bool sorted, size_t size)
{
    struct GIF_ColorTable *out = malloc(sizeof(*out));
    out->sorted = sorted;
    out->size = size;
    out->colors = malloc(3 * size);
    efread(out->colors, 3, size, file);
    return out;
}

/** Deinterlace interlaced GIF image data. */
void deinterlace(struct GIF_Image *image)
{
    uint8_t *interlaced = image->pixels;
    uint8_t *deinterlaced = malloc(image->size);

    /* Current row index of the interlaced data. */
    size_t n = 0;

    /* Group 1: Every 8th row, starting from row 0. */
    for (size_t y = 0; y < image->height; ++n, y += 8)
    {
        size_t il_row = n * image->width;
        size_t di_row = y * image->width;
        memcpy(deinterlaced + di_row, interlaced + il_row, image->width);
    }
    /* Group 2: Every 8th row, starting from row 4. */
    for (size_t y = 4; y < image->height; ++n, y += 8)
    {
        size_t il_row = n * image->width;
        size_t di_row = y * image->width;
        memcpy(deinterlaced + di_row, interlaced + il_row, image->width);
    }
    /* Group 3: Every 4th row, starting from row 2. */
    for (size_t y = 2; y < image->height; ++n, y += 4)
    {
        size_t il_row = n * image->width;
        size_t di_row = y * image->width;
        memcpy(deinterlaced + di_row, interlaced + il_row, image->width);
    }
    /* Group 4: Every 2nd row, starting from row 1. */
    for (size_t y = 1; y < image->height; ++n, y += 2)
    {
        size_t il_row = n * image->width;
        size_t di_row = y * image->width;
        memcpy(deinterlaced + di_row, interlaced + il_row, image->width);
    }
    free(interlaced);
    image->pixels = deinterlaced;
}


/* ===[ Parser State Functions ]=== */
ParseState state_extension(Parser *p)
{
    uint8_t first = parser_next(p);
    if (first != GIF_ExtensionIntroducer)
    {
        parser_error(
            p, "expected Extension Introducer (0x%.2hhx), got 0x%.2hhx",
            GIF_ExtensionIntroducer, first);
    }

    struct GenericExtension ext;
    ext.label = parser_next(p);
    read_data_sub_blocks(p->stream, &ext.data_size, &ext.data);

    add_extension(p, ext);

    return STATE_DATA;
}

/* TODO: Pseudo-state for now. */
ParseState _state_image_data(Parser *p, struct GIF_Image *image)
{
    uint8_t min_code_size;
    parser_read(p, &min_code_size, 1);

    uint8_t *compressed = NULL;
    size_t compressed_size = 0;
    read_data_sub_blocks(p->stream, &compressed_size, &compressed);

    image->size = unlzw(min_code_size, compressed, &image->pixels);
    if (image->interlace_flag)
        deinterlace(image);

    free(compressed);
    return STATE_DATA;
}

ParseState state_image(Parser *p)
{
    uint8_t separator = parser_next(p);
    if (separator != GIF_ImageSeparator)
    {
        parser_error(
            p, "expected image separator (0x%.2hhx), got 0x%.2hhx",
            GIF_ImageSeparator, separator);
    }

    struct GIF_Image image;
    uint8_t fields;
    parser_read(p, &image.left, 2);
    parser_read(p, &image.top, 2);
    parser_read(p, &image.width, 2);
    parser_read(p, &image.height, 2);
    parser_read(p, &fields, 1);

    uint8_t lct_exponent = fields & 7;
    bool sort_flag = (fields >> 5) & 1;
    image.interlace_flag = (fields >> 6) & 1;
    bool lct_flag = (fields >> 7) & 1;
    size_t lct_size = 1 << (lct_exponent + 1);

    image.color_table = NULL;
    if (lct_flag)
        image.color_table = read_color_table(p->stream, sort_flag, lct_size);
    else
        image.color_table = p->result.global_color_table;

    _state_image_data(p, &image);

    struct GIF_Graphic *graphic = malloc(sizeof(*graphic));
    graphic->extension = p->pushed_gext;
    graphic->is_img = true;
    graphic->img = image;
    p->pushed_gext = NULL;
    linkedlist_append(&p->result.graphics, linkedlist_new(graphic));

    return STATE_DATA;
}

ParseState state_trailer(Parser *p)
{
    uint8_t trailer = parser_next(p);
    if (trailer != GIF_Trailer)
    {
        parser_error(
            p, "expected trailer (0x%.2hhx), got 0x%.2hhx",
            GIF_Trailer, trailer);
    }
    return STATE_FINISHED;
}

ParseState state_data(Parser *p)
{
    uint8_t byte = parser_peek(p);
    switch (byte)
    {
    case GIF_ExtensionIntroducer:   return STATE_EXTENSION;
    case GIF_ImageSeparator:        return STATE_IMAGE;
    case GIF_Trailer:               return STATE_TRAILER;
    }
    parser_error(p, "unexpected byte 0x%.2hhx\n", byte);
}

ParseState state_logical_screen_descriptor(Parser *p)
{
    uint8_t fields;
    parser_read(p, &p->result.width, 2);
    parser_read(p, &p->result.height, 2);
    parser_read(p, &fields, 1);
    parser_read(p, &p->result.bg_color_index, 1);
    parser_read(p, &p->result.pixel_aspect_ratio, 1);

    uint8_t gct_exponent = fields & 7;
    bool sort_flag = (fields >> 3) & 1;
    p->result.color_resolution = (fields >> 4) & 7;
    bool gct_flag = (fields >> 7) & 1;
    size_t gct_size = 1 << (gct_exponent + 1);

    p->result.global_color_table = NULL;
    if (gct_flag)
    {
        p->result.global_color_table = read_color_table(
            p->stream, sort_flag, gct_size);
    }
    return STATE_DATA;
}

ParseState state_header(Parser *p)
{
    char header[6];
    parser_read(p, header, 6);

    if (strncmp(header, "GIF", 3) != 0)
        parser_error(p, "bad signature '%.3s'\n", header);

    p->result.version = GIF_Version_Unknown;
    if (strncmp(header + 3, "87a", 3) == 0)
        p->result.version = GIF_Version_87a;
    else if (strncmp(header + 3, "89a", 3) == 0)
        p->result.version = GIF_Version_89a;
    else
        warn("unknown version '%.3s'\n", header + 3);

    return STATE_LSD;
}


GIF gif_from_file(char const *filename)
{
    errno = 0;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        fatal("fopen: %s\n", strerror(errno));

    Parser p = {.stream = file, .state = STATE_HEADER, .pushed_gext=NULL};
    while (p.state.fn)
        p.state = p.state.fn(&p);

    errno = 0;
    if (fclose(file))
        fatal("fclose: %s\n", strerror(errno));

    return p.result;
}
