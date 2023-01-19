/*
 * gif.c -- GIF functions.
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

#include <stdlib.h>


void gif_free(GIF gif)
{
    if (gif.global_color_table != NULL)
    {
        free(gif.global_color_table->colors);
        free(gif.global_color_table);
    }

    for (LinkedList *node = gif.graphics; node != NULL;)
    {
        struct GIF_Graphic *g = node->data;
        if (g->is_img)
        {
            if (g->extension)
                free(g->extension);
            free(g->img.pixels);
            if (g->img.color_table != NULL
                && g->img.color_table != gif.global_color_table)
            {
                free(g->img.color_table->colors);
                free(g->img.color_table);
            }
        }
        else
        {
            free(g->plaintext.data);
        }
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
        struct GIF_ApplicationExt *ext = node->data;
        free(ext->data);
        LinkedList *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }
}
