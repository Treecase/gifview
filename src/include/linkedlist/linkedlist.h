/*
 * linkedlist.h -- Linked list.
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

#ifndef GIFVIEW_LINKEDLIST_H
#define GIFVIEW_LINKEDLIST_H


/** Linked List node. */
typedef struct LinkedList
{
    void *data;
    struct LinkedList *next;
} LinkedList;


/** Allocate a new LinkedList node containing DATA. */
LinkedList *linkedlist_new(void *data);

/** Append END to the linked list pointed to by HEAD. */
void linkedlist_append(LinkedList **head, LinkedList *end);


#endif /* GIFVIEW_LINKEDLIST_H */
