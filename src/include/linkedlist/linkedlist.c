/*
 * linkedlist.c -- Linked list.
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

#include "linkedlist.h"

#include <stdlib.h>


LinkedList *linkedlist_new(void *data)
{
    LinkedList *new = malloc(sizeof(LinkedList));
    new->data = data;
    new->next = NULL;
    return new;
}

void linkedlist_append(LinkedList **head, LinkedList *end)
{
    if (*head == NULL)
    {
        *head = end;
        return;
    }
    LinkedList *curr = *head;
    while (curr->next != NULL)
        curr = curr->next;
    curr->next = end;
}
