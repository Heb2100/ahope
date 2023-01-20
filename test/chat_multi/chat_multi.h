#include <pthread.h>
#define IP_LEN 16
#define BUF_MAX 6
#define INFO_MSG_LEN 32
#define THREAD_ID_LEN 8
#define CLIENT_NUM_MAX 5
#define FILE_MSG_LEN 32


struct client_context {
    struct server_management *management;
    int sock;
};

struct server_thread_input {
    int c_num;
    int c_sock;
    int *file_ptr;
};

struct server_management {
    int socks[CLIENT_NUM_MAX];
} _server_management;

void *server_proc_entry(struct server_thread_input *s_thread);
int *client_proc_entry(void *client_sock);



struct http_methods {
    (int *)(*client_proc_entry)(void *) client_entry;
};