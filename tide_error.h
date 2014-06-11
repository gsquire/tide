#ifndef TIDE_ERROR 
#define TIDE_ERROR

void error_exit(const char *message);
void server_error(int sock, const char *message);

#endif
