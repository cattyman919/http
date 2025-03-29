#include "server.h"
#include "routes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void) {
  int listener; // Listening socket descriptor
  int yes = 1;  // For setsockopt() SO_REUSEADDR, below
  int rv;

  struct addrinfo hints, *ai, *p;

  // Get us a socket and bind it
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }

    // Lose the pesky "address already in use" error message
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }

    break;
  }

  // If we got here, it means we didn't get bound
  if (p == NULL) {
    return -1;
  }

  freeaddrinfo(ai); // All done with this

  // Listen
  if (listen(listener, 10) == -1) {
    return -1;
  }

  return listener;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count,
                 int *fd_size) {
  // If we don't have room, add more space in the pfds array
  if (*fd_count == *fd_size) {
    *fd_size *= 2; // Double it

    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
  }

  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
  (*pfds)[*fd_count].revents = 0;

  (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
  // Copy the one from the end over this one
  pfds[i] = pfds[*fd_count - 1];

  (*fd_count)--;
}

char *render_static_file(char *fileName) {
  FILE *file = fopen(fileName, "r");

  if (file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *temp = malloc(sizeof(char) * (fsize + 1));
  char ch;
  int i = 0;
  while ((ch = fgetc(file)) != EOF) {
    temp[i] = ch;
    i++;
  }
  fclose(file);
  return temp;
}

void handle_client(int clientfd, char buf[], Route *root) {

  printf("[LOG] HTTP Request :\n%s", buf);

  // Parsing client HTTP request
  char *method = "";
  char *urlRoute = "";
  char *client_http_header = strtok(buf, "\n");

  printf("[LOG] Client HTTP Header: %s\n", client_http_header);
  char *header_token = strtok(client_http_header, " ");

  int header_parse_counter = 0;
  while (header_token != NULL) {

    switch (header_parse_counter) {
    case 0:
      method = header_token;
      break;
    case 1:
      urlRoute = header_token;
      break;
    }
    header_token = strtok(NULL, " ");
    header_parse_counter++;
  }

  printf("[LOG] Client HTTP Method: %s\n", method);
  printf("[LOG] Client HTTP URL: %s\n", urlRoute);

  // Find the HTML file based on the URL Route
  char template[100] = "templates/";

  printf("[LOG] Searching path...\n");

  Route *destination = search(root, urlRoute);
  if (destination == NULL)
    strcat(template, "404.html");
  else
    strcat(template, destination->value);

  printf("[LOG] Path : %s\n", template);

  // Render the HTML file as characters
  char *response_data = render_static_file(template);

  // {Status Line} \r\n {Headers} \r\n {Body}
  char res[BUFFER_SIZE * 4] = "HTTP/1.1 200 OK\r\n\r\n";
  strcat(res, response_data);
  strcat(res, "\r\n\r\n");

  printf("[LOG] HTTP Response :\n%s", res);

  send(clientfd, res, sizeof(res), 0);
  close(clientfd);

  free(response_data);
}
