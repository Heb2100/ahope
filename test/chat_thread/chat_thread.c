#include "chat_thread.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

int *server_proc_entry(void *client_sock){
    int sock = (int)client_sock;
    int r, cnt, requested_socket[THREAD_ID_LEN];
    char msg[INFO_MSG_LEN];
    int requesting_socket = sock;
    
    r = sprintf(msg, "Your socket id is %d\n", sock);
    write(sock , msg , strlen(msg));
    printf("existing sock, ");
    for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
        printf("%d ", _server_management.socks[cnt]);
    } printf("\n");
    
    printf("child thread generate sock %d\n", sock);
    
    while (1) {
        int recv_len = 0;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(sock, buf, BUF_MAX - 1, 0);
            if (recv_len < 0) { printf("recv failure\n"); break; }
            if (strstr(buf, "[c]")) {
                sscanf(buf, "[c] %d", requested_socket);
                printf("socket %d request connection and connected with socket %d\n", requesting_socket, requested_socket[0]);
                _server_management.connected_socks[0] = requesting_socket;
                _server_management.connected_socks[1] = requested_socket[0];
            }
            if (!strcmp(buf, "quit\n")) {
                memset(buf, 0, BUF_MAX);
                printf("id %d is disconnected, existing sock, ", requesting_socket);
                for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
                    if (_server_management.socks[cnt] == requesting_socket) _server_management.socks[cnt] = 0;
                    printf("%d ", _server_management.socks[cnt]);
                } printf("\n");
                break;
            }
            r = send(requesting_socket == _server_management.connected_socks[0] ? _server_management.connected_socks[1] : _server_management.connected_socks[0], buf, strlen(buf), 0);
            ch = buf[recv_len - 1];
        }
    }
    close(sock);
    return 0;
}

int *client_proc_entry(void *client_sock){
    int sock = (int)client_sock;
        
    while (1) {
        int recv_len = 0;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(sock, buf, BUF_MAX - 1, 0);
            if (!strcmp(buf, "quit\n")) { printf("client is disconnected by quit\n"); break; }
            if (recv_len < 0) { printf("client is disconnected by -1\n"); break; }
            ch = buf[recv_len - 1];
            printf("%s", buf);
        }
    }
    close(sock);
    return 0;
}

