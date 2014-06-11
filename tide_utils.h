#ifndef TIDE_UTILS
#define TIDE_UTILS

#include <stddef.h>

void *safe_malloc(size_t size);
char *parse_get(char *buf);
void write_client(int sock, const char *path);

#endif
