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
#include <stdarg.h>
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

size_t _efread(
    char const *file, int line, char const *func, void *restrict ptr,
    size_t size, size_t n, FILE *restrict stream)
{
    errno = 0;
    size_t value = fread(ptr, size, n, stream);
    if (value != n)
    {
        if (value == 0 && feof(stream))
        {
            _giflog(WARN, file, line, func, "Unexpected EOF.\n");
        }
        else
        {
            _giflog(FATAL, file, line, func, "%s\n", strerror(errno));
        }
    }
    return value;
}

char *estrcat(char const *prefix, char const *suffix)
{
    size_t prefix_len = strlen(prefix),
           suffix_len = strlen(suffix);
    char *out = malloc(prefix_len + suffix_len + 1);
    memcpy(out, prefix, prefix_len);
    memcpy(out + prefix_len, suffix, suffix_len);
    out[prefix_len + suffix_len] = '\0';
    return out;
}

void _giflog(
    GIF_LoggingLevels level, char const *file, int line, char const *func,
    char const *format, ...)
{
    FILE *stream = stdout;
    char const *string = "LOG";
    switch (level)
    {
    case LOG:
        stream = stdout;
        string = "LOG";
        break;
    case WARN:
        stream = stdout;
        string = "WARNING";
        break;
    case ERROR:
        stream = stderr;
        string = "ERROR";
        break;
    case FATAL:
        stream = stderr;
        string = "FATAL ERROR";
        break;
    }
    va_list ap;
    va_start(ap, format);
    fprintf(stream, "%s: %s:%d:%s -- ", string, file, line, func);
    vfprintf(stream, format, ap);
    va_end(ap);
    if (level == FATAL)
    {
        exit(EXIT_FAILURE);
    }
}
