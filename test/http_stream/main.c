#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include "http.h"
#define SERVER_PORT_LEN 8
#define SERVER_BUF_LEN 6
#define CLIENT_URL_MAX 64
#define CLIENT_MSG_MAX 128

int main(void){
    char cmd[4];
    int r, opt = 1, sock;
    
    fputs("원하는 기능을 선택하세요(1 : 서버대기  2 : 클라이언트) : ", stdout);
    fgets(cmd, 4, stdin);
    
    if (cmd[0] == '1'){
        int client_sock;
        int is_first, socklen, recv_len = SERVER_BUF_LEN;
        char ch, buf[SERVER_BUF_LEN], port[SERVER_PORT_LEN];
        struct sockaddr_in addr;
        struct sockaddr_in client_addr;
        
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);
        port[strlen(port) - 1] = '\0';
        printf("포트 %s으로 통신\n", port);
        
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, (socklen_t)sizeof(opt)) < 0) printf("socket set opt failed\n");
        
        /* socket */
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((int)atoi(port));
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        printf("socket\n");

        r = bind(sock, (const struct sockaddr *)&addr, (socklen_t)sizeof(struct sockaddr_in));
        if (r < 0) { printf("bind failed Error code: %d\n", errno);  return -1;}
        printf("bind\n");

        r = listen(sock, 5);
        if (r < 0) { printf("listen failed\n");  }
        printf("listen\n");
        
        /* socket toward client */
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        client_sock = accept(sock, (struct sockaddr *)&client_addr, (socklen_t *)&socklen);
        if (client_sock < 0) { printf("S accept failed\n"); return -1; }
        printf("accept\n");
        
        while (1) {
            is_first = 1;
            ch = 0;
            while (ch != '\n') {
                memset(buf, 0, SERVER_BUF_LEN);
                recv_len = recv(client_sock, buf, SERVER_BUF_LEN, 0);
                if (recv_len < 0) { printf("client close\n"); break; }
                
                ch = buf[recv_len - 1];
                buf[recv_len - 1] = 0;
                
                if (is_first) { printf("msg : "); is_first = 0; }
                printf("%s%c", buf, ch == '\n' ? '\n' : ch);
            }
        }
        close(sock);
    }
    else if (cmd[0] == '2'){
        char url[CLIENT_URL_MAX], message[CLIENT_MSG_MAX], *ip = NULL;
        int send_len, msg_len, port = 80;
        struct sockaddr_in client_addr;
        struct hostent *host_infos;
        struct in_addr addr;
        struct http_req req;
        struct http_res *res;
        
        memset(message, 0, CLIENT_URL_MAX);
        
        while (1){
            fputs("도메인을 입력하세요 : ", stdout);
            fgets(url, sizeof(url) , stdin);
            
            parse_url(url, &req);
            if((host_infos = gethostbyname(req.dmn)) == NULL) {
                fprintf(stderr, "%s는 등록되지 않은 서버명입니다.\n", req.dmn);
                return -1;
            }
            printf("Official name은 %s ", host_infos->h_name);
            ip = inet_ntoa(*(struct in_addr*)host_infos->h_addr);
            printf("IP주소는 %s입니다\n", ip);
            
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0)  printf("C socket failed\n");
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, (socklen_t)sizeof(opt)) < 0) printf("socket set opt failed\n");
            
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(port);
            inet_pton(AF_INET, ip, &client_addr.sin_addr.s_addr);
            
            r = connect(sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
            if (r < 0) { printf("C connect failed errno : %d\n", errno); return -1; }
            
            msg_len = sprintf(message, "GET %s HTTP/1.1\nHost: %s\nUser-Agent: curl/7.68.0\nAccept: */*\n\n", req.path, req.dmn);
            free(req.dmn); free(req.path);
            printf("\n<<request msg>>\n%s", message);

            send_len = send(sock, message, msg_len, 0);
            if (send_len < 1) printf("C send failed\n");
            
            res = get_response_from_server(sock);
            free(res);
            
            if (res->is_content_len) {
                printf("%s", res->header_left);
                /* cut off tmp and /r/n */
                res->body_len = atoi(res->content_len_str) - strlen(res->header_left) + 1;
                res->recv_body_len = recv(sock, res->body, res->body_len, 0);
                if (res->recv_body_len < 1) printf("recv failed %d\n", errno);
                printf("%s\n", res->body);
            } else if (res->is_chunked) {
                printf("%s", res->header_left);
                res->chunk_start = res->header_left + 2;
                res->chunk_end = strstr(res->chunk_start, "\r\n");
                memcpy(res->chunk_hex, res->chunk_start, res->chunk_end - res->chunk_start);
                /* remove \r\n hex number \r\n */
                res->excess_tmp_len = strlen(res->header_left) - strlen(res->chunk_hex) - 4;
                res->chunk_len = strtol(res->chunk_hex, NULL, 16) - res->excess_tmp_len;
                res->tmp_len = 0;
                while (res->tmp_len <= res->chunk_len){
                    res->recv_body_len = recv(sock, res->body, res->chunk_len, 0);
                    res->tmp_len += res->recv_body_len;
                    printf("%s", res->body);
                    printf("\ninput %d total intput is %d\n", res->recv_body_len, res->tmp_len);
                }
                printf("chunk_len is %d\n", res->chunk_len);
            } else {
                printf("Content length or chunked doesnt exist\n");
            }
//            free(res);
            close(sock);
        }
    } else printf("1, 2만 입력하세요");
        
    
	return 0;
}

