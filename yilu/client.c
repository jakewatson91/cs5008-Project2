#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 3200
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Connect to server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return EXIT_FAILURE;
    }

    printf("Connected to the server.\n");

    while (1) {
        // Receive server prompt
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("Disconnected from the server.\n");
            break;
        }

        printf("%s", buffer);

        // If server sends "TIME OUT!" message, disconnect
        if (strstr(buffer, "TIME OUT!") != NULL) {
            printf("Server timed out. Closing connection.\n");
            break;
        }

        // Get user input
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);

        // Send input to server
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
