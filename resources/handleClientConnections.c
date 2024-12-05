#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> // For inet_ntoa()

#define PORT    5555    // Port to listen on
#define MAXMSG  512     // Maximum message size

/* Function to read data from a client socket */
int read_from_client(int filedes) {
    char buffer[MAXMSG];
    int nbytes;

    // Read data from the socket
    nbytes = read(filedes, buffer, MAXMSG);
    if (nbytes < 0) {
        /* If there is a read error, print the error and exit */
        perror("read");
        exit(EXIT_FAILURE);
    } else if (nbytes == 0) {
        /* End-of-file: the client has closed the connection */
        return -1;
    } else {
        /* Data has been read; print the message */
        fprintf(stderr, "Server: got message: `%s'\n", buffer);
        return 0;
    }
}

int main(void) {
    extern int make_socket(uint16_t port); // External function to create a socket
    int sock;                              // Listening socket
    fd_set active_fd_set, read_fd_set;    // Sets of file descriptors for select()
    int i;                                // Iterator for looping through descriptors
    struct sockaddr_in clientname;        // Client's address
    size_t size;

    /* Create the listening socket and bind it to the specified port */
    sock = make_socket(PORT);
    if (listen(sock, 1) < 0) { // Listen for incoming connections
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Initialize the set of active sockets */
    FD_ZERO(&active_fd_set);     // Clear the set
    FD_SET(sock, &active_fd_set); // Add the listening socket to the set

    while (1) {
        /* Block until input arrives on one or more sockets */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all sockets with input pending */
        for (i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &read_fd_set)) { // Check if this socket is ready
                if (i == sock) {
                    /* New connection request on the listening socket */
                    int new_sock;
                    size = sizeof(clientname);
                    new_sock = accept(sock, (struct sockaddr *) &clientname, &size);
                    if (new_sock < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr,
                            "Server: connect from host %s, port %hd.\n",
                            inet_ntoa(clientname.sin_addr),  // Convert IP to string
                            ntohs(clientname.sin_port));    // Convert port to host byte order
                    FD_SET(new_sock, &active_fd_set); // Add the new socket to the set
                } else {
                    /* Data arriving on an already-connected socket */
                    if (read_from_client(i) < 0) {
                        /* If the client closed the connection, close the socket and remove it */
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
        }
    }
}
