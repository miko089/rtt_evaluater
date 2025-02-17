#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

void usage(char* name) {
    printf("Usage: %s <port> <size of packet>", name);
}

bool validate_number(char* num) {
    if (strlen(num) > 9) return false;
    while (*num != '\0') {
        if (*num < '0' || *num > '9') return false;
        num++;
    }
    return true;
}

int main(int argc, char** argv) {
    int port, buffer_size;
    if (argc != 3) {
        usage(argv[0]);
        return 4;
    }
    if (!validate_number(argv[1])) {
        perror("port is not a number");
        usage(argv[0]);
        return 5;
    } else {
        port = atoi(argv[1]);
        if (port < 0 || port > 65535) {
            perror("port is invalid");
            return 8;
        }
    } 
    if (!validate_number(argv[2])) {
        perror("packet_size is not a number");
        usage(argv[0]);
        return 9;
    } else {
        buffer_size = atoi(argv[2]);
        if (buffer_size < 16 || buffer_size > 8192) {
            perror("supported packet_size is 16 to 8192 bytes");
            return 12;
        }
    }

    struct sockaddr_in serv_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char* buffer = malloc(buffer_size);
    if (buffer == 0) {
        perror("error with buffer allocation");
        return 13;
    }
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket couldn't be created");
        return 1;
    } 

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror("error getting current flags");
        close(sock);
        return 2;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("error setting non-blocking flag");
        close(sock);
        return 3;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("error binding");
        close(sock);
        return 15;
    }

    printf("Server started on %d port\n", port);
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval timeout;
        timeout.tv_sec  = 5;
        timeout.tv_usec = 0;
        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("select errored");
            close(sock);
            return 78;
        }
        if (activity == 0) {
            printf("No data in last 5 secs..(\n");
            continue;
        }
        if (FD_ISSET(sock, &readfds)) {
           ssize_t recv_bytes = recvfrom(sock, buffer, buffer_size, 0,
                                         (struct sockaddr *)&client_addr, &addr_len);

           if (recv_bytes < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } else {
                    perror("error reading");
                    close(sock);
                    return 86;
                }
           }
           if (recv_bytes == 0) {
                continue;
           }
           if (recv_bytes > 0) {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                printf("message from %s, %zd bytes!!!!\n", client_ip, recv_bytes);
                ssize_t sent_bytes = sendto(sock, buffer, recv_bytes,
                       0, (struct sockaddr *)&client_addr, addr_len);
                if (sent_bytes < 0) {
                    perror("error reply"); 
                } else {
                    printf("Reply sent, %zd bytes\n", sent_bytes);
                }
           }
        }
    }
    close(sock);
    return 0;
}
