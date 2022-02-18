/*
 * util.h -- Utility function declarations.
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

#ifndef GIFVIEW_UTIL_H
#define GIFVIEW_UTIL_H

#include <stdio.h>

/* Error checked wrapper around `fread` */
int safe_fread(void *restrict ptr, size_t size, size_t n,
               FILE *restrict stream);


#endif  // GIFVIEW_UTIL_H
