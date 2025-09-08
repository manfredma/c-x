#include "kqueue_net.h"
#include "kv_store.h"
#include "http_parser.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

static bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

KVServer* server_create(int port) {
    KVServer *server = calloc(1, sizeof(KVServer));
    if (!server) return NULL;
    server->port = port;
    server->server_fd = -1;
    server->kqueue_fd = -1;
    server->running = false;
    server->kv_store = kv_store_create(0);
    if (!server->kv_store) {
        free(server);
        return NULL;
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].fd = -1;
    }
    return server;
}

void server_destroy(KVServer *server) {
    if (!server) return;
    server_stop(server);
    if (server->kv_store) {
        kv_store_destroy(server->kv_store);
    }
    free(server);
}

static bool setup_server_socket(KVServer *server) {
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) return false;
    int opt = 1;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server->server_fd);
        return false;
    }
    if (!set_nonblocking(server->server_fd)) {
        close(server->server_fd);
        return false;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(server->port);
    if (bind(server->server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(server->server_fd);
        return false;
    }
    if (listen(server->server_fd, SOMAXCONN) == -1) {
        close(server->server_fd);
        return false;
    }
    return true;
}

static bool setup_kqueue(KVServer *server) {
    server->kqueue_fd = kqueue();
    if (server->kqueue_fd == -1) return false;
    struct kevent event;
    EV_SET(&event, server->server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(server->kqueue_fd, &event, 1, NULL, 0, NULL) == -1) {
        close(server->kqueue_fd);
        return false;
    }
    return true;
}

bool server_start(KVServer *server) {
    if (!server || server->running) return false;
    if (!setup_server_socket(server)) return false;
    if (!setup_kqueue(server)) {
        close(server->server_fd);
        return false;
    }
    server->running = true;
    printf("KV 存储服务器启动成功，监听端口 %d\n", server->port);
    return true;
}

void server_stop(KVServer *server) {
    if (!server || !server->running) return;
    server->running = false;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd != -1) {
            close(server->clients[i].fd);
            server->clients[i].fd = -1;
        }
    }
    if (server->server_fd != -1) {
        close(server->server_fd);
        server->server_fd = -1;
    }
    if (server->kqueue_fd != -1) {
        close(server->kqueue_fd);
        server->kqueue_fd = -1;
    }
    printf("KV 存储服务器已停止\n");
}

static ClientConnection* find_client(KVServer *server, int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == fd) {
            return &server->clients[i];
        }
    }
    return NULL;
}

static void init_client(ClientConnection *client, int fd) {
    client->fd = fd;
    client->buffer_len = 0;
    client->request_complete = false;
    memset(client->buffer, 0, BUFFER_SIZE);
}

static void cleanup_client(ClientConnection *client) {
    if (client->fd != -1) {
        VERBOSE_LOG("清理客户端连接，fd: %d", client->fd);
        close(client->fd);
        client->fd = -1;
    }
    client->buffer_len = 0;
    client->request_complete = false;
    VERBOSE_LOG("客户端连接清理完成");
}

static void handle_new_connection(KVServer *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    VERBOSE_LOG("处理新连接请求");
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        VERBOSE_LOG("accept 失败: %s", strerror(errno));
        return;
    }
    VERBOSE_LOG("接受新连接，fd: %d，地址: %s:%d",
                client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    if (!set_nonblocking(client_fd)) {
        VERBOSE_LOG("设置非阻塞失败，关闭连接 fd: %d", client_fd);
        close(client_fd);
        return;
    }
    VERBOSE_LOG("设置客户端 fd %d 为非阻塞模式", client_fd);
    ClientConnection *client = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == -1) {
            client = &server->clients[i];
            break;
        }
    }
    if (!client) {
        VERBOSE_LOG("客户端连接数已满，关闭 fd %d", client_fd);
        close(client_fd);
        return;
    }
    init_client(client, client_fd);
    struct kevent event;
    EV_SET(&event, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(server->kqueue_fd, &event, 1, NULL, 0, NULL) == -1) {
        VERBOSE_LOG("添加客户端到 kqueue 失败: %s", strerror(errno));
        cleanup_client(client);
        return;
    }
    VERBOSE_LOG("客户端 fd %d 已添加到 kqueue", client_fd);
    printf("新客户端连接: %s:%d (fd=%d)\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           client_fd);
    VERBOSE_LOG("客户端连接初始化完成");
}

// 处理静态文件请求
static void serve_static_file(int client_fd, const char *path) {
    // 如果请求 /web 路径，返回测试页面
    if (strcmp(path, "/web") == 0 || strcmp(path, "/web/") == 0 || strcmp(path, "/web/index.html") == 0) {
        FILE *file = fopen("web/index.html", "r");
        if (file) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            // 检查文件大小是否合理
            if (file_size <= 0 || file_size > 1024 * 1024) { // 限制文件大小为1MB
                fclose(file);
                goto send_404;
            }

            char *file_content = malloc(file_size + 1);
            if (!file_content) {
                fclose(file);
                goto send_404;
            }

            size_t bytes_read = fread(file_content, 1, file_size, file);
            fclose(file);

            if (bytes_read != (size_t)file_size) {
                free(file_content);
                goto send_404;
            }

            file_content[file_size] = '\0';

            // 发送 HTML 响应 - 分别发送头部和内容
            char header[300];
            int header_len = snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n", file_size);

            if (header_len > 0 && header_len < (int)sizeof(header)) {
                // 先发送头部
                send(client_fd, header, header_len, 0);
                // 再发送文件内容
                send(client_fd, file_content, file_size, 0);
            }

            free(file_content);
            return;
        }
    }

send_404:
    // 文件未找到，返回 404
    {
        const char *not_found = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "Content-Length: 9\r\n"
                               "Connection: close\r\n"
                               "\r\nNot Found";
        send(client_fd, not_found, strlen(not_found), 0);
    }
}

static void process_http_request(KVServer *server, int client_fd, const char *request, size_t length) {
    VERBOSE_LOG("=== 处理 HTTP 请求 ===");
    VERBOSE_LOG("客户端 fd: %d", client_fd);
    VERBOSE_LOG("请求长度: %zu", length);
    VERBOSE_LOG("请求内容: %.200s%s", request, length > 200 ? "..." : "");

    HttpRequest *http_req = http_parse_request(request, length);
    if (!http_req) {
        VERBOSE_LOG("HTTP 请求解析失败");
        HttpResponse *response = http_create_response(400, "Bad Request");
        if (response) {
            size_t response_len;
            char *response_str = http_build_response(response, &response_len);
            if (response_str) {
                send(client_fd, response_str, response_len, 0);
                free(response_str);
            }
            http_free_response(response);
        }
        return;
    }

    VERBOSE_LOG("HTTP 请求解析成功:");
    VERBOSE_LOG("  方法: %d", http_req->method);
    VERBOSE_LOG("  路径: %s", http_req->path ? http_req->path : "NULL");
    VERBOSE_LOG("  请求体长度: %zu", http_req->body_length);
    if (http_req->body && http_req->body_length > 0) {
        VERBOSE_LOG("  请求体: %.100s%s", http_req->body, http_req->body_length > 100 ? "..." : "");
    }

    // 严格按照 HTTP 协议处理请求，只允许特定的合法操作

    // 1. 处理根路径重定向 (仅 GET 方法)
    if (http_req->method == HTTP_GET && strcmp(http_req->path, "/") == 0) {
        VERBOSE_LOG("处理根路径重定向到 /web/");
        const char *redirect_response = "HTTP/1.1 302 Found\r\n"
                                      "Location: /web/\r\n"
                                      "Content-Type: text/html\r\n"
                                      "Content-Length: 47\r\n"
                                      "Connection: close\r\n"
                                      "\r\n"
                                      "<html><body>Redirecting to <a href=\"/web/\">/web/</a></body></html>";
        send(client_fd, redirect_response, strlen(redirect_response), 0);
        http_free_request(http_req);
        VERBOSE_LOG("根路径重定向处理完成");
        return;
    }

    // 2. 处理静态文件请求 (仅 GET 方法，仅 /web 路径)
    if (http_req->method == HTTP_GET && strncmp(http_req->path, "/web", 4) == 0) {
        VERBOSE_LOG("处理静态文件请求: %s", http_req->path);
        serve_static_file(client_fd, http_req->path);
        http_free_request(http_req);
        VERBOSE_LOG("静态文件请求处理完成");
        return;
    }

    // 2.5. 处理健康检查和连接测试请求 (仅 GET 方法)
    if (http_req->method == HTTP_GET &&
        (strcmp(http_req->path, "/test_connection") == 0 || strcmp(http_req->path, "/health") == 0)) {
        VERBOSE_LOG("处理健康检查/连接测试请求: %s", http_req->path);

        // 构建 JSON 响应
        char json_response[200];
        int json_len = snprintf(json_response, sizeof(json_response),
            "{\"status\":\"ok\",\"service\":\"KV Storage Server\",\"timestamp\":%ld}",
            time(NULL));

        char *health_response = malloc(json_len + 150);
        if (health_response) {
            int response_len = snprintf(health_response, json_len + 150,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n%s", json_len, json_response);

            send(client_fd, health_response, response_len, 0);
            free(health_response);
        }

        http_free_request(http_req);
        VERBOSE_LOG("健康检查/连接测试请求处理完成");
        return;
    }

    // 3. 处理 API 请求 (GET, POST, DELETE 方法，仅 /api/ 路径)
    if (strncmp(http_req->path, "/api/", 5) == 0) {
        VERBOSE_LOG("处理 API 请求: %s", http_req->path);
        // 检查 HTTP 方法是否合法
        if (http_req->method != HTTP_GET && http_req->method != HTTP_POST && http_req->method != HTTP_DELETE) {
            VERBOSE_LOG("API 请求方法不允许: %d", http_req->method);
            HttpResponse *response = http_create_response(405, "Method Not Allowed");
            if (response) {
                size_t response_len;
                char *response_str = http_build_response(response, &response_len);
                if (response_str) {
                    send(client_fd, response_str, response_len, 0);
                    free(response_str);
                }
                http_free_response(response);
            }
            http_free_request(http_req);
            return;
        }

        // 处理 API 操作
        const char *key = http_req->path + 5; // 跳过 "/api/" 前缀
        VERBOSE_LOG("提取的键名: '%s'", key);

        if (strlen(key) == 0) {
            VERBOSE_LOG("键名为空，返回 400 错误");
            HttpResponse *response = http_create_response(400, "Bad Request - Key cannot be empty");
            if (response) {
                size_t response_len;
                char *response_str = http_build_response(response, &response_len);
                if (response_str) {
                    send(client_fd, response_str, response_len, 0);
                    free(response_str);
                }
                http_free_response(response);
            }
            http_free_request(http_req);
            return;
        }

        // 执行 KV 操作
        HttpResponse *response = NULL;
        VERBOSE_LOG("执行 KV 操作，方法: %d，键: '%s'", http_req->method, key);
        switch (http_req->method) {
            case HTTP_GET: {
                VERBOSE_LOG("执行 GET 操作");
                char *value = kv_get(server->kv_store, key);
                if (value) {
                    VERBOSE_LOG("GET 成功，值: '%.50s%s'", value, strlen(value) > 50 ? "..." : "");
                    response = http_create_response(200, value);
                    free(value);
                } else {
                    VERBOSE_LOG("GET 失败，键不存在");
                    response = http_create_response(404, "Key not found");
                }
                break;
            }
            case HTTP_POST: {
                VERBOSE_LOG("执行 POST 操作");
                if (http_req->body && http_req->body_length > 0) {
                    VERBOSE_LOG("POST 请求体: '%.50s%s'", http_req->body, http_req->body_length > 50 ? "..." : "");
                    if (kv_set(server->kv_store, key, http_req->body)) {
                        VERBOSE_LOG("POST 成功");
                        response = http_create_response(201, "Created");
                    } else {
                        VERBOSE_LOG("POST 失败，内部错误");
                        response = http_create_response(500, "Internal Server Error");
                    }
                } else {
                    VERBOSE_LOG("POST 失败，缺少请求体");
                    response = http_create_response(400, "Request body required");
                }
                break;
            }
            case HTTP_DELETE: {
                VERBOSE_LOG("执行 DELETE 操作");
                if (kv_delete(server->kv_store, key)) {
                    VERBOSE_LOG("DELETE 成功");
                    response = http_create_response(204, "");
                } else {
                    VERBOSE_LOG("DELETE 失败，键不存在");
                    response = http_create_response(404, "Key not found");
                }
                break;
            }
            default:
                VERBOSE_LOG("不支持的方法: %d", http_req->method);
                response = http_create_response(405, "Method Not Allowed");
                break;
        }

        if (response) {
            size_t response_len;
            char *response_str = http_build_response(response, &response_len);
            if (response_str) {
                VERBOSE_LOG("发送响应，状态码: %d，长度: %zu", response->status_code, response_len);
                send(client_fd, response_str, response_len, 0);
                free(response_str);
            }
            http_free_response(response);
        }
        http_free_request(http_req);
        VERBOSE_LOG("API 请求处理完成");
        return;
    }

    // 4. 所有其他请求一律返回 404
    VERBOSE_LOG("未匹配任何路径，返回 404: %s", http_req->path);
    HttpResponse *response = http_create_response(404, "Not Found");
    if (response) {
        size_t response_len;
        char *response_str = http_build_response(response, &response_len);
        if (response_str) {
            send(client_fd, response_str, response_len, 0);
            free(response_str);
        }
        http_free_response(response);
    }
    http_free_request(http_req);
    VERBOSE_LOG("404 响应发送完成");
    return;

}

static void handle_client_data(KVServer *server, int client_fd) {
    VERBOSE_LOG("处理客户端数据，fd: %d", client_fd);
    ClientConnection *client = find_client(server, client_fd);
    if (!client) {
        VERBOSE_LOG("未找到客户端连接，fd: %d", client_fd);
        return;
    }

    // 安全检查：确保缓冲区有足够空间
    if (client->buffer_len >= BUFFER_SIZE - 1) {
        VERBOSE_LOG("客户端缓冲区已满，fd: %d", client_fd);
        cleanup_client(client);
        return;
    }

    ssize_t bytes_read = recv(client_fd,
                             client->buffer + client->buffer_len,
                             BUFFER_SIZE - client->buffer_len - 1,
                             0);

    VERBOSE_LOG("从客户端 fd %d 读取 %zd 字节", client_fd, bytes_read);

    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            VERBOSE_LOG("客户端 fd %d 关闭连接", client_fd);
        } else {
            VERBOSE_LOG("从客户端 fd %d 读取数据失败: %s", client_fd, strerror(errno));
        }
        cleanup_client(client);
        return;
    }

    client->buffer_len += bytes_read;
    client->buffer[client->buffer_len] = '\0';

    VERBOSE_LOG("客户端 fd %d 缓冲区总长度: %zu", client_fd, client->buffer_len);

    char *request_end = strstr(client->buffer, "\r\n\r\n");
    if (!request_end) {
        request_end = strstr(client->buffer, "\n\n");
    }

    if (request_end) {
        VERBOSE_LOG("检测到完整的 HTTP 请求，fd: %d", client_fd);
        process_http_request(server, client_fd, client->buffer, client->buffer_len);
        cleanup_client(client);
    } else if (client->buffer_len >= BUFFER_SIZE - 1) {
        VERBOSE_LOG("请求过大，拒绝处理，fd: %d", client_fd);
        HttpResponse *response = http_create_response(400, "Request too large");
        if (response) {
            size_t response_len;
            char *response_str = http_build_response(response, &response_len);
            if (response_str) {
                send(client_fd, response_str, response_len, 0);
                free(response_str);
            }
            http_free_response(response);
        }
        cleanup_client(client);
    } else {
        VERBOSE_LOG("等待更多数据，fd: %d，当前长度: %zu", client_fd, client->buffer_len);
    }
}

static void handle_client_disconnect(KVServer *server, int client_fd) {
    ClientConnection *client = find_client(server, client_fd);
    if (client) {
        cleanup_client(client);
    }
}

void server_run(KVServer *server) {
    if (!server || !server->running) return;
    struct kevent events[MAX_EVENTS];
    printf("服务器开始运行，按 Ctrl+C 停止...\n");

    while (server->running) {
        int event_count = kevent(server->kqueue_fd, NULL, 0, events, MAX_EVENTS, NULL);
        if (event_count == -1) {
            if (errno == EINTR) continue;
            perror("kevent");
            break;
        }

        for (int i = 0; i < event_count; i++) {
            struct kevent *event = &events[i];
            if (event->ident == (uintptr_t)server->server_fd) {
                handle_new_connection(server);
            } else {
                if (event->flags & EV_EOF) {
                    handle_client_disconnect(server, event->ident);
                } else if (event->filter == EVFILT_READ) {
                    handle_client_data(server, event->ident);
                }
            }
        }
    }
}
