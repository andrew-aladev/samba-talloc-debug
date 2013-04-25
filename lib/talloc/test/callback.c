#include "stdint.h"

#define TALLOC_DEBUG_CALLBACK
#include <talloc.h>
#include <list.h>

typedef struct callback {
    int8_t id;
    const TALLOC_CTX * parent;
    const TALLOC_CTX * child;
} callback;

static list * callbacks;

void add ( const TALLOC_CTX * parent, const TALLOC_CTX * child ) {
    callback * cb = malloc ( sizeof ( callback ) );
    cb->id     = 1;
    cb->parent = parent;
    cb->child  = child;
    list_append ( callbacks, cb );
}
void del ( const TALLOC_CTX * parent, const TALLOC_CTX * child ) {
    callback * cb = malloc ( sizeof ( callback ) );
    cb->id     = 2;
    cb->parent = parent;
    cb->child  = child;
    list_append ( callbacks, cb );
}
void free_callback ( void * data ) {
    callback * cb = data;
    free ( cb );
}

int main () {
    callbacks = list_create();

    talloc_set_callback_fn ( add, del );

    // 1. add child "root" to the parent "NULL"
    TALLOC_CTX * root = talloc_new ( NULL );
    
    if ( callbacks->length == 1 ) {
        callback * cb = callbacks->first->data;
        if ( cb->id != 1 || cb->parent != NULL || cb->child != root ) {
            talloc_free ( root );
            list_each ( callbacks, free_callback );
            list_free ( callbacks );
            return 1;
        }
    } else {
        talloc_free ( root );
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 2;
    }
    list_clear ( callbacks );

    // 1. add child "a" to the parent "root"
    char * a = talloc ( root, char );

    if ( callbacks->length == 1 ) {
        callback * cb = callbacks->first->data;
        if ( cb->id != 1 || cb->parent != root || cb->child != a ) {
            talloc_free ( root );
            list_each ( callbacks, free_callback );
            list_free ( callbacks );
            return 3;
        }
    } else {
        talloc_free ( root );
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 4;
    }
    list_clear ( callbacks );

    // 1. add child "b" to the parent "a"
    short * b = talloc ( a, short );
    // 2. add child "c" to the parent "a"
    int * c   = talloc ( a, int );
    // 3. add child "d" to the parent "c"
    long * d  = talloc ( c, long );

    if ( callbacks->length == 3 ) {
        callback * cb1 = callbacks->first->data;
        callback * cb2 = callbacks->first->next->data;
        callback * cb3 = callbacks->last->data;
        if (
            cb1->id != 1 || cb1->parent != a || cb1->child != b ||
            cb2->id != 1 || cb2->parent != a || cb2->child != c ||
            cb3->id != 1 || cb3->parent != c || cb3->child != d
        ) {
            talloc_free ( root );
            list_each ( callbacks, free_callback );
            list_free ( callbacks );
            return 5;
        }
    } else {
        talloc_free ( root );
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 6;
    }
    list_clear ( callbacks );

    // 1. del child "b" from the parent "a"
    // 2. add child "b" to   the parent "root"
    b = talloc_move ( root, &b );

    if ( callbacks->length == 2 ) {
        callback * cb1 = callbacks->first->data;
        callback * cb2 = callbacks->last->data;
        if (
            cb1->id != 2 || cb1->parent != a    || cb1->child != b ||
            cb2->id != 1 || cb2->parent != root || cb2->child != b
        ) {
            talloc_free ( root );
            list_each ( callbacks, free_callback );
            list_free ( callbacks );
            return 7;
        }
    } else {
        talloc_free ( root );
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 8;
    }
    list_clear ( callbacks );

    // 1. del child "a" from the parent "root"
    // 2. add child "a" to   the parent "b"
    a = talloc_move ( b, &a );
    // 3. del child "d" from the parent "c"
    // 4. add child "d" to   the parent "a"
    d = talloc_move ( a, &d );

    if ( callbacks->length == 4 ) {
        callback * cb1 = callbacks->first->data;
        callback * cb2 = callbacks->first->next->data;
        callback * cb3 = callbacks->first->next->next->data;
        callback * cb4 = callbacks->last->data;
        if (
            cb1->id != 2 || cb1->parent != root || cb1->child != a ||
            cb2->id != 1 || cb2->parent != b    || cb2->child != a ||
            cb3->id != 2 || cb3->parent != c    || cb3->child != d ||
            cb4->id != 1 || cb4->parent != a    || cb4->child != d
        ) {
            talloc_free ( root );
            list_each ( callbacks, free_callback );
            list_free ( callbacks );
            return 9;
        }
    } else {
        talloc_free ( root );
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 10;
    }
    list_clear ( callbacks );

    // del all 5 elements
    talloc_free ( root );
    
    if (callbacks->length != 5) {
        list_each ( callbacks, free_callback );
        list_free ( callbacks );
        return 11;
    }

    list_each ( callbacks, free_callback );
    list_free ( callbacks );

    return 0;
}
