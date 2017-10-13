#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <stdlib.h>  
#include "uv.h"  
#include <cstring>  
  
uv_loop_t *loop;  
uv_tcp_t *client;  
  
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);  
  
void do_read();  
  
void do_write();  
  
void echo_write(uv_write_t *req, int status);  
  
void echo_read(uv_stream_t *client_, ssize_t nread, const uv_buf_t* buf) ;  
  
void on_connection(uv_connect_t* req, int status);  
  
void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {  
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);  
}  
  
void do_read()  
{  
    int r = uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);    
    if(r<0)  
    {  
        fprintf(stderr, "Read error %s\n", uv_err_name(r));  
        uv_close((uv_handle_t*)client, NULL);  
    }  
}  
  
void do_write()  
{  
    uv_write_t *req_write = (uv_write_t *) malloc(sizeof(uv_write_t));    
  
    char *line = (char*)malloc(1024*sizeof(char));  
    memset(line, 0, 1024);  
    int len = 0;  
    while (gets(line))  
    {  
      for(; len<1024; len++)  
      {  
          if(line[len] == 0)  
              break;  
      }  
      break;  
    }  
  
    uv_buf_t *buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));  
    uv_buf_init(line, len+1);  
    buf->base = line;  
    buf->len = len+1;  
    req_write->data = (void*) buf->base;  
    int r = uv_write(req_write, (uv_stream_t*)client, buf, 1, echo_write);  
    if(r<0)  
    {  
        fprintf(stderr, "write error %s\n", uv_err_name(r));  
        uv_close((uv_handle_t*)client, NULL);  
    }  
}  
  
void echo_write(uv_write_t *req, int status)  
{  
    if (status < 0) {  
        fprintf(stderr, "Write error %s\n", uv_err_name(status));  
    }  
    char *base = (char*) req->data;  
    free(base);  
    free(req);  
  
    do_write();  
}  
  
void echo_read(uv_stream_t *client_, ssize_t nread, const uv_buf_t* buf)  
{  
    if (nread < 0) {  
        if (nread == UV_EOF){  
            fprintf(stderr, "UV_EOF....\n");  
        }else{  
            fprintf(stderr, "Read error %s : %s\n", uv_err_name(nread), uv_strerror(nread));  
        }  
        uv_close((uv_handle_t*) client_, NULL);  
        return;  
    }else{  
        printf("%s\n", buf->base);  
    }  
  
    do_read();  
}  
  
void on_connection(uv_connect_t* req, int status) {  
    if (status < 0) {  
        // error!  
        fprintf(stderr, "connect error %s : %s\n", uv_err_name(status), uv_strerror(status));  
        return;  
    }  
  
    free(req);  
  
    do_read();  
    do_write();  
  
}  
  
int main() {  
    loop = uv_default_loop();  
  
    client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));  
    uv_tcp_init(loop, client);  
  
    struct sockaddr_in bind_addr;  
    uv_ip4_addr("192.168.155.4", 7000, &bind_addr);  
  
    uv_connect_t *req = (uv_connect_t *)malloc(sizeof(uv_connect_t));  
    int r = uv_tcp_connect(req, client, (const sockaddr*)&bind_addr, on_connection);  
    if (r) {  
        fprintf(stderr, "connect error %s\n", uv_err_name(r));  
        return 1;  
    }  
    return uv_run(loop, UV_RUN_DEFAULT);  
} 
