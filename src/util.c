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
#include <string.h>


/* Error checked wrapper around `fread`.  Returns 0 on success, or the value of
 * errno on failure. */
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
