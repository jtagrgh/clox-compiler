#ifndef clox_stack_h
#define clox_stack_h

#include "value.h"

typedef struct {
    int capacity;
    int count;
    Value *values;
} Stack;

void initStack(Stack* stack);
void pushStack(Stack* stack, Value value);
Value popStack(Stack* stack);
void freeStack(Stack* stack);
Value* stackTop(Stack* stack);

#endif
