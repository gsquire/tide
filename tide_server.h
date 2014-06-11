#ifndef TIDE_SERVER
#define TIDE_SERVER

void sock_4(int *sock, int port);
void sock_6(int *sock, int port);
int serv_sock(int use_ipv6, int port);
void *handle_req(void *socket);
void serve_forever(int sock);

#endif
