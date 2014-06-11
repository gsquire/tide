#ifndef TIDE_UTILS
#define TIDE_UTILS

#include <stddef.h>

void *safe_malloc(size_t size);
char *parse_get(char *buf);
void client_404(int sock);
void client_403(int sock);
int client_200(int sock, off_t file_len, const char *cont_type);
void client_write_file(int sock, int fd);
void get_file_type(const char *file, char *type);
void write_client(int sock, const char *path);

#endif
