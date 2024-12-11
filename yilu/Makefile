# Compiler and Flags
CC = gcc
CFLAGS = -Wall -pthread
TARGETS = server client

# Default target
all: $(TARGETS)

# Server target
server: server.c
	$(CC) $(CFLAGS) -o server server.c

# Client target
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Clean target to remove binaries and temporary files
clean:
	rm -f $(TARGETS) *.o

# Phony targets
.PHONY: all clean
