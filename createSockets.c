// Create a socket using the address information from getaddrinfo
if ((socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
    // If socket creation fails, print an error and continue to the next result
    perror("server: socket");
}

// Set socket options to allow reuse of the same address
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    // If setting socket options fails, print an error and exit
    perror("setsockopt");
    exit(1);
}
