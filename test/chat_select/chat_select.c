#define BUF_MAX 6
#include <chat_select.h>
#include <stdio.h>

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
            fflush(stdout);
        }
    }
    close(sock);
    return 0;
}
