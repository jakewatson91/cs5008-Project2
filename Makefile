.PHONY: all clean

# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Output log files
LOG_FILE = log.txt

TARGET = select_client select_server

all: $(TARGET)

select_client: select_client.c
	$(CC) $(CFLAGS) -o select_client select_client.c
	./select_client.c >> $(LOG_FILE) 

select_server: select_server.c
	$(CC) $(CFLAGS) -o select_server select_server.c logging.c
	./select_server >> $(LOG_FILE)

clean:
	rm -f $(TARGET) $(LOG_FILE)