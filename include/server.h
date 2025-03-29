#ifndef Server_h
#define Server_h

#include "routes.h"
#include <arpa/inet.h>
#include <poll.h>

#define BUFFER_SIZE 1024 * 2
#define PORT "8080" // Port we're listening on

void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(void);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);
void handle_client(int clientfd, char buf[], Route *root);
char *render_static_file(char *fileName);

#endif
