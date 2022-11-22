/*
 * args.h -- Argument handling declarations.
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

#ifndef _GIFVIEW_ARGS_H
#define _GIFVIEW_ARGS_H

#include <stdbool.h>


/** Print GIFView help information. */
void usage(char const *name, bool print_long);

/** Print GIFView version information. */
void version(void);

/** Parse command-line arguments. */
char const *parse_args(int argc, char *argv[]);


#endif /* _GIFVIEW_ARGS_H */
