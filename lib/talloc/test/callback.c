#include "../talloc.h"

void add(const TALLOC_CTX * context, const TALLOC_CTX * parent, const TALLOC_CTX * child) {
    printf("%s %p %p %p\n", "add", context, parent, child);
}
void del(const TALLOC_CTX * context, const TALLOC_CTX * parent, const TALLOC_CTX * child) {
    printf("%s %p %p %p\n", "del", context, parent, child);
}

int main () {
    TALLOC_CTX * root = talloc_new ( NULL );
    talloc_set_callback_fn(root, add, del);
    
    char * c  = talloc(root, char);
    short * s = talloc(c, short);
    int * i   = talloc(c, int);
    long * l  = talloc(i, long);
    
    s = talloc_move(root, &s);
    i = talloc_move(root, &i);
    l = talloc_move(root, &l);
    
    talloc_free(root);
    
    return 0;
}