// linux-specific server implementation
#if !defined(_MSC_VER) && !defined(__MINGW32__) && !defined(__MINGW64__)

// Taken from http://beej.us/guide/bgnet/html
//
// "The C source code presented in this document is hereby granted to
// the public domain, and is completely free of any license restriction."
//
// Modifications made to support an in-process callback

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <functional>

// active client
static int clientSocket = 0;

// number of queued connections to make
static const int backlog = 10;

void sigchld_handler(int /*s*/) {
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int createServer(const char *port, std::function<void()> callback) {
  int listenSocket; // listen on sock_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((listenSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(listenSocket, p->ai_addr, p->ai_addrlen) == -1) {
      close(listenSocket);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(listenSocket, backlog) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while (1) { // main accept() loop
    printf("server: waiting for connection...\n");
    sin_size = sizeof their_addr;
    clientSocket =
        accept(listenSocket, (struct sockaddr *)&their_addr, &sin_size);
    if (clientSocket == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);

    callback(); // call callback
  }

  close(listenSocket);
  return 0;
}

int readSocket(char *recvbuf, int recvbuflen) {
  ssize_t n = read(clientSocket, recvbuf, recvbuflen);
  if (n < 0) {
    perror("socket: read");
    close(clientSocket);
  }
  return n;
}

int writeSocket(const char *sendbuf, int sendbuflen) {
  ssize_t n = write(clientSocket, sendbuf, sendbuflen);
  if (n < 0) {
    perror("socket: write");
    close(clientSocket);
  }
  return n;
}

void closeSocket() { close(clientSocket); }

#endif
