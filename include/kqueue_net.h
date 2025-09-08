#ifndef KQUEUE_NET_H
#define KQUEUE_NET_H

#include <sys/event.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

// 外部声明全局详细日志标志
extern bool g_verbose;

// 详细日志宏
#define VERBOSE_LOG(fmt, ...) do { \
    if (g_verbose) { \
        printf("[VERBOSE] " fmt "\n", ##__VA_ARGS__); \
        fflush(stdout); \
    } \
} while(0)

// 前向声明
struct KVStore;

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 1000

// 客户端连接结构
typedef struct {
    int fd;
    char buffer[BUFFER_SIZE];
    size_t buffer_len;
    bool request_complete;
} ClientConnection;

// 服务器结构
typedef struct {
    int server_fd;
    int kqueue_fd;
    int port;
    struct KVStore *kv_store;
    ClientConnection clients[MAX_CLIENTS];
    bool running;
} KVServer;

// 网络服务器接口
KVServer* server_create(int port);
void server_destroy(KVServer *server);
bool server_start(KVServer *server);
void server_stop(KVServer *server);
void server_run(KVServer *server);

// 内部函数
static bool setup_server_socket(KVServer *server);
static bool setup_kqueue(KVServer *server);
static void handle_new_connection(KVServer *server);
static void handle_client_data(KVServer *server, int client_fd);
static void handle_client_disconnect(KVServer *server, int client_fd);
static void process_http_request(KVServer *server, int client_fd, const char *request, size_t length);
static ClientConnection* find_client(KVServer *server, int fd);
static void init_client(ClientConnection *client, int fd);
static void cleanup_client(ClientConnection *client);

#endif // KQUEUE_NET_H

