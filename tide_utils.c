#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "tide_utils.h"

#define BUF 256
#define GET "GET "
/* just a check to see if we could really get memory
*/
void *safe_malloc(size_t size) {
    if (size == 0)
        error_exit("malloc size of 0");
    
    void *mem = malloc(size);
    if (mem == NULL)
        error_exit("malloc call failed");

    return mem;
}

/* http get request to return the file requested
 * return will need to be freed upon successful request
*/
char *parse_get(char *buf) {
    char *req_t = safe_malloc(BUF);
    char *file = safe_malloc(BUF);

    sscanf(buf, "%s %s", req_t, file);

    if (!strcmp(req_t, GET)) {
        free(req_t);
        free(file);
        return NULL;
    }

    free(req_t);
    return file;
}

/* file reading function for writing back to client
*/
void write_client(int sock, const char *path) {
    struct stat buf;

    ssize_t bytes;

    char *rel_path = safe_malloc(BUF);

    strcpy(rel_path, ".");
    strcat(rel_path, path);

    int fd = open(rel_path, O_RDONLY);
    if (fd < 0) {
        free(rel_path);
        server_error(sock, "opening requested file");
    }

    if ((stat(rel_path, &buf)) < 0) {
        free(rel_path);
        close(fd);
        server_error(sock, "stat of requested file");
        return;
    }

    char *read_buf = safe_malloc(BUF);
    while ((bytes = read(fd, read_buf, BUF)) > 0) {
        if (write(sock, read_buf, bytes) < 0) {
            free(read_buf);
            free(rel_path);
            close(fd);
            server_error(sock, "writing to client");
        }
    }

    free(read_buf);
    free(rel_path);
    close(fd);
}
