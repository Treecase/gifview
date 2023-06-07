/*
 * signal.c -- Signals.
 *
 * Copyright (C) 2023 Trevor Last
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

#include "signal.h"

#include <stdlib.h>


Signal *signal_new(void)
{
    Signal *sig = malloc(sizeof(Signal));
    sig->connections = NULL;
    return sig;
}


void signal_free(Signal *sig)
{
    for (LinkedList *node = sig->connections; node != NULL;)
    {
        LinkedList *next = node->next;
        free(node->data);
        free(node);
        node = next;
    }
    free(sig);
}


void signal_connect(Signal *sig, BoundFunction fn)
{
    BoundFunction *fnp = malloc(sizeof(fn));
    *fnp = fn;
    linkedlist_append(&sig->connections, linkedlist_new(fnp));
}


void signal_emit(Signal *sig)
{
    for (   LinkedList const *node = sig->connections;
            node != NULL;
            node = node->next)
        boundfunction_invoke(*(BoundFunction *)node->data);
}
