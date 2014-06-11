/* A server utility file for creating sockets */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "tide_error.h"
#include "tide_server.h"
#include "tide_utils.h"

/* buffer size for reading and writing
*/
#define BUF 1024

/* create a new IPV6 socket
*/
void sock_6(int *sock, int port) {
    struct sockaddr_in6 sa;
    struct in6_addr any6 = IN6ADDR_LOOPBACK_INIT;
    
    sa.sin6_family = AF_INET6;
    sa.sin6_flowinfo = 0;
    sa.sin6_scope_id = 0;
    sa.sin6_port = htons(port);
    
    sa.sin6_addr = any6;
    
    int succ = bind(*sock, (struct sockaddr *) &sa, sizeof(sa));
    if (succ < 0) {
        close(*sock);
        error_exit("bind of server socket for version 6");
    }
}

/* create a new IPV4 socket
*/
void sock_4(int *sock, int port) {
    struct sockaddr_in sa;
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;
    
    int succ = bind(*sock, (struct sockaddr *) &sa, sizeof(sa));
    if (succ < 0) {
        close(*sock);
        error_exit("bind of socket for version 4");
    }
}

/* create a new server socket for use
   parameter is to check if we want an IPV6 socket or not
*/
int serv_sock(int use_ipv6, int port) {
    int sock;
    
    if (use_ipv6) {
        sock = socket(AF_INET6, SOCK_STREAM, 0);
        if (sock < 0)
            error_exit("creating server socket");
        sock_6(&sock, port);
    }
    else {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            error_exit("creating server socket");
        sock_4(&sock, port);
    }
    
    return sock;
}

/* our thread entrance function and client request handler
*/
void *handle_req(void *socket) {
    int sock = *((int *) socket);

    char *buf = safe_malloc(BUF);

    ssize_t bytes = read(sock, buf, BUF);

    if (bytes < 0) 
        server_error(sock, "reading from client");
    else {
        char *file = parse_get(buf);
        // TODO add logging
        if (file == NULL) {
            printf("not a GET request\n");
            free(file);
            close(sock);
            return NULL;
        }

        write_client(sock, file);
        free(file);
    }

    close(sock);
    free(buf);

    return NULL;
}

/* start listening on the socket we created
*/
void serve_forever(int sock) {
    if ((listen(sock, SOMAXCONN)) < 0) {
        error_exit("setting up socket to listen");
        exit(EXIT_FAILURE);
    }

    int client; // client socket that we accept
    
    struct sockaddr cli_sa;
    socklen_t cli_len = sizeof(cli_sa);

    // serve for eternity
    for (;;) {
        if ((client = accept(sock, &cli_sa, &cli_len)) < 0)
            server_error(sock, "accepting client");

        pthread_t cli_t; // thread id used for client connection
        if ((pthread_create(&cli_t, NULL, handle_req, (void *) &client)) < 0) {
            close(client);
            server_error(sock, "thread creation");
        }

        if ((pthread_join(cli_t, NULL)) < 0) {
            close(client);
            server_error(sock, "thread joining");
        }
    }
}
