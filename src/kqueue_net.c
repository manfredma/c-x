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
        close(client->fd);
        client->fd = -1;
    }
    client->buffer_len = 0;
    client->request_complete = false;
}

static void handle_new_connection(KVServer *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server->server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) return;
    if (!set_nonblocking(client_fd)) {
        close(client_fd);
        return;
    }
    ClientConnection *client = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].fd == -1) {
            client = &server->clients[i];
            break;
        }
    }
    if (!client) {
        close(client_fd);
        return;
    }
    init_client(client, client_fd);
    struct kevent event;
    EV_SET(&event, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(server->kqueue_fd, &event, 1, NULL, 0, NULL) == -1) {
        cleanup_client(client);
        return;
    }
    printf("新客户端连接: %s:%d (fd=%d)\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           client_fd);
}

static void process_http_request(KVServer *server, int client_fd, const char *request, size_t length) {
    HttpRequest *http_req = http_parse_request(request, length);
    if (!http_req) {
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

    HttpResponse *response = NULL;
    const char *key = http_req->path;
    if (key[0] == '/') key++;

    if (strlen(key) == 0) {
        response = http_create_response(400, "Key cannot be empty");
    } else {
        switch (http_req->method) {
            case HTTP_GET: {
                char *value = kv_get(server->kv_store, key);
                if (value) {
                    response = http_create_response(200, value);
                    free(value);
                } else {
                    response = http_create_response(404, "Key not found");
                }
                break;
            }
            case HTTP_POST: {
                if (http_req->body && http_req->body_length > 0) {
                    if (kv_set(server->kv_store, key, http_req->body)) {
                        response = http_create_response(201, "Created");
                    } else {
                        response = http_create_response(500, "Internal Server Error");
                    }
                } else {
                    response = http_create_response(400, "Request body required");
                }
                break;
            }
            case HTTP_DELETE: {
                if (kv_delete(server->kv_store, key)) {
                    response = http_create_response(204, "");
                } else {
                    response = http_create_response(404, "Key not found");
                }
                break;
            }
            default:
                response = http_create_response(405, "Method Not Allowed");
                break;
        }
    }

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
}

static void handle_client_data(KVServer *server, int client_fd) {
    ClientConnection *client = find_client(server, client_fd);
    if (!client) return;

    ssize_t bytes_read = recv(client_fd,
                             client->buffer + client->buffer_len,
                             BUFFER_SIZE - client->buffer_len - 1,
                             0);

    if (bytes_read <= 0) {
        cleanup_client(client);
        return;
    }

    client->buffer_len += bytes_read;
    client->buffer[client->buffer_len] = '\0';

    char *request_end = strstr(client->buffer, "\r\n\r\n");
    if (!request_end) {
        request_end = strstr(client->buffer, "\n\n");
    }

    if (request_end) {
        process_http_request(server, client_fd, client->buffer, client->buffer_len);
        cleanup_client(client);
    } else if (client->buffer_len >= BUFFER_SIZE - 1) {
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
