#include "server.h"
#include <stdio.h>

int main() {

  int listener; // Listening socket descriptor

  int newfd;                          // Newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr; // Client address
  socklen_t addrlen;

  char buf[256]; // Buffer for client data

  char remoteIP[INET6_ADDRSTRLEN];

  // Start off with room for 5 connections
  // (We'll realloc as necessary)
  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

  printf("Setting up socket...\n");
  // Set up and get a listening socket
  listener = get_listener_socket();

  if (listener == -1) {
    fprintf(stderr, "error getting listening socket\n");
    exit(1);
  }

  printf("Listening on localhost:%s\n", PORT);
  // Add the listener to set
  pfds[0].fd = listener;
  pfds[0].events = POLLIN; // Report ready to read on incoming connection

  fd_count = 1; // For the listener

  // Main loop
  for (;;) {
    int poll_count = poll(pfds, fd_count, -1);

    if (poll_count == -1) {
      perror("poll");
      exit(1);
    }

    // Run through the existing connections looking for data to read*/
    for (int i = 0; i < fd_count; i++) {

      // Check if someone's ready to read*/
      if (pfds[i].revents & (POLLIN | POLLHUP)) { // We got one!!*/

        if (pfds[i].fd == listener) {
          // If listener is ready to read, handle new connection*/

          addrlen = sizeof remoteaddr;
          newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

          if (newfd == -1) {
            perror("accept");
          } else {
            add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

            printf("[LOG] New connection from %s on "
                   "socket %d\n",
                   inet_ntop(remoteaddr.ss_family,
                             get_in_addr((struct sockaddr *)&remoteaddr),
                             remoteIP, INET6_ADDRSTRLEN),
                   newfd);
          }
        } else {
          // If not the listener, we're just a regular client*/
          int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

          int sender_fd = pfds[i].fd;

          printf("[LOG] Bytes received %d from socket %d\n", nbytes, sender_fd);

          if (nbytes <= 0) {
            // Got error or connection closed by client*/
            if (nbytes == 0) {
              // Connection closed*/
              printf("[LOG] Socket %d hung up\n", sender_fd);
            } else {
              perror("recv");
            }

            close(pfds[i].fd); // Bye!*/

            del_from_pfds(pfds, i, &fd_count);

          } else {

            printf("[LOG] Sending html files to socket %d\n", sender_fd);
            handle_client(sender_fd, buf);
            del_from_pfds(pfds, i, &fd_count);
          }
        }
      }
    }
  }

  free(pfds);
  printf("Exiting\n");
  return 0;
}
