#include <errno.h>
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

/* return a 404 response to the client
*/
void client_404(int sock) {
    char *buf = safe_malloc(BUF);

    strcpy(buf, "HTTP/1.1 404 Not Found\r\n");
    strcat(buf, "<HTML><HEAD><TITLE>HTTP ERROR 404</TITLE></HEAD><BODY>");
    strcat(buf,
        "404 Not Found.  Your request could not be completed due to");
    strcat(buf, " encountering HTTP error number 404.</BODY></HTML>");
    strcat(buf, "\r\n");

    if ((write(sock, buf, strlen(buf)) < 0)) {
        free(buf);
        server_error(sock, "writing 404 to client");
    }

    free(buf);
}

/* return a 403 response to the client
*/
void client_403(int sock)
{
    char *buf = safe_malloc(BUF);
    strcpy(buf, "HTTP/1.1 403 Forbidden\r\n");
    strcat(buf, "<HTML><HEAD><TITLE>HTTP ERROR 403</TITLE></HEAD><BODY>");
    strcat(buf,
    "403 Forbidden.  Your request could not be completed due to encountering");
    strcat(buf, " HTTP error number 403.</BODY></HTML>");
    strcat(buf, "\r\n");
    
    if ((write(sock, buf, strlen(buf)) < 0)) {
        free(buf);
        server_error(sock, "writing 403 to client");
    }

    free(buf);
}

/* a successfull HTTP 200 response
*/
int client_200(int sock, off_t file_len, const char *cont_type)
{
    // buffer used to write back to the socket
    char *buf = safe_malloc(BUF);
    char *flen = safe_malloc(BUF);
    sprintf(flen, "%ld", file_len);

    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    strcat(buf, "Content-Type: ");
    strcat(buf, cont_type);
    strcat(buf, "\r\nContent-Length: ");
    strcat(buf, flen);
    strcat(buf, "\r\nConnection: close");
    strcat(buf, "\r\n\r\n");

    if ((write(sock, buf, strlen(buf)) < 0)) {
        free(buf);
        free(flen);
        server_error(sock, "writing 200 to client");
        return 1;
    }

    free(buf);
    free(flen);
    
    return 0;
}

/* write the requested file back to the client
*/
void client_write_file(int sock, int fd) {
    char *read_buf = safe_malloc(BUF);

    ssize_t bytes;

    while ((bytes = read(fd, read_buf, BUF)) > 0) {
        if (write(sock, read_buf, bytes) < 0) {
            free(read_buf);
            close(fd);
            server_error(sock, "writing file to client");
        }
    }

    free(read_buf);
    close(fd);
}

/* get the file type for HTTP response based on the extension
 * returns text/plain if there was no extension
*/
char *get_file_type(const char *file) {
    char *type = safe_malloc(BUF);
    char *ext = strrchr(file, '.');

    if (!ext || !*(ext + 1))
        type = "text/plain";
    else if (!strcmp(ext + 1, "txt"))
        type = "text/plain";
    else if (!strcmp(ext + 1, "html"))
        type = "text/html";
    else if (!strcmp(ext + 1, "jpeg") || !strcmp(ext + 1, "jpg"))
        type = "image/jpeg";
    else if (!strcmp(ext + 1, "png"))
        type = "image/png";

    return type;
        
}

/* file reading function for writing back to client
 * TODO add logging support for server errors here
*/
void write_client(int sock, const char *path) {
    struct stat buf;

    char *rel_path = safe_malloc(BUF);

    strcpy(rel_path, ".");
    strcat(rel_path, path);

    int fd = open(rel_path, O_RDONLY);
    if (fd < 0) {
        free(rel_path);
        client_404(sock);
        return;
    }

    if ((stat(rel_path, &buf)) < 0) {
        free(rel_path);
        close(fd);
        if (errno == EACCES)
            client_403(sock);
        else
            client_404(sock);
        return;
    }

    // successful request, print 200 and file
    char *type = get_file_type(path);
    int fail = client_200(sock, buf.st_size, type);
    if (!fail)
        client_write_file(sock, fd);
    else {
        free(type);
        free(rel_path);
        server_error(sock, "writing 200 request to client");
        return;
    }
    
    free(type);
    free(rel_path);
}
