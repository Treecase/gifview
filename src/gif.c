/*
 * gif.c -- GIF reading definitions.
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
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Generic GIF Extension data. */
struct GenericExtension
{
    uint8_t label;
    size_t data_size;
    uint8_t *data;
};

/* GIF Block Identifiers. */
enum GIF_BlockIdentifiers
{
    GIF_Block_Extension = 0x21,
    GIF_Block_Image = 0x2C,
    GIF_Block_Trailer = 0x3B,
};

/* GIF Extension Block labels. */
enum GIF_ExtensionLabels
{
    GIF_Ext_PlainText = 0x01,
    GIF_Ext_GraphicControl = 0xF9,
    GIF_Ext_Comment = 0xFE,
    GIF_Ext_ApplicationExtension = 0xFF
};


/* Read data sub-blocks from FILE, store these in DATA, and stop when a block
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

        /* get the number of bytes in the next block */
        if (safe_fread(&block_size, 1, 1, file))
        {
            exit(EXIT_FAILURE);
        }

        /* a block_size of 0 means we're done */
        if (block_size == 0)
        {
            return;
        }

        /* resize the data buffer to fit the block */
        errno = 0;
        *data = realloc(*data, (*data_size) + block_size);
        if (*data == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }

        /* read the data block */
        if (safe_fread(*data + *data_size, 1, block_size, file))
        {
            exit(EXIT_FAILURE);
        }
        *data_size += block_size;
    }
}

/* Read a Header block from FILE, and placing the parsed version number into
 * VERSION. */
void read_header(FILE *file, enum Version *version)
{
    char header[6];

    /* read header */
    if (safe_fread(header, 1, 6, file)) exit(EXIT_FAILURE);

    /* check signature */
    if (strncmp(header, "GIF", 3) != 0)
    {
        fprintf(stderr, "read_header: bad signature '%.3s'\n", header);
        exit(EXIT_FAILURE);
    }

    /* check version */
    if (strncmp(header + 3, "87a", 3) == 0)
    {
        *version = GIF_Version_87a;
    }
    else if (strncmp(header + 3, "89a", 3) == 0)
    {
        *version = GIF_Version_89a;
    }
    else
    {
        *version = GIF_Version_Unknown;
        printf("WARNING: read_header -- unknown version '%.3s'\n", header + 3);
    }
}

/* Read SIZE*3 bytes of Color Table data from FILE, storing it in TABLE.
 * Memory pointed to by TABLE must be freed.
 */
void read_color_table(FILE *file, size_t size, uint8_t **table)
{
    /* allocate the table buffer */
    errno = 0;
    *table = malloc(3 * size);
    if (*table == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    /* read the table data */
    if (safe_fread(*table, 3, size, file))   exit(EXIT_FAILURE);
}

/* Read a Logical Screen Descriptor from FILE, storing the data in LSD. */
void read_logical_screen_descriptor(FILE *file, struct GIF_LSD *lsd)
{
    uint8_t fields;

    /* read all the LSD data */
    if (safe_fread(&lsd->width, 2, 1, file))                exit(EXIT_FAILURE);
    if (safe_fread(&lsd->height, 2, 1, file))               exit(EXIT_FAILURE);
    if (safe_fread(&fields, 1, 1, file))                    exit(EXIT_FAILURE);
    if (safe_fread(&lsd->bg_color_index, 1, 1, file))       exit(EXIT_FAILURE);
    if (safe_fread(&lsd->pixel_aspect_ratio, 1, 1, file))   exit(EXIT_FAILURE);

    /* grab all the packed fields */
    uint8_t gct_exponent = fields & 7;
    lsd->sort_flag = (fields >> 3) & 1;
    lsd->color_resolution = (fields >> 4) & 7;
    lsd->gct_flag = (fields >> 7) & 1;
    lsd->gct_size = 1 << (gct_exponent + 1);

    /* get the Global Color Table, if it exists */
    if (lsd->gct_flag)
    {
        read_color_table(file, lsd->gct_size, &lsd->color_table);
    }
    else
    {
        lsd->color_table = NULL;
    }
}

/* Fill IMAGE with Table-Based Image Data data read from FILE. */
void read_image_data(FILE *file, struct GIF_ImageData *image)
{
    if (safe_fread(&image->min_code_size, 1, 1, file)) exit(EXIT_FAILURE);
    uint8_t *compressed = NULL;
    size_t compressed_size = 0;
    read_data_sub_blocks(file, &compressed_size, &compressed);
    image->image_size = unlzw(image->min_code_size, compressed, &image->image);
}

/* Fill IMAGE with Image Descriptor data read from FILE. */
void read_image_descriptor(FILE *file, struct GIF_Image *image)
{
    uint8_t fields;

    safe_fread(&image->left, 2, 1, file);
    safe_fread(&image->right, 2, 1, file);
    safe_fread(&image->width, 2, 1, file);
    safe_fread(&image->height, 2, 1, file);
    safe_fread(&fields, 1, 1, file);

    uint8_t lct_exponent = fields & 7;
    image->sort_flag = (fields >> 5) & 1;
    image->interlace_flag = (fields >> 6) & 1;
    image->lct_flag = (fields >> 7) & 1;
    image->lct_size = 1 << (lct_exponent + 1);

    /* get the Local Color Table */
    if (image->lct_flag)
    {
        read_color_table(file, image->lct_size, &image->color_table);
        free(image->color_table);
    }
    else
    {
        image->color_table = NULL;
    }

    image->data.image = NULL;
    read_image_data(file, &image->data);
}

/* Fill EXTENSION with generic GIF extension data read from FILE. */
void read_extension(FILE *file, struct GenericExtension *extension)
{
    if (safe_fread(&extension->label, 1, 1, file))  exit(EXIT_FAILURE);
    read_data_sub_blocks(file, &extension->data_size, &extension->data);
}


/* Convert generic extension data from EXT into a GraphicControl extension in
 * OUT. */
void parse_graphic_ext(struct GenericExtension ext, struct GIF_GraphicExt *out)
{
    uint8_t fields = 0;
    memcpy(&fields                    , ext.data+0, 1);
    memcpy(&out->delay_time           , ext.data+1, 2);
    memcpy(&out->transparent_color_idx, ext.data+3, 1);

    out->transparent_color_flag = fields & 1;
    out->user_input_flag = (fields >> 1) & 1;
    uint8_t dm = (fields >> 2) & 7;
    switch (dm)
    {
    case 0:
        out->disposal_method = GIF_DisposalMethod_None;
        break;
    case 1:
        out->disposal_method = GIF_DisposalMethod_DoNotDispose;
        break;
    case 2:
        out->disposal_method = GIF_DisposalMethod_RestoreBackground;
        break;
    case 3:
        out->disposal_method = GIF_DisposalMethod_RestorePrevious;
        break;
    default:
        out->disposal_method = GIF_DisposalMethod_Undefined;
        break;
    }
}

/* Convert generic extension data from EXT into a PlainText extension in OUT. */
void parse_plaintext_ext(struct GenericExtension ext, struct GIF_PlainTextExt *out)
{
    memcpy(&out->tg_left    , ext.data+ 0, 2);
    memcpy(&out->tg_top     , ext.data+ 2, 2);
    memcpy(&out->tg_width   , ext.data+ 4, 2);
    memcpy(&out->tg_height  , ext.data+ 6, 2);
    memcpy(&out->cell_width , ext.data+ 8, 1);
    memcpy(&out->cell_height, ext.data+ 9, 1);
    memcpy(&out->fg_idx     , ext.data+10, 1);
    memcpy(&out->bg_idx     , ext.data+11, 1);
    out->data_size = ext.data_size - 12;
    out->data = malloc(out->data_size);
    memcpy(out->data, ext.data + 12, out->data_size);
}


/* Allocate a new GIF_Graphic object containing a Graphic extension and
 * PlainText extension */
struct GIF_Graphic *mk_ext_plaintext(
    struct GenericExtension gce, struct GenericExtension pte)
{
    struct GIF_Graphic *graphic = calloc(1, sizeof(struct GIF_Graphic));
    graphic->has_extension = true;
    parse_graphic_ext(gce, &graphic->extension);
    graphic->is_img = false;
    parse_plaintext_ext(pte, &graphic->plaintext);
    return graphic;
}

/* Allocate a new GIF_Graphic object containing a PlainText extension */
struct GIF_Graphic *mk_noext_plaintext(struct GenericExtension ext)
{
    struct GIF_Graphic *graphic = calloc(1, sizeof(struct GIF_Graphic));
    graphic->has_extension = false;
    graphic->is_img = false;
    parse_plaintext_ext(ext, &graphic->plaintext);
    return graphic;
}

/* Allocate a new GIF_Graphic object containing a Graphic extension and Image */
struct GIF_Graphic *mk_ext_image(struct GenericExtension gce)
{
    struct GIF_Graphic *graphic = calloc(1, sizeof(struct GIF_Graphic));
    graphic->has_extension = true;
    parse_graphic_ext(gce, &graphic->extension);
    graphic->is_img = true;
    return graphic;
}

/* Allocate a new GIF_Graphic object containing an Image */
struct GIF_Graphic *mk_noext_image(void)
{
    struct GIF_Graphic *graphic = calloc(1, sizeof(struct GIF_Graphic));
    graphic->has_extension = false;
    graphic->is_img = true;
    return graphic;
}


/* Append NEW to the linked list of GIF_Graphics pointed to by HEAD. */
void append_graphic(struct GIF_Graphic **head,
                    struct GIF_Graphic *new)
{
    if (*head == NULL)
    {
        *head = new;
        return;
    }
    struct GIF_Graphic *curr = *head;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new;
}


/* Parse a GIF datastream from FILE, storing the data in GIF. */
void read_GIF(FILE *file, GIF *gif)
{
    read_header(file, &gif->version);
    read_logical_screen_descriptor(file, &gif->lsd);

    /* read data blocks until we hit a GIF Trailer */
    for (bool done = false; !done;)
    {
        /* get the block identifier for the next block */
        uint8_t code = 0;
        safe_fread(&code, 1, 1, file);
        switch (code)
        {
        case GIF_Block_Extension:
            /* get the extension label for the block */
            struct GenericExtension extension;
            read_extension(file, &extension);
            switch (extension.label)
            {
            /* extensionless PlainText */
            case GIF_Ext_PlainText:
                struct GIF_Graphic *graphic = mk_noext_plaintext(extension);
                append_graphic(&gif->graphics, graphic);
                break;

            /* Graphic Block with extension */
            case GIF_Ext_GraphicControl:
                uint8_t code = 0;
                safe_fread(&code, 1, 1, file);
                switch (code)
                {
                /* expecting a PlainText... */
                case GIF_Block_Extension:
                    struct GenericExtension extension2;
                    read_extension(file, &extension2);
                    /* PlainText with extension */
                    if (extension2.label == GIF_Ext_PlainText)
                    {
                        struct GIF_Graphic *graphic =\
                            mk_ext_plaintext(extension, extension2);
                        append_graphic(&gif->graphics, graphic);
                    }
                    /* expected a PlainText, got something else! */
                    else
                    {
                        fprintf(
                            stderr,
                            (   "ERROR: (%ld) read_GIF -- expected extension"
                                "block 0x01, got %#.2hhx\n"),
                            ftell(file),
                            extension2.label);
                        exit(EXIT_FAILURE);
                    }
                    break;

                /* Image with extension */
                case GIF_Block_Image:
                    struct GIF_Graphic *graphic = mk_ext_image(extension);
                    read_image_descriptor(file, &graphic->img);
                    append_graphic(&gif->graphics, graphic);
                    break;
                }
                break;

            case GIF_Ext_Comment:
                // data < special purpose block
                printf("WARNING: (%ld) comment extension not implemented!\n",
                    ftell(file));
                break;

            case GIF_Ext_ApplicationExtension:
                // data < special purpose block
                printf(
                    "WARNING: (%ld) application extension not implemented!\n",
                    ftell(file));
                break;

            default:
                fprintf(
                    stderr,
                    "WARNING: (%ld) read_GIF -- unknown label '%#.2hhx'\n",
                    ftell(file),
                    extension.label);
                break;
            }
            free(extension.data);
            break;

        /* extensionless Image */
        case GIF_Block_Image:
            struct GIF_Graphic *graphic = mk_noext_image();
            read_image_descriptor(file, &graphic->img);
            append_graphic(&gif->graphics, graphic);
            break;

        case GIF_Block_Trailer:
            done = true;
            break;

        default:
            fprintf(
                stderr,
                "ERROR: (%ld) read_GIF -- unknown introducer '%#.2hhx'\n",
                ftell(file),
                code);
            exit(EXIT_FAILURE);
            break;
        }
    }
}


/* Load a GIF from a file. */
GIF load_gif_from_file(char const *filename)
{
    errno = 0;
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR: fopen -- %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    GIF gif = {};
    read_GIF(file, &gif);

    if (fclose(file))
    {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
    file = NULL;

    return gif;
}
