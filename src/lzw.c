/*
 * lzw.c -- LZW decoding definitions.
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

#include "lzw.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct Bitstream
{
    uint8_t const *stream;
    size_t byte;
    size_t bit;
};

struct String
{
    size_t size;
    uint8_t *data;
};

struct Buffer
{
    size_t size;
    size_t allocated;
    uint8_t *data;
};


/** Read N bits from STREAM. */
unsigned int bitstream_read(size_t n, struct Bitstream *stream)
{
    unsigned int out = 0;
    size_t bit = 0;

    for (size_t i = 0; i < n; ++i)
    {
        unsigned int b = (stream->stream[stream->byte] >> (stream->bit++)) & 1;
        if (stream->bit >= 8)
        {
            stream->byte++;
            stream->bit = 0;
        }
        out |= b << (bit++);
    }
    return out;
}

/** Append SUFFIX to the end of PREFIX. */
struct String concat(struct String prefix, char suffix)
{
    struct String new = {
        .size = prefix.size + 1,
        .data = malloc(new.size)
    };
    memcpy(new.data, prefix.data, prefix.size);
    new.data[prefix.size] = suffix;
    return new;
}

/** Append DATA to BUFFER. */
void append(struct Buffer *buffer, struct String data)
{
    /* Number of extra bytes to allocate when the buffer is resized. */
    static size_t const growth_amount = 1024 * 8;

    size_t const newsize = buffer->size + data.size;
    if (newsize >= buffer->allocated)
    {
        buffer->allocated = newsize + growth_amount;
        buffer->data = realloc(buffer->data, buffer->allocated);
    }
    memcpy(buffer->data + buffer->size, data.data, data.size);
    buffer->size += data.size;
}


size_t unlzw(size_t min_code_size, uint8_t const *in, uint8_t **out)
{
    /* GIF has a maximum code size of 12 bits, the maximum code table size is
     * 2^12 = 4096 codes. */
    size_t const table_size = 4096;
    struct String table[table_size];

    /* Since GIF LZW has a clear code and end-of-input, the code size starts
     * off 1 larger than the minimum code size. */
    size_t code_size = min_code_size + 1;

    /* Clear code -- when encountered, the table is cleared. */
    uint16_t const cc = 1 << min_code_size;
    /* End of input marker. */
    uint16_t const eoi = cc + 1;
    /* Index of next available code in table. */
    uint16_t next = cc + 2;

    /* Initialize the code table with all values less than 2^min_code_size. */
    for (uint16_t i = 0; i < cc; ++i)
    {
        table[i].size = 1;
        table[i].data = malloc(1);
        table[i].data[0] = i;
    }

    struct Bitstream input = {.stream = in, .byte = 0, .bit = 0};
    struct Buffer output = {.size = 0, .data = NULL};

    uint16_t symbol = 0;
    /* Table is in default state already, so we can skip any leading clear
     * codes until we get a proper code. */
    do
    {
        symbol = bitstream_read(code_size, &input);
        if (symbol == eoi)
            goto LZW_done;
    } while (symbol == cc);
    struct String previous = table[symbol];
    append(&output, previous);

    for(;;)
    {
        symbol = bitstream_read(code_size, &input);
        if (symbol == cc)
        {
            for (uint16_t i = cc + 2; i < next; ++i)
                free(table[i].data);
            code_size = min_code_size + 1;
            next = cc + 2;
            do
            {
                symbol = bitstream_read(code_size, &input);
                if (symbol == eoi)
                    goto LZW_done;
            } while (symbol == cc);
            previous = table[symbol];
            append(&output, previous);
        }
        else if (symbol == eoi)
        {
            goto LZW_done;
        }
        else if (symbol < next)
        {
            struct String const W = table[symbol];
            append(&output, W);
            if (next < table_size)
                table[next++] = concat(previous, W.data[0]);
            previous = W;
        }
        else
        {
            struct String const V = concat(previous, previous.data[0]);
            append(&output, V);
            if (next < table_size)
                table[next++] = V;
            previous = V;
        }
        /* Once the code table contains 2^code_size values, the code size must
         * be increased. */
        if (next == (1 << code_size) && next < table_size)
            code_size++;
    }

LZW_done:
    for (size_t i = 0; i < next; ++i)
        if (i != cc && i != eoi)
            free(table[i].data);
    *out = realloc(output.data, output.size);
    return output.size;
}
