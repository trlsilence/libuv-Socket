#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include "uv.h"  

uv_loop_t *loop;  

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {  
*buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);  
}  

void echo_write(uv_write_t *req, int status) {  
if (status < 0) {  
    fprintf(stderr, "Write error %s\n", uv_err_name(status));  
}  
char *base = (char*) req->data;  
free(base);  
free(req);  
}  

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t* buf) {  
if (nread < 0) {  
    if (nread == UV_EOF){  
        fprintf(stderr, "UV_EOF....\n");  
    }else{  
        fprintf(stderr, "Read error %s : %s\n", uv_err_name(nread), uv_strerror(nread));  
    }  
    uv_close((uv_handle_t*) client, NULL);  
    return;  
}  

printf("%s\n", buf->base);  

uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));    //alloc write watcher  
req->data = (void*) buf->base;  
uv_write(req, client, buf, 1, echo_write);  
}  

void on_new_connection(uv_stream_t *server, int status) {  

printf("enter on_new_connection...\n");  

if (status == -1) {  
    // error!  
    return;  
}  

uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));  
uv_tcp_init(loop, client);  
if (uv_accept(server, (uv_stream_t*) client) == 0) {  
    int r = uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);    
    if(r<0)  
    {  
        fprintf(stderr, "Read error %s\n", uv_err_name(r));  
        uv_close((uv_handle_t*)client, NULL);  
    }  
}  
else {  
    uv_close((uv_handle_t*) client, NULL);  
}  
}  

int main() {  
loop = uv_default_loop();  

uv_tcp_t server;  
uv_tcp_init(loop, &server);  

struct sockaddr_in bind_addr;  
uv_ip4_addr("0.0.0.0", 7000, &bind_addr);  
uv_tcp_bind(&server, (const sockaddr*)&bind_addr, 0);    //0--ipv4; 1--ipv6  
int r = uv_listen((uv_stream_t*) &server, 128, on_new_connection);  
if (r) {  
    fprintf(stderr, "Listen error %s\n", uv_err_name(r));  
    return 1;  
}  
return uv_run(loop, UV_RUN_DEFAULT);  
}  
