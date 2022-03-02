/*
 * lzw.h -- LZW decoding declarations.
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

#ifndef GIFVIEW_LZW_H
#define GIFVIEW_LZW_H

#include <stdint.h>
#include <stddef.h>

/* Decompress LZW-compressed data from IN into OUT.  Returns the number of
 * bytes stored in OUT. */
size_t unlzw(size_t min_code_size, uint8_t const *in, uint8_t **out);

#endif  // GIFVIEW_LZW_H
