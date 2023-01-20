int *server_proc_entry(void *client_sock){
    int sock = (int)client_sock, r;
    char msg[THREAD_ID_LEN];
    int sock_from_outside = sock;
    
    sprintf(msg, "Your id is %d\n", sock);
    write(sock , msg , strlen(msg));
    printf("existing sock, %d, %d, %d\n", _server_management.socks[0], _server_management.socks[1], _server_management.socks[2]);
    printf("child thread generate sock %d\n", sock);
    
    while (1) {
        int is_first = 1, recv_len = 0;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(sock, buf, BUF_MAX - 1, 0);
            r = send(sock_from_outside == _server_management.socks[0] ? _server_management.socks[1] : _server_management.socks[0], buf, strlen(buf), 0);
            if (!strcmp(buf, "quit\n")) goto onclose;
            if (recv_len < 0) { printf("client close\n"); goto onclose; }
            ch = buf[recv_len - 1];
        }
    }
    printf("chatting with %d is disconnected by client..\n", sock);
    close(sock);
    return 0;
onclose:
    printf("chatting with %d is disconnected by client..\n", sock);
    close(sock);
    return -1;
}

int *client_proc_entry(void *client_sock){
    int sock = (int)client_sock, r;
    char msg[THREAD_ID_LEN];
    int sock_from_outside = sock;
        
    while (1) {
        int recv_len = 0;
        char buf[BUF_MAX], ch = 0;
        while (ch != '\n') {
            memset(buf, 0, BUF_MAX);
            recv_len = recv(sock, buf, BUF_MAX - 1, 0);
            if (!strcmp(buf, "quit\n")) goto onclose;
            if (recv_len < 0) { printf("client close\n"); goto onclose; }
            ch = buf[recv_len - 1];
            printf("%s", buf);
        }
    }
    printf("chatting with %d is disconnected by client..\n", sock);
    close(sock);
    return 0;
onclose:
    printf("chatting with %d is disconnected by client..\n", sock);
    close(sock);
    return -1;
}