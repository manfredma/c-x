#include "http_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// HTTP 方法转字符串
const char* http_method_to_string(HttpMethod method) {
    switch (method) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

// 字符串转 HTTP 方法
HttpMethod http_string_to_method(const char *method_str) {
    if (strcmp(method_str, "GET") == 0) return HTTP_GET;
    if (strcmp(method_str, "POST") == 0) return HTTP_POST;
    if (strcmp(method_str, "DELETE") == 0) return HTTP_DELETE;
    return HTTP_UNKNOWN;
}

// 获取状态码对应的状态文本
const char* http_status_text(int status_code) {
    switch (status_code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

// 解析 HTTP 请求
HttpRequest* http_parse_request(const char *raw_request, size_t length) {
    if (!raw_request || length == 0) {
        return NULL;
    }

    HttpRequest *request = calloc(1, sizeof(HttpRequest));
    if (!request) {
        return NULL;
    }

    // 复制原始请求以便解析
    char *request_copy = malloc(length + 1);
    if (!request_copy) {
        free(request);
        return NULL;
    }
    memcpy(request_copy, raw_request, length);
    request_copy[length] = '\0';

    // 查找请求行结束位置
    char *line_end = strstr(request_copy, "\r\n");
    if (!line_end) {
        line_end = strstr(request_copy, "\n");
        if (!line_end) {
            free(request_copy);
            free(request);
            return NULL;
        }
    }

    // 解析请求行：METHOD PATH HTTP/1.1
    *line_end = '\0';
    char *method_str = strtok(request_copy, " ");
    char *path_str = strtok(NULL, " ");
    char *version_str = strtok(NULL, " ");

    if (!method_str || !path_str || !version_str) {
        free(request_copy);
        free(request);
        return NULL;
    }

    // 设置方法
    request->method = http_string_to_method(method_str);

    // 设置路径
    request->path = strdup(path_str);
    if (!request->path) {
        free(request_copy);
        free(request);
        return NULL;
    }

    // 查找请求头和请求体的分界线
    char *headers_start = line_end + (line_end[0] == '\r' ? 2 : 1);
    char *body_separator = strstr(raw_request, "\r\n\r\n");
    if (!body_separator) {
        body_separator = strstr(raw_request, "\n\n");
    }

    if (body_separator) {
        // 有请求体
        size_t headers_len = body_separator - headers_start;
        if (headers_len > 0) {
            request->headers = malloc(headers_len + 1);
            if (request->headers) {
                memcpy(request->headers, headers_start, headers_len);
                request->headers[headers_len] = '\0';
            }
        }

        // 解析请求体
        char *body_start = body_separator + (strstr(body_separator, "\r\n\r\n") ? 4 : 2);
        size_t body_len = length - (body_start - raw_request);

        if (body_len > 0) {
            request->body = malloc(body_len + 1);
            if (request->body) {
                memcpy(request->body, body_start, body_len);
                request->body[body_len] = '\0';
                request->body_length = body_len;
            }
        }
    } else {
        // 没有请求体，只有请求头
        size_t headers_len = length - (headers_start - raw_request);
        if (headers_len > 0) {
            request->headers = malloc(headers_len + 1);
            if (request->headers) {
                memcpy(request->headers, headers_start, headers_len);
                request->headers[headers_len] = '\0';
            }
        }
    }

    free(request_copy);
    return request;
}

// 释放 HTTP 请求
void http_free_request(HttpRequest *request) {
    if (request) {
        free(request->path);
        free(request->body);
        free(request->headers);
        free(request);
    }
}

// 创建 HTTP 响应
HttpResponse* http_create_response(int status_code, const char *body) {
    HttpResponse *response = calloc(1, sizeof(HttpResponse));
    if (!response) {
        return NULL;
    }

    response->status_code = status_code;
    response->status_text = strdup(http_status_text(status_code));
    response->content_type = strdup("text/plain");

    if (body) {
        response->body = strdup(body);
        response->body_length = strlen(body);
    } else {
        response->body = strdup("");
        response->body_length = 0;
    }

    if (!response->status_text || !response->content_type || !response->body) {
        http_free_response(response);
        return NULL;
    }

    return response;
}

// 构建 HTTP 响应字符串
char* http_build_response(HttpResponse *response, size_t *response_length) {
    if (!response) {
        return NULL;
    }

    // 计算响应字符串的大小
    size_t header_size = snprintf(NULL, 0,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        response->status_code,
        response->status_text,
        response->content_type,
        response->body_length);

    size_t total_size = header_size + response->body_length;
    char *response_str = malloc(total_size + 1);
    if (!response_str) {
        return NULL;
    }

    // 构建响应头
    int written = snprintf(response_str, header_size + 1,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        response->status_code,
        response->status_text,
        response->content_type,
        response->body_length);

    // 添加响应体
    if (response->body_length > 0) {
        memcpy(response_str + written, response->body, response->body_length);
    }

    response_str[total_size] = '\0';

    if (response_length) {
        *response_length = total_size;
    }

    return response_str;
}

// 释放 HTTP 响应
void http_free_response(HttpResponse *response) {
    if (response) {
        free(response->status_text);
        free(response->content_type);
        free(response->body);
        free(response);
    }
}

