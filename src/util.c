/*
 * util.c -- Utility function definitions.
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

#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


LinkedList *linkedlist_new(void *data)
{
    LinkedList *new = malloc(sizeof(LinkedList));
    new->data = data;
    new->next = NULL;
    return new;
}

void linkedlist_append(LinkedList **head, LinkedList *end)
{
    if (*head == NULL)
    {
        *head = end;
        return;
    }
    LinkedList *curr = *head;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = end;
}

int safe_fread(void *restrict ptr, size_t size, size_t n, FILE *restrict stream)
{
    int e = 0;
    errno = 0;
    size_t value = fread(ptr, size, n, stream);
    e = errno;
    if (value != n)
    {
        perror("fread");
        return e;
    }
    return 0;
}

char *estrcat(char const *prefix, char const *suffix)
{
    size_t prefix_len = strlen(prefix),
           suffix_len = strlen(suffix);
    char *out = malloc(prefix_len + suffix_len + 1);
    memcpy(out, prefix, prefix_len);
    memcpy(out + prefix_len, suffix, suffix_len);
    out[prefix_len + suffix_len + 1] = '\0';
    return out;
}
