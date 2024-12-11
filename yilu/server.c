#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>

#define DEFAULT_PORT 3200
#define MAX_USERS 3
#define BUFFER_SIZE 1024
#define MAX_REQUESTS 2
#define TIME_WINDOW 60 // Time window in seconds for rate limiting

int running = 1; // Server running flag
int active_users = 0; // Number of active users
pthread_mutex_t user_lock; // Mutex to protect active_users

typedef struct {
    int request_count;
    time_t first_request_time;
} client_rate_data;

// Function prototypes
void *handle_client(void *client_socket);
void log_event(const char *event, const char *client_ip);
int check_rate_limit(client_rate_data *rate_data);
int file_exists(const char *filename);

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int port = DEFAULT_PORT;

    // Initialize the mutex
    pthread_mutex_init(&user_lock, NULL);

    // Create and bind server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_USERS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (running) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Get the client's IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        pthread_mutex_lock(&user_lock);
        if (active_users >= MAX_USERS) {
            // Too many users; reject the connection
            const char *busy_message = "Server is busy. Please try again later.\n";
            send(client_socket, busy_message, strlen(busy_message), 0);
            close(client_socket);
            log_event("Connection refused: server is busy", client_ip);
            pthread_mutex_unlock(&user_lock);
            continue;
        }

        // Increment the active user count and log the connection
        active_users++;
        log_event("Connection accepted", client_ip);
        pthread_mutex_unlock(&user_lock);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)&client_socket);
        pthread_detach(thread); // Automatically free thread resources
    }

    close(server_socket);
    pthread_mutex_destroy(&user_lock);
    printf("Server shut down.\n");
    return 0;
}

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    client_rate_data rate_data = {0, 0}; // Initialize rate data

    while (1) {
        // Send menu prompt to client
        const char *menu = "Enter 'close' to disconnect, 'shutdown' to turn off the server, or a QR code file name to get the URL: ";
        send(sock, menu, strlen(menu), 0);

        // Read client input
        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        // Check for empty input
        if (strlen(buffer) == 0) {
            const char *empty_message = "Need an existing filename.\n";
            send(sock, empty_message, strlen(empty_message), 0);
            continue;
        }

        // Rate limiting check
        if (check_rate_limit(&rate_data) == 0) {
            const char *rate_limit_message = "Too many requests, please wait.\n";
            send(sock, rate_limit_message, strlen(rate_limit_message), 0);
            continue; // Skip processing and wait for the next command
        }

        if (strcmp(buffer, "close") == 0) {
            printf("Client requested to disconnect.\n");
            break;
        } else if (strcmp(buffer, "shutdown") == 0) {
            printf("Client requested to shut down the server.\n");
            running = 0;
            break;
        } else { // Assume it's a file name
            printf("Client sent file name: %s\n", buffer);

            // Check if the file exists
            if (!file_exists(buffer)) {
                const char *file_not_found_message = "File doesn't exist.\n";
                send(sock, file_not_found_message, strlen(file_not_found_message), 0);
                continue; // Skip processing and wait for the next command
            }

            // Invoke ZXing library to decode QR code
            char command[BUFFER_SIZE];
            snprintf(command, BUFFER_SIZE, "java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner %s", buffer);
            printf("Executing command: %s\n", command);
            int ret = system(command);
            if (ret != 0) {
                const char *error_message = "Error processing the QR code file.\n";
                send(sock, error_message, strlen(error_message), 0);
            }
        }
    }

    // Decrement active user count when client disconnects
    pthread_mutex_lock(&user_lock);
    active_users--;
    pthread_mutex_unlock(&user_lock);

    close(sock);
    return NULL;
}

// Check if a client exceeds the rate limit
int check_rate_limit(client_rate_data *rate_data) {
    time_t now = time(NULL);

    if (rate_data->first_request_time == 0 || now - rate_data->first_request_time > TIME_WINDOW) {
        // Reset rate limit window
        rate_data->first_request_time = now;
        rate_data->request_count = 1;
        return 1; // Allowed
    }

    if (rate_data->request_count < MAX_REQUESTS) {
        // Increment request count within the time window
        rate_data->request_count++;
        return 1; // Allowed
    }

    return 0; // Exceeded rate limit
}

// Check if a file exists
int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

// Log events with timestamps and client IP
void log_event(const char *event, const char *client_ip) {
    FILE *log_file = fopen("server.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d %s: %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            client_ip, event);

    fclose(log_file);
}
