#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>
#include <stdbool.h>

// HTTP 方法枚举
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE,
    HTTP_UNKNOWN
} HttpMethod;

// HTTP 请求结构
typedef struct {
    HttpMethod method;
    char *path;
    char *body;
    size_t body_length;
    char *headers;
} HttpRequest;

// HTTP 响应结构
typedef struct {
    int status_code;
    char *status_text;
    char *content_type;
    char *body;
    size_t body_length;
} HttpResponse;

// HTTP 解析和构建接口
HttpRequest* http_parse_request(const char *raw_request, size_t length);
void http_free_request(HttpRequest *request);

HttpResponse* http_create_response(int status_code, const char *body);
char* http_build_response(HttpResponse *response, size_t *response_length);
void http_free_response(HttpResponse *response);

// 辅助函数
const char* http_method_to_string(HttpMethod method);
HttpMethod http_string_to_method(const char *method_str);
const char* http_status_text(int status_code);

#endif // HTTP_PARSER_H

