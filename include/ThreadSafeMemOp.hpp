#pragma once

#include <stddef.h>

namespace tsmo {

void* malloc(size_t size);
void free(void* ptr);

} // namespace tsmo