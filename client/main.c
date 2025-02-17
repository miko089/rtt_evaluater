#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

void usage(char *name) { printf("Usage: %s <server_ip> <port> <num_packets> <packet_size>\n", name); }
bool validate_number(char *num) { 
    if(strlen(num) > 9)return false; while(*num!='\0') { 
        if(*num<'0' || *num>'9') return false; 
        num++; 
    } 
    return true; 
}

int main(int argc, char **argv) {
    if(argc != 5) { 
        usage(argv[0]); 
        return 4;
    }
    int port, num_packets, packet_size;
    if (!validate_number(argv[2])) { 
        perror("port is not a number"); 
        usage(argv[0]); 
        return 5; 
    } else { 
        port = atoi(argv[2]); 
        if(port < 0 || port > 65535) { 
            perror("port is invalid"); 
            return 8; 
        } 
    }
    if (!validate_number(argv[3])) { 
        perror("num_packets is not a number"); 
        usage(argv[0]); 
        return 9; 
    } else { 
        num_packets = atoi(argv[3]); 
        if(num_packets <= 0) { 
            perror("num_packets must be positive"); 
            return 10; 
        } 
    }
    if (!validate_number(argv[4])) { 
        perror("packet_size is not a number"); 
        usage(argv[0]); 
        return 11; 
    } else { 
        packet_size = atoi(argv[4]); 
        if(packet_size < 16 || packet_size > 8192) { 
            perror("supported packet_size is 16 to 8192 bytes"); 
            return 12; 
        } 
    }
    char *server_ip = argv[1];
    char *send_buf = malloc(packet_size);
    if(send_buf == 0) { 
        perror("error allocating send buffer"); 
        return 13; 
    }
    memset(send_buf, 'A', packet_size);
    double *rtts = malloc(num_packets * sizeof(double));
    if(rtts == 0) { 
        perror("error allocating RTT array"); 
        free(send_buf); 
        return 14; 
    }
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) { 
        perror("socket couldn't be created"); 
        free(send_buf); 
        free(rtts); 
        return 1; 
    }
    int flags = fcntl(sock, F_GETFL, 0);
    if(flags == -1) { 
        perror("error getting flags"); 
        close(sock); free(send_buf); 
        free(rtts); 
        return 2; 
    }
    if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) { 
        perror("error setting non-blocking flag"); 
        close(sock); 
        free(send_buf); 
        free(rtts); 
        return 3; 
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) { 
        perror("invalid server IP"); 
        close(sock); 
        free(send_buf); 
        free(rtts); 
        return 15; 
    }
    socklen_t addr_len = sizeof(serv_addr);
    
    struct timespec send_time, recv_time;
    uint32_t seq = 0;
    
    int sent_packets = 0, received_packets = 0;
    while(sent_packets < num_packets) {
        if(clock_gettime(CLOCK_MONOTONIC, &send_time) != 0){ perror("clock_gettime"); break; }
        memcpy(send_buf, &seq, sizeof(uint32_t));
        ssize_t sent = sendto(sock, send_buf, packet_size, 0, (struct sockaddr *)&serv_addr, addr_len);
        if(sent < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK){ usleep(1000); continue; }
            else { perror("sendto"); break; }
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
        if(activity < 0){ if(errno == EINTR) continue; perror("select"); break; }
        if(activity > 0 && FD_ISSET(sock, &readfds)) {
            ssize_t recvd = recvfrom(sock, send_buf, packet_size, 0, NULL, NULL);
            if(recvd >= 0) {
                if(clock_gettime(CLOCK_MONOTONIC, &recv_time) != 0){ perror("clock_gettime"); break; }
                double rtt = (recv_time.tv_sec - send_time.tv_sec) * 1000.0 + (recv_time.tv_nsec - send_time.tv_nsec) / 1e6;
                rtts[seq] = rtt;
                received_packets++;
            } else { if(errno != EAGAIN && errno != EWOULDBLOCK){ perror("recvfrom"); break; } }
        }
        seq++;
        sent_packets++;
    }
    
    double sum = 0, min = 1e9, max = 0;
    int count = 0;
    for(int i = 0; i < num_packets; i++){
        if(rtts[i] > 0){ sum += rtts[i]; count++; if(rtts[i] < min) min = rtts[i]; if(rtts[i] > max) max = rtts[i]; }
    }
    double avg = (count > 0) ? sum / count : 0;
    double deviation = 0;
    for(int i = 0; i < num_packets; i++){
        if(rtts[i] > 0) deviation += (rtts[i] - avg) * (rtts[i] - avg);
    }
    deviation = (count > 0) ? deviation / count : 0;
    double stddev = sqrt(deviation);
    printf("Packets sent: %d\n Packets received: %d\n", sent_packets, received_packets);
    printf("RTT: avg = %.3f ms, stddev = %.3f ms, min = %.3f ms, max = %.3f ms\n", avg, stddev, min, max);
    free(send_buf);
    free(rtts);
    close(sock);
    return 0;
}
