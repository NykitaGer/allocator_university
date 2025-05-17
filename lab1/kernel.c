#include "kernel.h"
#include <stdlib.h>

void* kernel_alloc(size_t size) {
	return malloc(size);
}