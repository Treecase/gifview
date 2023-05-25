/*
 * gif.c -- GIF functions.
 *
 * Copyright (C) 2022-2023 Trevor Last
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

#include <stdlib.h>


void gif_free_colortable(struct GIF_ColorTable *colortable)
{
    free(colortable->colors);
}

void gif_free_image(struct GIF_Image *image, struct GIF_ColorTable *gct)
{
    /* color_table can point to the GCT, so we need to check for that to avoid
     * a double-free. */
    if (image->color_table != NULL && image->color_table != gct)
    {
        gif_free_colortable(image->color_table);
        free(image->color_table);
    }
    free(image->pixels);
}

void gif_free_plaintextext(struct GIF_PlainTextExt *pte)
{
    free(pte->data);
}

void gif_free_applicationext(struct GIF_ApplicationExt *appext)
{
    free(appext->data);
}

void gif_free_graphic(struct GIF_Graphic *g, struct GIF_ColorTable *gct)
{
    if (g->is_img)
    {
        if (g->extension)
            free(g->extension);
        gif_free_image(&g->img, gct);
    }
    else
        gif_free_plaintextext(&g->plaintext);
}


void gif_free(GIF gif)
{
    if (gif.global_color_table != NULL)
    {
        gif_free_colortable(gif.global_color_table);
        free(gif.global_color_table);
    }

    for (LinkedList *node = gif.graphics; node != NULL;)
    {
        gif_free_graphic(node->data, gif.global_color_table);
        LinkedList *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }

    for (LinkedList *node = gif.comments; node != NULL;)
    {
        LinkedList *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }

    for (LinkedList *node = gif.app_extensions; node != NULL;)
    {
        gif_free_applicationext(node->data);
        LinkedList *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }
}
