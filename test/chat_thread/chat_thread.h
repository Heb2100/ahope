#define IP_LEN 16
#define BUF_MAX 6
#define INFO_MSG_LEN 32
#define THREAD_ID_LEN 8
#define CLIENT_NUM_MAX 5

struct client_context {
    struct server_management *management;
    int sock;
};

struct server_management {
    int socks[CLIENT_NUM_MAX];
    int connected_socks[CLIENT_NUM_MAX];
} _server_management;

int *server_proc_entry(void *client_sock);
int *client_proc_entry(void *client_sock);