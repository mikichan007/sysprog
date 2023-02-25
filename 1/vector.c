#include <stdlib.h>
#include <stdbool.h>
#include "vector.h"


void vector_expand(struct vector* v) {
    if (v->size - 1 == v->top) {
        v->size *= 2;
        v->data = realloc(v->data, v->size * sizeof(void *));
    }
}

void vector_push(struct vector* v, void* item) {
    vector_expand(v);
    v->data[++v->top] = item;
}

void *vector_pop(struct vector* v) {
    return vector_empty(v) ? NULL : v->data[v->top--];
}

void *vector_peak(struct vector* v) {
    return vector_empty(v) ? NULL : v->data[v->top]; 
}

void vector_delete(struct vector *v) {
    for (int i = 0; i <= v->top; i++) {
        free(v->data[i]);
    }
    free(v->data);
    free(v);
}

bool vector_empty(struct vector* v) {
    return v->top == -1;
}


struct vector* vector_create(unsigned initial_size) {
    struct vector* v = malloc(sizeof(struct vector));
    v->top = -1;

    initial_size = initial_size > 1 ? initial_size : 1;
    v->size = initial_size;

    v->data = malloc(initial_size * sizeof(void *));
    
    return v;
}
