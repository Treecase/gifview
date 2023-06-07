/*
 * boundfunction.h -- Function binding.
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

#ifndef GIFVIEW_BOUNDFUNCTION_H
#define GIFVIEW_BOUNDFUNCTION_H


typedef struct BoundFunction
{
    void (*fn)(void *);
    void *argument;
} BoundFunction;


BoundFunction bind(void (*fn)(), void *argument);
void boundfunction_invoke(BoundFunction fn);


#endif /* GIFVIEW_BOUNDFUNCTION_H */
