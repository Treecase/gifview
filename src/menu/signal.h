/*
 * signal.h -- Signals.
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

#ifndef GIFVIEW_SIGNAL_H
#define GIFVIEW_SIGNAL_H

#include "boundfunction.h"
#include "linkedlist/linkedlist.h"


typedef struct Signal
{
    /** LinkedList of BoundFunctions. */
    LinkedList *connections;
} Signal;


Signal *signal_new(void);
void signal_free(Signal *sig);

void signal_connect(Signal *sig, BoundFunction fn);
void signal_emit(Signal *sig);


#endif /* GIFVIEW_BOUNDFUNCTION_H */

