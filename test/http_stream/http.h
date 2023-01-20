#define ARG_LEN 32
#define URL_LEN 64
#define BUF_MAX_SIZE 64
#define ONE_BLOCK_FROM_SERVER 32
#define BODY_MAX 20000

struct http_req {
    char *path, *dmn;
};

struct http_res {
    int header_end, is_method, is_header, is_chunked, is_content_len;
    char req_status[ARG_LEN], status_code[ARG_LEN], protocol[ARG_LEN];
    int buffer_len, method_len, key_len, value_len;
    char *header_end_ptr, *method_end, *method_ptr;
    char tmp[BUF_MAX_SIZE], header_left[BUF_MAX_SIZE], key[ONE_BLOCK_FROM_SERVER], value[ONE_BLOCK_FROM_SERVER], buf_tmp[BUF_MAX_SIZE];
    char *ptr_key, *ptr_value, *chunk_end, *p_body_end, *chunk_start;
    int sscanf_out, body_len, tmp_len, excess_tmp_len;
    char *header, body[BODY_MAX];
    char content_len_str[ARG_LEN], buf[BUF_MAX_SIZE], chunk_hex[8];
    int chunk_len, recv_body_len;
};

struct http_res*  get_response_from_server(int server_sock);
void parse_url(char *src, struct http_req *req);

