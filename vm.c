#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "stack.h"
#include "vm.h"

VM vm;

void initVM() {
	initStack(&vm.stack);
}

void freeVM() {
	freeStack(&vm.stack);
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() (vm.chunk->constants.values[ \
		READ_BYTE() + (READ_BYTE()<<8) + (READ_BYTE()<<16)])
#define BINARY_OP(op) \
	do { \
		double b = popStack(&vm.stack); \
		double a = popStack(&vm.stack); \
		pushStack(&vm.stack, a op b); \
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
			case OP_ADD:
				{
					BINARY_OP(+);
					break;
				}
			case OP_SUBTRACT:
				{
					BINARY_OP(-);
					break;
				}
			case OP_MULTIPLY:
				{
					BINARY_OP(*);
					break;
				}
			case OP_DIVIDE:
				{
					BINARY_OP(/);
					break;
				}
			case OP_NEGATE:
				{
					pushStack(&vm.stack, -popStack(&vm.stack));
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
