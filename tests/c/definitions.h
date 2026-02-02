#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(expr) do {                                   \
if (expr) {                                             \
printf("[ASSERT PASS] %s:%d: %s\n",                \
__FILE__, __LINE__, #expr);                 \
} else {                                                \
fprintf(stderr, "[ASSERT FAIL] %s:%d: %s\n",       \
__FILE__, __LINE__, #expr);                \
abort();                                           \
}                                                       \
} while(0)

#endif // DEFINITIONS_H