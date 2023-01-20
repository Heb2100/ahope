#define INFO_MSG_LEN 32
#define PORT_LEN 8
#define IP_LEN 16
#define MSG_MAX 6
#define BUF_LEN 6
#define URL_LEN 128
#define CLIENT_NUM_MAX 3
#define BUF_MAX 6

struct server_management {
    int socks[CLIENT_NUM_MAX];
} _server_management;

int *client_proc_entry(void *client_sock);

