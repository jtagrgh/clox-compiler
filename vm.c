#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "stack.h"
#include "vm.h"
#include "line.h"

VM vm;

void initVM() {
	initStack(&vm.stack);
}

void freeVM() {
	freeStack(&vm.stack);
}

void push(Value value) {
	pushStack(&vm.stack, value);
}

Value pop() {
	return popStack(&vm.stack);
}

static Value peek(int distance) {
	return peekStack(&vm.stack, distance);
}

static bool isFalsey(Value value) {
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void runtimeError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	size_t instruction = vm.ip - vm.chunk->code - 1;
	int line = getLineNumber(&vm.chunk->lines, instruction);
	fprintf(stderr, "[line %d] in script\n", line);
	freeStack(&vm.stack);
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() (vm.chunk->constants.values[ \
		READ_BYTE() + (READ_BYTE()<<8) + (READ_BYTE()<<16)])
#define BINARY_OP(valueType, op) \
	do { \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
			runtimeError("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(popStack(&vm.stack)); \
		double a = AS_NUMBER(popStack(&vm.stack)); \
		pushStack(&vm.stack, valueType(a op b)); \
	} while (false)

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: 
				{ 
					Value constant = READ_CONSTANT();
					pushStack(&vm.stack, constant);
					break;
				}
			case OP_CONSTANT_LONG:
				{
					Value constant = READ_CONSTANT_LONG();
					pushStack(&vm.stack, constant);
					break;
				}
			case OP_NIL:
				{
					push(NIL_VAL);
					break;
				}
			case OP_TRUE:
				{
					push(BOOL_VAL(true));
					break;
				}
			case OP_FALSE:
				{
					push(BOOL_VAL(false));
					break;
				}
			case OP_EQUAL:
				{
					Value a = pop();
					Value b = pop();
					push(BOOL_VAL(valuesEqual(a,b)));
					break;
				}
			case OP_GREATER:
				{
					BINARY_OP(BOOL_VAL, >);
					break;
				}
			case OP_LESS:
				{
					BINARY_OP(BOOL_VAL, <);
					break;
				}
			case OP_ADD:
				{
					BINARY_OP(NUMBER_VAL, +);
					break;
				}
			case OP_SUBTRACT:
				{
					BINARY_OP(NUMBER_VAL, -);
					break;
				}
			case OP_MULTIPLY:
				{
					BINARY_OP(NUMBER_VAL, *);
					break;
				}
			case OP_DIVIDE:
				{
					BINARY_OP(NUMBER_VAL, /);
					break;
				}
			case OP_NOT:
				{
					push(BOOL_VAL(isFalsey(pop())));
					break;
				}
			case OP_NEGATE:
				{
					if (!IS_NUMBER(peek(0))) {
						runtimeError("Operand must be a number.");
						return INTERPRET_RUNTIME_ERROR;
					}
					push(NUMBER_VAL(-AS_NUMBER(pop())));
					break;
				}
			case OP_RETURN: 
				{ 
					printValue(popStack(&vm.stack));
					printf("\n");
					return INTERPRET_OK; 
				}
		}
#ifdef DEBUG_TRACE_EXECUTION
		printf("	");
		for (Value* slot = vm.stack.values; slot < stackTop(&vm.stack); slot++) {
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		}
		printf("\n");
#endif
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_CONSTANT_LONG
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);

	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();

	freeChunk(&chunk);
	return result;
}
