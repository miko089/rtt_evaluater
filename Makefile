CC = gcc
CFLAGS = -Wall -O2

CLIENT_DIR = client
SERVER_DIR = server

CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.c)
SERVER_SRCS = $(wildcard $(SERVER_DIR)/*.c)

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

all: client_app server_app

client_app: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o client_app $(CLIENT_OBJS)

server_app: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server_app $(SERVER_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_DIR)/*.o $(SERVER_DIR)/*.o client_app server_app

.PHONY: all client_app server_app clean
