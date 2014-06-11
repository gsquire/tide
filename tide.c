#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tide_server.h"

/* our global server socket
*/
int sock;

/* interrupt signal handler
*/
void sig_handler(int sig) {
    printf("server exiting cleanly...\n");
    sleep(1);
    close(sock);
    exit(EXIT_SUCCESS);
}

/* print usage and exit if bad options are passed
*/
void usage_exit() {
    fprintf(stderr, "Usage: tide [OPTION(S)]...\n");
    fprintf(stderr, "\t-h, print usage and exit\n");
    fprintf(stderr, "\t-p [PORT], which port to listen on\n");
    fprintf(stderr, "\t-6, specifies to create an IPV6 socket\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int opt, ind;

    const char *opts = "6hp:";

    // port option, 0 for default lettins OS choose
    int port = 0;

    // ipv6 option
    int ipv6 = 0;

    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case '6':
                ipv6 = 1;
                break;
            case 'h':
                usage_exit();
            default:
                usage_exit();
        }
    }

    // register our signal handler
    struct sigaction sa;
    sa.sa_handler = sig_handler;

    if (sigaction(SIGINT, &sa, NULL) == -1)
        error_exit("creating signal");
    
    sock = serv_sock(ipv6, port);
    serve_forever(sock);

    return 0;
}
