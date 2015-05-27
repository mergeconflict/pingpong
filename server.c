#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <sys/socket.h>

void on_close(uv_handle_t* handle) {
    free(handle);
}

void alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void on_send_pong(uv_udp_send_t* req, int status) {
    free(req);
    if (status) {
        fprintf(stderr, "uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}
void send_pong(uv_udp_t* handle, const uv_buf_t* buf, const struct sockaddr* addr) {
    uv_udp_send_t* req = malloc(sizeof(uv_udp_send_t));
    int r = uv_udp_send(req, handle, buf, 1, addr, on_send_pong);
    if (r) {
        fprintf(stderr, "uv_udp_send error: %s\n", uv_strerror(r));
    }
}

void on_recv_ping(uv_udp_t* handle, ssize_t nread, const uv_buf_t* ping_buf, const struct sockaddr* addr, unsigned flags) {
    if (nread) {
        uv_buf_t pong_buf = uv_buf_init(ping_buf->base, sizeof(uint64_t));
        send_pong(handle, &pong_buf, addr);
    }
    free(ping_buf->base);
}
void recv_ping(uv_udp_t* handle) {
    int r = uv_udp_recv_start(handle, alloc_buf, on_recv_ping);
    if (r) {
        fprintf(stderr, "uv_udp_recv_start error: %s\n", uv_strerror(r));
    }
}

int main(int argc, char** argv) {
    int r;

    uv_loop_t* loop = uv_default_loop();

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 31337, &addr);

    // closed below, freed in on_close()
    uv_udp_t* handle = (uv_udp_t*) malloc(sizeof(uv_udp_t));
    uv_udp_init(loop, handle);

    r = uv_udp_bind(handle, (const struct sockaddr*) &addr, 0);
    if (r) {
        fprintf(stderr, "uv_udp_bind error: %s\n", uv_strerror(r));
        return 1;
    }

    recv_ping(handle);

    r = uv_run(loop, UV_RUN_DEFAULT);

    uv_close((uv_handle_t*) handle, on_close);

    return r;
}
