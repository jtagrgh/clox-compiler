#include "memory.h"
#include "line.h"

void initLineArray(LineArray* array) {
    array->capacity = 0;
    array->count = 0;
    array->lines = NULL;
}

void writeLineArray(LineArray* array, int number) {
    if (array->count > 0 &&
            array->lines[array->count - 1].number == number) {
        array->lines[array->count - 1].count++;
    } else {
        if (array->capacity < array->count + 1) {
            int oldCapacity = array->capacity;
            array->capacity = GROW_CAPACITY(oldCapacity);
            array->lines = GROW_ARRAY(Line, array->lines,
                    oldCapacity, array->capacity);
        }
        Line newLine = {number, 1};
        array->lines[array->count] = newLine;
        array->count++;
    }
}

void freeLineArray(LineArray* array) {
    FREE_ARRAY(Line, array->lines, array->capacity);
    initLineArray(array);
}

int getLineNumber(LineArray* array, int index) {
    int lb = 0;
    for (int i = 0; i < array->count; i++) {
        int ub = lb + array->lines[i].count;
        if (index >= lb && index < ub) {
            return array->lines[i].number;
        }
        lb = ub;
    }

    return -1;
}
