#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include "chat_select.h"
#include <pthread.h>


int main(void){
    
    char cmd_str[4];
    short cmd;
    int r, opt = 1;
    char port[PORT_LEN], ip[IP_LEN];
    
    fputs("원하는 기능을 선택하세요(1 : 서버대기  2 : 클라이언트) : ", stdout);
    fgets(cmd_str, 4, stdin); 
    cmd = cmd_str[0];
    
    if (cmd == '1') {
        struct sockaddr_in sockaddr;
        int client_sock[CLIENT_NUM_MAX], server_sock[CLIENT_NUM_MAX];
        struct sockaddr_in client_addr;
        int socklen, recv_len = BUF_LEN, cnt = 0;
        char buf[BUF_LEN] = {0};
        int port[CLIENT_NUM_MAX] = {5050, 5454, 5656};
        memset(server_sock, 0, sizeof(server_sock));
        memset(client_sock, 0, sizeof(client_sock));
        
        while (cnt < CLIENT_NUM_MAX){
            
            printf("포트넘버 %d로 통신합니다\n", port[cnt]);
            
            /*create server socket*/
            server_sock[cnt] = socket(AF_INET, SOCK_STREAM, 0);
            if (setsockopt(server_sock[cnt], SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, (socklen_t)sizeof(opt)) < 0) printf("socket set opt failed");
            
            /*set server socket address*/
            memset(&sockaddr, 0, sizeof(struct sockaddr_in));
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_port = htons((port[cnt]));
            sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
            printf("socket address\n");
            
            /* bind */
            r = bind(server_sock[cnt], (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));
            if (r < 0) { printf("bind failed Error code: %d\n", errno);  return -1;}
            printf("bind\n");

            /* listen */
            r = listen(server_sock[cnt], 5);
            if (r < 0) { printf("listen failed\n");  }
            printf("listen\n");
            
            /* accept */
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_sock[cnt] = accept(server_sock[cnt], (struct sockaddr *)&client_addr, (socklen_t *)&socklen);
            if (client_sock < 0) { printf("S accept failed\n"); return -1; }
            printf("S accept sock %d\n", client_sock[cnt]);
            
            while (cnt == CLIENT_NUM_MAX - 1) {
                fd_set readfds;
                int max_fd, is_first, i;
                char ch, init_msg[INFO_MSG_LEN];
                FD_ZERO(&readfds);
                
                for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
                    FD_SET(client_sock[cnt], &readfds);
                    max_fd = (max_fd > client_sock[cnt]) ? max_fd : client_sock[cnt];
                }
                
                r = select(max_fd + 1, &readfds, NULL, NULL, NULL);
                if (r < 0) { printf("select failed \n"); return 0; }
                
                for (cnt = 0; cnt < CLIENT_NUM_MAX; cnt++){
                    if (FD_ISSET(client_sock[cnt], &readfds)){
                        sprintf(init_msg, "msg from sock %d : ", client_sock[cnt]);
                        is_first = 1; ch = 0;
                        while (ch != '\n') {
                            memset(buf, 0, BUF_LEN);
                            recv_len = recv(client_sock[cnt], buf, BUF_LEN - 1, 0);
                            if (recv_len < 0) { printf("client close\n"); break; }
                            ch = buf[recv_len - 1];
                            for (i = 0; i < CLIENT_NUM_MAX; i++){
                                if (client_sock[i] != client_sock[cnt]) {
                                    if (is_first) { send(client_sock[i], init_msg, strlen(init_msg), 0); }
                                    send(client_sock[i], buf, strlen(buf), 0);
                                }
                            }
                            if (is_first) is_first = 0; 
                        }
                    }
                }
                cnt = CLIENT_NUM_MAX - 1;
            }
            cnt++;
        }
    }
    else if (cmd == '2'){
        
        int client_sock;
        struct sockaddr_in client_addr;
        int thread_id[CLIENT_NUM_MAX], cnt = 0, is_first;
        char msg[MSG_MAX] = {0,}, send_msg[INFO_MSG_LEN];
        
        memset(msg, 0, MSG_MAX);
        
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);

        fputs("ip를 입력하세요 : ", stdout);
        fgets(ip, sizeof(ip) , stdin);
        
        port[strlen(port)-1] = '\0';
        ip[strlen(ip)-1] = '\0';
        
        printf("포트넘버 %s, ip(도메인) %s로 통신합니다\n", port, ip);
        
        /* client socket */
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        opt = 1;
        
        setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (client_sock < 0) { printf("C socket failed\n");  return -1;}
        printf("C socket\n");
        
        /* client_sockaddr */
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons((int)atoi(port));
        inet_pton(AF_INET, ip, &client_addr.sin_addr.s_addr);
        printf("C socket address\n");
        
        /* connect */
        r = connect(client_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
        if (r < 0) { printf("C connect failed errno : %d\n", errno); return -1; }
        printf("C connect\n");
        
        /* receiver */
        r = pthread_create(&thread_id[cnt], NULL, client_proc_entry, (void *)client_sock);
        sprintf(send_msg, "메시지를 입력하세요 : ");
                
        /* sender */
        while (1) {
            is_first = 1;
            fputs(send_msg, stdout);
            while (1){
                fgets(msg, BUF_MAX, stdin);
                r = send(client_sock, msg, strlen(msg), 0);
                if (msg[r-1] == '\n') break;
            }
        }
        close(client_sock);
    } else printf("1, 2만 입력하세요");
        
    
	return 0;
}

