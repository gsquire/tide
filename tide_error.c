#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tide_error.h"

void error_exit(const char *message) {
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

/* add support for logging here...
*/
void server_error(int sock, const char *message) {
    close(sock);
    printf("%s\n", message);
}
