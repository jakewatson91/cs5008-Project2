#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
    const char *port = "3200";
    struct addrinfo hints, *host;
    int r, sockfd;
    char *filename;
    FILE *fp;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server> <image_file>\n", argv[0]);
        exit(1);
    }
    const char *server = argv[1];
    filename = argv[2];

    // Open the file in binary mode
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Get the file size
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Resolve the server's address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    r = getaddrinfo(server, port, &hints, &host);
    if (r != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        fclose(fp);
        exit(1);
    }

    // Create the socket
    sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
    if (sockfd == -1) {
        perror("Socket creation failed");
        freeaddrinfo(host);
        fclose(fp);
        exit(1);
    }

    // Connect to the server
    r = connect(sockfd, host->ai_addr, host->ai_addrlen);
    if (r == -1) {
        perror("Connect failed");
        freeaddrinfo(host);
        fclose(fp);
        close(sockfd);
        exit(1);
    }
    freeaddrinfo(host);

    // Send the file size to the server
    filesize = htonl(filesize); // Convert to network byte order
    if (send(sockfd, &filesize, sizeof(filesize), 0) == -1) {
        perror("Failed to send file size");
        fclose(fp);
        close(sockfd);
        exit(1);
    }

    // Send the file contents
    char buffer[BUFSIZ];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            perror("Failed to send file data");
            fclose(fp);
            close(sockfd);
            exit(1);
        }
    }

    printf("File sent successfully!\n");

    // Clean up
    fclose(fp);
    close(sockfd);
    return 0;
}
