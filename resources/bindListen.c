/*
** server.c -- a stream socket server demo
** This program demonstrates a basic TCP server that listens for incoming
** connections, accepts them, and responds with a simple "Hello, world!" message.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // The port clients will connect to
#define BACKLOG 10   // Maximum number of pending connections in the queue

// Signal handler for cleaning up zombie processes
void sigchld_handler(int s) {
    (void)s; // Suppress unused variable warning

    // Save the current errno to restore it later
    int saved_errno = errno;

    // Wait for any child processes that have terminated
    while (waitpid(-1, NULL, WNOHANG) > 0);

    // Restore the original errno
    errno = saved_errno;
}

// Helper function to retrieve the IPv4 or IPv6 address from a sockaddr structure
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) { // If the address is IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    // Otherwise, assume it's IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    int sockfd, new_fd;  // sockfd is the listening socket, new_fd is for accepted connections
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Client's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN]; // Buffer for storing the client's IP address
    int rv;

    // Configure the hints structure for getaddrinfo
    memset(&hints, 0, sizeof hints); // Zero out the hints structure
    hints.ai_family = AF_UNSPEC;       // Use either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;   // Use a TCP stream socket
    hints.ai_flags = AI_PASSIVE;       // Automatically fill in the local IP address

    // Get a list of address structures suitable for binding
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Report errors from getaddrinfo
        return 1;
    }

    // Loop through the address list and attempt to bind to one
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Create a socket for this address
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket"); // Report socket creation errors
            continue; // Try the next address
        }

        // Set socket options to allow reusing the address
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Bind the socket to the address
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd); // Close the socket if binding fails
            perror("server: bind");
            continue;
        }

        break; // If we successfully bind, stop the loop
    }

    freeaddrinfo(servinfo); // Free the address list after binding

    // If no addresses were valid for binding
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Set up the signal handler to clean up zombie child processes
    sa.sa_handler = sigchld_handler; // Use sigchld_handler for SIGCHLD signals
    sigemptyset(&sa.sa_mask);        // Block no additional signals
    sa.sa_flags = SA_RESTART;        // Restart interrupted system calls
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    // Main loop to accept and handle incoming connections
    while (1) {
        sin_size = sizeof their_addr; // Size of the client's address structure

        // Accept an incoming connection
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept"); // Report accept errors
            continue;         // Try the next connection
        }

        // Convert the client's address to a human-readable string
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        // Fork a new process to handle the client
        if (!fork()) { // This is the child process
            close(sockfd); // The child doesn't need the listening socket
            if (send(new_fd, "Hello, world!", 13, 0) == -1) { // Send a message to the client
                perror("send");
            }
            close(new_fd); // Close the client's socket
            exit(0);       // Terminate the child process
        }

        close(new_fd); // The parent doesn't need the client socket
    }

    return 0;
}
