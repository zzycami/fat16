#include "stack.h"

void* stack[512];
int top = -1;

void push(void *p){
	stack[++top] = p;
}

void *pop(){
	return stack[top--];
}

int isempty(){
	return top == -1;
}

void clear(){
	top = -1;
}
