#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

struct sockaddr_in server_addr;

void on_close(uv_handle_t* handle) {
    free(handle);
}

void alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void send_ping(uv_udp_t*);

void on_recv_pong(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    if (nread) {
        uint64_t resp = *((uint64_t*) buf->base);
        printf("%llu\n", uv_hrtime() - resp);
    } else {
        send_ping(handle);
    }
    free(buf->base);
}
void recv_pong(uv_udp_t* handle) {
    int r = uv_udp_recv_start(handle, alloc_buf, on_recv_pong);
    if (r) {
        fprintf(stderr, "uv_udp_recv_start error: %s\n", uv_strerror(r));
    }
}

void on_send_ping(uv_udp_send_t* req, int status) {
    free(req);
    if (status) {
        fprintf(stderr, "uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}
void send_ping(uv_udp_t* handle) {
    uv_udp_send_t* req = malloc(sizeof(uv_udp_send_t));

    uint64_t tm = uv_hrtime();
    uv_buf_t buf = uv_buf_init((char*) &tm, sizeof(tm));

    int r = uv_udp_send(req, handle, &buf, 1, (const struct sockaddr*) &server_addr, on_send_ping);
    if (r) {
        fprintf(stderr, "uv_udp_send error: %s\n", uv_strerror(r));
    }
}

int main(int argc, char** argv) {
    int r;

    uv_loop_t* loop = uv_default_loop();

    uv_ip4_addr("75.119.221.89", 31337, &server_addr);

    // closed below, freed in on_close()
    uv_udp_t* handle = (uv_udp_t*) malloc(sizeof(uv_udp_t));
    uv_udp_init(loop, handle);

    recv_pong(handle);
    send_ping(handle);

    r = uv_run(loop, UV_RUN_DEFAULT);

    uv_close((uv_handle_t*) handle, on_close);

    return r;
}
