#ifndef STACK_INCLUDED
#define STACK_INCLUDED
#include <stdbool.h>


struct vector {
    int top;
    unsigned size;
    void **data;
};

struct vector* vector_create(unsigned initial_size);
void vector_delete(struct vector*);

void vector_push(struct vector* v, void* item);
void *vector_pop(struct vector*);
void *vector_peak(struct vector*);

bool vector_empty(struct vector* v);

#endif