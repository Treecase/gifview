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

/* Add the __FILE__, __LINE__ to the _giflog call. */
#define _loglower(lvl, func, fmt, ...)  (\
    _giflog(lvl, __FILE__, __LINE__, func, fmt, ##__VA_ARGS__))

/* Warning printing. */
#define warn(fmt, ...)  _loglower(WARN, __func__, (fmt), ##__VA_ARGS__)

/* Error printing. */
#define error(fmt, ...) _loglower(ERROR, __func__, (fmt), ##__VA_ARGS__)

/* Fatal error printing. */
#define fatal(fmt, ...) _loglower(FATAL, __func__, (fmt), ##__VA_ARGS__)

/* Macro for _efread, adds parameters needed for the error printed. */
#define efread(ptr, size, n, stream)    (\
    _efread(__FILE__, __LINE__, __func__, (ptr), (size), (n), (stream)))

/* _giflog levels. */
typedef enum
{
    LOG,
    WARN,
    ERROR,
    FATAL,
} GIF_LoggingLevels;

/* Linked List node. */
typedef struct LinkedList
{
    void *data;
    struct LinkedList *next;
} LinkedList;

/* Allocate a new LinkedList node containing DATA. */
LinkedList *linkedlist_new(void *data);

/* Append END to the linked list pointed to by HEAD. */
void linkedlist_append(LinkedList **head, LinkedList *end);

/* Error-checked fread.  If an error occurs, prints the error message and dies.
 * If EOF is hit, prints a warning. */
size_t _efread(
    char const *file, int line, char const *func, void *restrict ptr,
    size_t size, size_t n, FILE *restrict stream);

/* Concatenate two strings, returning the result in a newly-allocated string. */
char *estrcat(char const *prefix, char const *suffix);

/* Low-level logging function. If LEVEL is `FATAL`, exits the program. */
void _giflog(
    GIF_LoggingLevels level, char const *file, int line, char const *func,
    char const *format, ...);

#endif  // GIFVIEW_UTIL_H
