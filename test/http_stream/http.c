#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "http.h"

void parse_url(char *src, struct http_req *req){
    char *domain = (char *)malloc(URL_LEN);
    char *path = (char *)malloc(URL_LEN);
    char URL[URL_LEN], proto[URL_LEN], *proto_ptr, *proto_domain;
    int proto_len, domain_len;
    char *p = src;
    
    proto_ptr = strstr(p, "://");
    if (proto_ptr) {
        proto_len = (int)(proto_ptr - p);
        memcpy(proto, p, proto_len);
        /* cut off protocol and :// */
        p += strlen(proto) + 3;
    }
    proto_domain = strstr(p, "/");
    if (proto_domain) {
        domain_len = (int)(proto_domain - p);
        memcpy(domain, p, domain_len);
        memcpy(path, p + domain_len, strlen(p) - domain_len);
    } else strcpy(domain, p);
    
    if (domain[strlen(domain) - 1] == '\n') domain[strlen(domain) - 1] = 0;
    if (path[strlen(path) - 1] == '\n') path[strlen(path) - 1] = 0;
    /* add / if path is empty */
    if (strlen(path) == 0) path[0] = 0x2F;
    req->dmn = domain;
    req->path = path;
    printf("프로토타입 %s, 도메인 %s, path %s입니다\n", proto, req->dmn, req->path);
}

struct http_res* get_response_from_server(int server_sock){
    struct http_res *res = (struct http_res *)malloc(sizeof(struct http_res));
    memset(res, 0, sizeof(struct http_res));
    {res->header_end = 1; res->is_method = 1; res->is_header = 0; res->is_chunked = 0;}
//    memset(res->content_len_str, 0, sizeof(res->content_len_str));
    memset(res->body, 0, sizeof(res->body));
        
    /* get header by 32 buf */
    while (res->header_end){
        memset(res->buf, 0, BUF_MAX_SIZE);
        res->buffer_len = recv(server_sock, res->buf, ONE_BLOCK_FROM_SERVER, 0);
        
        res->header_end_ptr = strstr(res->buf, "\r\n\r\n");
        if (res->header_end_ptr) res->header_end = 0;
        
        res->method_ptr = res->buf;
        if (res->is_method) {
            res->method_end = strchr(res->buf, '\n');
            res->method_len = (int)(res->method_end - res->method_ptr + 1);
            sscanf(res->buf, "%s %s %s\r", res->protocol, res->status_code, res->req_status);
            printf("%s %s %s\n", res->protocol, res->status_code, res->req_status);
            res->is_method = 0; res->is_header = 1;
            memcpy(res->header_left, res->buf + res->method_len, res->buffer_len - res->method_len);
            memset(res->buf, 0, strlen(res->buf));
        }
        
        if (res->is_header) {
            sprintf(res->buf_tmp, "%s%s", res->header_left, res->buf);
            memcpy(res->buf, res->buf_tmp, BUF_MAX_SIZE);
            memset(res->buf_tmp, 0, BUF_MAX_SIZE);
            
            res->header = res->buf;
            while (1){
                memset(res->key, 0, ONE_BLOCK_FROM_SERVER);
                memset(res->value, 0, ONE_BLOCK_FROM_SERVER);
                
                memccpy(res->key, res->header, ': ', ONE_BLOCK_FROM_SERVER);
                res->key_len = strlen(res->key);
                if (!res->key_len) break;
                res->header += res->key_len;
                
                res->ptr_value = memccpy(res->value, res->header, '\r\n', ONE_BLOCK_FROM_SERVER);
                if (res->ptr_value == NULL) {
                    res->header -= res->key_len; 
                    break;
                }
                res->header += strlen(res->value) ;
                
                printf("%s%s", res->key, res->value);
                
                if (!strncmp(res->key, "Content-Length: ", 16)) {
                    strcpy(res->content_len_str, res->value);
                    /* remove \r\n */
                    res->content_len_str[strlen(res->content_len_str) - 2] = 0;
                    res->is_content_len = 1;
                }
                if (!strncmp(res->key, "Transfer-Encoding: ", 19)) {
                    if (!strncmp(res->value, "chunked", 7)) res->is_chunked = 1;
                }
            }
            memset(res->header_left, 0, BUF_MAX_SIZE);
            strcpy(res->header_left, res->header);
        }
    }
    return res;
}
