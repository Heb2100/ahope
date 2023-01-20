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
#define PORT_LEN 8
#define MSG_MAX 6
#define BUF_LEN 6
#define PORT 80
#define RECV_MAX 512
#define IP_LEN 32
#define URL_LEN 128
char domain[URL_LEN];

char* domain_to_ip(char *domain, char *path){
    char URL[URL_LEN];
    char proto[IP_LEN];
    char *temp, *temp2;
    int proto_len;
    char **path_ptr = path;
    memset(proto, 0, IP_LEN);
    memset(URL, 0, IP_LEN);
    
    /* domain, path, ip settingz */
    temp = strstr(domain, "://");
    if (temp != 0) {
        strncpy(URL, domain, URL_LEN);
        proto_len = (temp-domain);
        memcpy(proto, URL, proto_len);
        strcpy(URL, temp+3);
        memset(domain, 0, strlen(domain));
        strcpy(domain, URL);
    }
    int domain_len;
    temp2 = strstr(domain, "/");
    if (temp2 != 0) {
        memset(URL, 0, strlen(URL));
        domain_len = temp2 - domain;
        strncpy(URL, domain, domain_len);
        strncpy(path, temp2, strlen(domain) - domain_len);
        path[strlen(path)-1]= 0;
        memset(domain, 0, strlen(domain));
        strcpy(domain, URL);
    }
    if (domain[strlen(domain)-1] == '\n') domain[strlen(domain)-1] = 0;
    if (path[strlen(path)-1] == '\n') path[strlen(path)-1] = 0;
    if (strlen(path) == 0){
        path_ptr[0] = (char *)0x2F;
    }
    printf("프로토타입 %s, 도메인 %s, path %s입니다\n", proto, domain, path);

    return domain;
}


int main(void){
    char cmd_str[4];
    short cmd;
    int r;
    int opt = 1;
    char port[PORT_LEN];
    char path[IP_LEN];
    struct hostent *host_infos;
    struct in_addr **addr_list;
    struct in_addr addr;
    
    memset(path, 0, IP_LEN);

    fputs("원하는 기능을 선택하세요(1 : 서버대기  2 : 클라이언트) : ", stdout);
    fgets(cmd_str, 4, stdin); 
    cmd = cmd_str[0];
    
    if (cmd == '1'){
        
        int server_sock;
        struct sockaddr_in sockaddr;
        int client_sock;
        struct sockaddr_in client_addr;
        int socklen, recv_len = BUF_LEN;
        int is_first;
        char buf[BUF_LEN] = {0,};
        char ch;
        
        fputs("포트를 입력하세요 : ", stdout);
        fgets(port, sizeof(port) , stdin);
        port[strlen(port)-1] = '\0';
        printf("포트넘버 %s로 통신합니다\n", port);
        
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
        int recv_len;
        char message[RECV_MAX];
        int send_len;
        char buf_recv[RECV_MAX];
        char *ip = NULL;
        char *recvlen_ptr = NULL;
        char *header_ptr = NULL;
        char content_len[5];
        int header_len;
        int msg_len;
        struct http_req {
            char *path;
            char *domain;
        };
        struct http_req req;
        int header_ini, header_end;
        
        memset(buf_recv, 0, RECV_MAX);
        memset(message, 0, RECV_MAX);
        
        while (1){
            fputs("도메인을 입력하세요 : ", stdout);
            fgets(domain, sizeof(domain) , stdin);
            
            memcpy(domain, domain_to_ip(domain, path), URL_LEN);
            req.path = path;
            req.domain = domain;
            
            /* check out ip address */
            if((host_infos = gethostbyname(domain)) == NULL) {
                fprintf(stderr, "%s는 등록되지 않은 서버명입니다.\n", domain);
                return -1;
            }
            printf("ip주소타입은 %d ", host_infos->h_length);
            printf("Official name은 %s ", host_infos->h_name);
            ip = inet_ntoa(*(struct in_addr*)host_infos->h_addr);
            printf("IP주소는 %s입니다\n", ip);
            
            /* client socket */
            client_sock = socket(AF_INET, SOCK_STREAM, 0);
            opt = 1;
            setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            if (client_sock < 0) { printf("C socket failed\n");  return -1;}
            printf("C socket\n");
            
            /* client_sockaddr */
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(PORT);
            inet_pton(AF_INET, ip, &client_addr.sin_addr.s_addr);
            printf("C socket address\n");
            
            /* connect */
            r = connect(client_sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_in));
            if (r < 0) { printf("C connect failed errno : %d\n", errno); return -1; }
            printf("C connect\n");
            
            /*rearrange message */
            char *path_p, *domain_p;
            path_p = malloc(sizeof(req.path));
            domain_p = malloc(sizeof(req.domain));
            path_p = req.path;
            domain_p = req.domain;
            msg_len = sprintf(message, "GET %s HTTP/1.1\nHost: %s\nUser-Agent: curl/7.68.0\nAccept: */*\n\n", path_p, domain_p);
            printf("\n<<request msg>>\n%s", message);

            /* request/response */
            send_len = send(client_sock, message, msg_len, 0);
            if (send_len < 1) printf("C send failed\n");
            
            recv_len = recv(client_sock, buf_recv, RECV_MAX, 0);
            if (recv_len < 1) printf("C recv failed %d\n", errno);
            
            recvlen_ptr = strstr(buf_recv, "Content-Length") + 16;
            if (recvlen_ptr){
                memccpy(content_len, recvlen_ptr, '\r', RECV_MAX);
                content_len[strlen(content_len) - 1] = 0;
                printf("will receive %s bytes\n", content_len);
                
                header_ini = strstr(buf_recv, "HTTP");
                header_end = strstr(buf_recv, "\r\n\r\n");
                header_len = header_end - header_ini;
                
                printf("\n<<response msg>>\n");
                printf("%s", buf_recv);
                recv_len = recv(client_sock, buf_recv, (int)content_len + header_len - RECV_MAX, 0);
                printf("%s\n<<response ended>>\n", buf_recv);
                close(client_sock);
            } else{
                while(1){
                    printf("%s");
                    recv_len = recv(client_sock, buf_recv, RECV_MAX, 0);
                    if (recv_len > RECV_MAX) break; 
                }
                close(client_sock);
            }
        }
    } else printf("1, 2만 입력하세요");
        
    
	return 0;
}

