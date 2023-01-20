#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#define PORT_LEN 8
#define IP_LEN 16
#define MSG_MAX 6
#define BUF_LEN 6
#define URL_LEN 128



int main(void){
    
    char cmd_str[4];
    short cmd;
    char tmp;
    int r;
    int opt = 1;
    char port[PORT_LEN];
    char ip[IP_LEN];
    char URL[URL_LEN];
    char proto[8];
    char path[32];
    int flag = 0;
    int flag_str = 0;
    
    memset(proto, 0, 8);
    memset(path, 0, 32);
    memset(URL, 0, URL_LEN);
    

    fputs("원하는 기능을 선택하세요(1 : 서버대기  2 : 클라이언트) : ", stdout);
    fgets(cmd_str, 4, stdin); 
    cmd = cmd_str[0];
    
    if (cmd == '1'){
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);
    
        fputs("ip를 입력하세요 : ", stdout);
        fgets(ip, sizeof(ip) , stdin);

        port[strlen(port)-1] = '\0';
        ip[strlen(ip)-1] = '\0';
    
        printf("포트넘버 %s, ip(도메인) %s로 통신합니다\n", port, ip);
    }
    


    
    if (cmd == '2'){
    
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);

        fputs("ip를 입력하세요 : ", stdout);
        fgets(ip, sizeof(ip) , stdin);
        
        port[strlen(port)-1] = '\0';
        ip[strlen(ip)-1] = '\0';
        
        printf("포트넘버 %s, ip(도메인) %s로 통신합니다\n", port, ip);
    }

    if(cmd == '1') {
        int server_sock;
        struct sockaddr_in sockaddr;
        
        /*create server socket*/
        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, (socklen_t)sizeof(opt)) < 0) printf("socket set opt failed");
        
        /*set server socket address*/
        memset(&sockaddr, 0, sizeof(struct sockaddr_in));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons((unsigned short int)atoi(port));
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        printf("socket address\n");
        
        /* bind */
        r = bind(server_sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));
        if (r < 0) { printf("bind failed Error code: %d\n", errno);  return -1;}
        printf("bind\n");

        /* listen */
        r = listen(server_sock, 5);
        if (r < 0) { printf("listen failed\n");  }
        printf("listen\n");
        {
            int client_sock;
            struct sockaddr_in client_addr;
            int socklen, recv_len = BUF_LEN;
            int is_first;
            char buf[BUF_LEN] = {0,};
            char ch;
            
            /* accept */
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&socklen);
            if (client_sock < 0) { printf("S accept failed\n"); return -1; }
            printf("S accept\n");
            
            while (1) {
                is_first = 1;
                ch = 0;
                while (ch != '\n') {
                    memset(buf, 0, BUF_LEN);
                    recv_len = recv(client_sock, buf, BUF_LEN, 0);
                    if (recv_len < 0) { printf("client close\n"); break; }
                    
                    ch = buf[recv_len - 1];
                    buf[recv_len - 1] = 0;
                    
                    if (is_first) { printf("msg : "); is_first = 0; }
                    printf("%s%c", buf, ch == '\n' ? '\n' : ch);
                }
            }
        }
        close(server_sock);
    }
    else if (cmd == '2'){
        
        int client_sock;
        struct sockaddr_in client_addr;
        char buf[BUF_LEN];
        int recv_len;
        char msg[MSG_MAX] = {0,};
        
        memset(msg, 0, MSG_MAX);
        
        /* client socket */
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        opt = 1;
        
        setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (client_sock < 0) { printf("C socket failed\n");  return -1;}
        printf("C socket\n");
        
        /* client_sockaddr */
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons((unsigned short int)atoi(port));
        inet_pton(AF_INET, ip, &client_addr.sin_addr.s_addr);
        printf("C socket address\n");
        
        /* connect */
        r = connect(client_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
        if (r < 0) { printf("C connect failed errno : %d\n", errno); return -1; }
        printf("C connect\n");
        
        while (1){
            fputs("메시지를 입력하세요 : ", stdout);
            fflush(stdout);
            while (1) {
                fgets(msg, MSG_MAX, stdin);
                r = send(client_sock, msg, strlen(msg), 0);
                if (msg[r-1] != '\n') break;
            } 
            
        }
        close(client_sock);
    } else printf("1, 2만 입력하세요");
        
    
	return 0;
}

