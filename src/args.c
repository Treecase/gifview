/*
 * args.c -- Argument handling definitions.
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

#include "args.h"
#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>


void usage(char const *name, bool print_long)
{
    printf("Usage: %s [OPTION]... FILE\n", name);
    if (print_long)
    {
        puts("\
Display GIF images.\n\
\n\
OPTIONS\n\
      --help     display this help and exit\n\
      --version  output version information and exit\n\
\n\
Report bugs to: <https://github.com/Treecase/gifview/issues>\n\
pkg home page: <https://github.com/Treecase/gifview>\
");
    }
    else
        printf("Try '%s --help' for more information.\n", name);
}

void version(void)
{
    puts(GIFVIEW_PROGRAM_NAME " " GIFVIEW_VERSION);
    puts("\
Copyright (C) 2022 Trevor Last\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\
");
}

char const *parse_args(int argc, char *argv[])
{
    static char const *const short_options = "";
    static struct option const long_options[] = {
        {"help",    no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {NULL, 0, NULL, 0}
    };

    bool bad_args = false;
    int c, long_opt_ptr;
    while (
        (c = getopt_long(
            argc, argv, short_options, long_options, &long_opt_ptr))
        != -1)
    {
        switch (c)
        {
        /* Long options. */
        case 0:
            switch (long_opt_ptr)
            {
            /* --help */
            case 0:
                usage(argv[0], true);
                exit(EXIT_SUCCESS);
                break;

            /* --version */
            case 1:
                version();
                exit(EXIT_SUCCESS);
                break;
            }
            break;

        /* Unrecognized option. */
        case '?':
            bad_args = true;
            break;

        default:
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (bad_args)
        exit(EXIT_FAILURE);

    if (optind >= argc)
    {
        usage(argv[0], false);
        exit(EXIT_FAILURE);
    }
    else if (optind + 1 != argc)
    {
        /* TODO: Handle this properly instead of just exiting. */
        usage(argv[0], false);
        exit(EXIT_FAILURE);
    }
    return argv[optind];
}
