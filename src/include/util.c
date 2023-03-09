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
#include <stdlib.h>
#include <string.h>


size_t efread(void *restrict ptr, size_t size, size_t n, FILE *restrict stream)
{
    errno = 0;
    size_t value = fread(ptr, size, n, stream);
    if (value != n)
    {
        if (value == 0 && feof(stream))
            warn("Unexpected EOF.\n");
        else if (ferror(stream))
            fatal("%s\n", strerror(errno));
    }
    return value;
}

ssize_t egetline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
#define FAIL(err) { errno = (err); return -1; }

    if (lineptr == NULL || n == NULL || stream == NULL)
        FAIL(EINVAL);
    if (*lineptr == NULL)
    {
        *n = 32;
        *lineptr = malloc(*n);
        if (!*lineptr)
            FAIL(ENOMEM);
    }

    ssize_t chars_read = 0;

    int ch;
    while ((ch = fgetc(stream)) != EOF)
    {
        if (chars_read + 1 >= *n)
        {
            if (*n == 0)
                *n = 32;
            else
                *n *= 2;
            *lineptr = realloc(*lineptr, *n);
            if (!*lineptr)
                FAIL(ENOMEM);
        }
        (*lineptr)[chars_read++] = ch;
        if (ch == '\n')
        {
            (*lineptr)[chars_read] = '\0';
            return chars_read;
        }
    }

    // Only reached on EOF.
    return -1;
#undef FAIL
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

char *estrdup(char const *s)
{
    if (!s)
        return NULL;
    return estrndup(s, strlen(s));
}

char *estrndup(char const *s, size_t n)
{
    if (!s)
        return NULL;
    char *out = malloc(n + 1);
    if (!out)
    {
        errno = ENOMEM;
        return NULL;
    }
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

int sprintfa(char **restrict str, char const *restrict fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt);
    va_copy(ap2, ap);

    int count = vsnprintf(NULL, 0, fmt, ap);
    if (!str)
        return count;
    *str = calloc(count + 1, 1);
    count = vsnprintf(*str, count + 1, fmt, ap2);

    va_end(ap);
    va_end(ap2);
    return count;
}
