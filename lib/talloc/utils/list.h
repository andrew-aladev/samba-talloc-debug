// This file is part of btbot. Copyright (C) 2013 Andrew Aladjev aladjev.andrew@gmail.com
// btbot is free software: you can redistribute it and/or modify it under the terms of the GNU General Lesser Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// btbot is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Lesser Public License for more details.
// You should have received a copy of the GNU General Lesser Public License along with btbot. If not, see <http://www.gnu.org/licenses/>.

#ifndef UTILS_LIST_H
#define UTILS_LIST_H

#include <stdbool.h>
#include <talloc_debug.h>

typedef struct list_node {
    void * data;
    struct list_node * next;
} list_node;

typedef struct list {
    list_node * first;
    list_node * last;
    size_t length;
} list;

extern inline
list * list_create () {
    list * list = malloc ( sizeof ( list ) );
    if ( !list ) {
        return NULL;
    }

    list->first  = NULL;
    list->last   = NULL;
    list->length = 0;
    return list;
}

extern inline
bool list_append ( list * list, void * data ) {
    list_node * node = malloc ( sizeof ( list_node ) );
    if ( !node ) {
        return false;
    }

    node->data = data;
    node->next = NULL;
    if ( !list->last ) {
        list->length = 1;
        list->first  = node;
        list->last   = node;
    } else {
        list->last->next = node;
        list->last       = node;
        list->length     = list->length + 1;
    }

    return true;
}

extern inline
void list_each ( list * list, void ( * callback ) ( void * data ) ) {
    list_node * node = list->first;
    while ( node ) {
        callback ( node->data );
        node = node->next;
    }
}

extern inline
void list_clear ( list * list ) {
    list_node * next_node = list->first;
    list_node * node;
    while ( next_node ) {
        node = next_node;
        next_node = next_node->next;
        free ( node );
    }
    list->first  = NULL;
    list->last   = NULL;
    list->length = 0;
}

extern inline
void list_free ( list * list ) {
    list_node * next_node = list->first;
    list_node * node;
    while ( next_node ) {
        node = next_node;
        next_node = next_node->next;
        free ( node );
    }
    free ( list );
}

#endif
