#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include "chat_multi.h"

// 단톡. 로그.
int main(void){
    struct http_methods *methods;
    struct server_management s_manage;
    char cmd_str[4], port[IP_LEN], ip[IP_LEN],  file_name[FILE_MSG_LEN];
    short cmd;
    int r, opt = 1;
    pthread_t thread_id[CLIENT_NUM_MAX];
    time_t time_raw_format;
    struct tm *ptr_time;
    
    fputs("원하는 기능을 선택하세요(1 : 서버대기  2 : 클라이언트) : ", stdout);
    fgets(cmd_str, 4, stdin); 
    cmd = cmd_str[0];
    
    if (cmd == '1'){
        int server_sock, client_sock, socklen, cnt = 0, i;
        struct sockaddr_in sockaddr;
        struct sockaddr_in clientaddr;
        struct server_thread_input s_thread;
        
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);
        port[strlen(port) - 1] = 0;
        printf("포트넘버 %s로 통신합니다\n", port);
        
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, (socklen_t)sizeof(opt)) < 0) printf("socket set opt failed");
        
        memset(&sockaddr, 0, sizeof(struct sockaddr_in));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons((int)atoi(port));
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        r = bind(server_sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)); if (r < 0) { printf("bind failed %d\n", errno);  return -1;}
        r = listen(server_sock, CLIENT_NUM_MAX); if (r < 0) { printf("listen failed\n"); return -1; }
        puts("Waiting for incoming connections...");
        
        time(&time_raw_format);
        ptr_time = localtime(&time_raw_format);
        if (strftime(file_name, 50, "%Y-%m-%d-%H:%M:%S", ptr_time) == 0) perror("Couldn't prepare formatted string");
        else s_thread.file_ptr = fopen(file_name, "w");
        
        while(!s_manage.socks[CLIENT_NUM_MAX - 1]) {
            client_sock = accept(server_sock, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
            printf("accepted client sock is %d\n", client_sock);
            s_thread.c_sock = client_sock;
            if (client_sock < 0) { perror("accept failed"); break; }
            r = pthread_create(&thread_id[cnt], NULL, server_proc_entry, &s_thread);
            if (r < 0) { perror("thread create failed"); break; } 
            cnt++;
        }
    }
    if (cmd == '2'){
        int client_sock, is_first, cnt = 0;
        struct sockaddr_in client_addr;
        char msg[BUF_MAX], client_id_msg[INFO_MSG_LEN], thread_id[THREAD_ID_LEN], send_msg[INFO_MSG_LEN], client_id[INFO_MSG_LEN];
        
        memset(msg, 0, BUF_MAX);
        memset(client_id_msg, 0, INFO_MSG_LEN);
        
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);
        fputs("ip를 입력하세요 : ", stdout);
        fgets(ip, sizeof(ip) , stdin);
        port[strlen(port)-1] = '\0';
        ip[strlen(ip)-1] = '\0';
        printf("포트넘버 %s, ip(도메인) %s로 통신합니다\n", port, ip);
        
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (client_sock < 0) { printf("C socket failed\n");  return -1;}
        
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons((unsigned short int)atoi(port));
        inet_pton(AF_INET, ip, &client_addr.sin_addr.s_addr);
        
        r = connect(client_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
        if (r < 0) { printf("C connect failed errno : %d\n", errno); return -1; }
        printf("client connect\n");
        
        r = recv(client_sock, client_id_msg, INFO_MSG_LEN, 0);
        printf("%s", client_id_msg);
        sscanf(client_id_msg, "Your socket id is %s\n", client_id);
        
        r = pthread_create(&thread_id[cnt], NULL, client_proc_entry, (void *)client_sock);
        sprintf(send_msg, "msg from client %s : ", client_id);
        
        while (1) {
            is_first = 1;
            fputs(send_msg, stdout);
            while (1){
                fgets(msg, BUF_MAX, stdin);
                if (is_first && (strcmp(msg, "quit\n")) && (strcmp(msg, "squit"))) {send(client_sock, send_msg, strlen(send_msg), 0); is_first = 0;}
                r = send(client_sock, msg, strlen(msg), 0);
                if (msg[r-1] == '\n') break;
            }
        }
    onclose:
        printf("socket id %s is disconnected\n", client_id);
        close(client_sock);
        return -1;
        
    } else printf("1, 2만 입력하세요");
    
}

