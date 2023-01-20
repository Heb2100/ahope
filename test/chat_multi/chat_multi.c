#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "chat_multi.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void quit_arrange(char* buf, int requesting_socket){
    memset(buf, 0, BUF_MAX);
    for (int cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
        if (_server_management.socks[cnt] == requesting_socket) {
            _server_management.socks[cnt] = 0;
        }
        if (_server_management.socks[cnt] == 0){
            if (_server_management.socks[cnt + 1] != 0){
                _server_management.socks[cnt] = _server_management.socks[cnt + 1];
                _server_management.socks[cnt + 1] = 0;
            }
        }
    }
    printf("id %d is disconnected, existing sock, ", requesting_socket);
    for (int cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
        printf("%d ", _server_management.socks[cnt]);
    } printf("\n");
    fflush(stdout);
}

void *server_proc_entry(struct server_thread_input *s_thread){
    int requesting_socket = s_thread->c_sock;
    int *pfile = s_thread->file_ptr;
    int r, cnt;
    char msg[INFO_MSG_LEN], log_msg[FILE_MSG_LEN];
    
    r = sprintf(msg, "Your socket id is %d\n", s_thread->c_sock);
    write(requesting_socket , msg , strlen(msg));
    
    pthread_mutex_lock(&mutex); 
    s_thread->c_num = 0;
    for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++) if (_server_management.socks[cnt] != 0) (s_thread->c_num)++;
    s_thread->c_num += 1;
    _server_management.socks[s_thread->c_num - 1] = requesting_socket;
    printf("existing socket, ");
    for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++) printf("%d ", _server_management.socks[cnt]);
    printf("\nclient num is %d\n", s_thread->c_num);
    pthread_mutex_unlock(&mutex);
    
    printf("socket id %d generated\n", s_thread->c_sock);
    memset(log_msg, 0, strlen(log_msg));
    sprintf(log_msg, "socket id %d generated\n", requesting_socket);
    fprintf(pfile, log_msg); 
    
    while (1) {
        int recv_len = 0, i, send_socket;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(requesting_socket, buf, BUF_MAX - 1, 0);
            if (recv_len < 0) goto onclose;
            if (!strcmp(buf, "quit\n")) { quit_arrange(buf, requesting_socket); goto onclose; }
            if (!strcmp(buf, "squit")) goto onserverclose;
            fprintf(pfile, buf); 
            for (i = 0; i < s_thread->c_num; i++){
                if (requesting_socket != _server_management.socks[i]) r = send(_server_management.socks[i], buf, strlen(buf), 0);
            }
            ch = buf[recv_len - 1];
        }
        fflush(pfile);
    }
onclose:
    memset(log_msg, 0, strlen(log_msg)); sprintf(log_msg, "socket id %d disconnected\n", requesting_socket); fprintf(pfile, log_msg); 
    close(requesting_socket);
    return 0;
onserverclose:
    memset(log_msg, 0, strlen(log_msg)); sprintf(log_msg, "server is closed\n"); fprintf(pfile, log_msg); 
    fclose(pfile);
    exit(0);
}

int *client_proc_entry(void *client_sock){
    int sock = (int)client_sock;
        
    while (1) {
        int recv_len = 0;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(sock, buf, BUF_MAX - 1, 0);
            if (!strcmp(buf, "quit\n")) { printf("client is disconnected\n"); break; }
            if (recv_len < 0) { printf("client is disconnected\n"); break; }
            ch = buf[recv_len - 1];
            printf("%s", buf);
        }
    }
    close(sock);
    return 0;
}

