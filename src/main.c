#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "kqueue_net.h"

// 全局服务器实例，用于信号处理
static KVServer *g_server = NULL;

// 全局详细日志标志
bool g_verbose = false;

// 信号处理函数
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\n收到停止信号，正在关闭服务器...\n");
        if (g_server) {
            server_stop(g_server);
        }
    }
}

// 打印使用说明
void print_usage(const char *program_name) {
    printf("用法: %s [选项] [端口号]\n", program_name);
    printf("  端口号: 服务器监听的端口号 (默认: 8080)\n");
    printf("\n");
    printf("选项:\n");
    printf("  -v, --verbose     启用详细日志输出\n");
    printf("  -h, --help        显示此帮助信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s        # 使用默认端口 8080\n", program_name);
    printf("  %s 9000   # 使用端口 9000\n", program_name);
    printf("  %s -v 8080 # 启用详细日志，使用端口 8080\n", program_name);
    printf("\n");
    printf("路径说明:\n");
    printf("  /             - 重定向到 /web/\n");
    printf("  /web/         - 测试页面\n");
    printf("  /api/{key}    - KV 操作 API\n");
    printf("  /health       - 健康检查端点\n");
    printf("  /test_connection - 连接测试端点\n");
    printf("\n");
    printf("API 使用说明:\n");
    printf("  GET /api/key      - 获取键值\n");
    printf("  POST /api/key     - 设置键值 (请求体为值)\n");
    printf("  DELETE /api/key   - 删除键值\n");
    printf("\n");
    printf("测试示例:\n");
    printf("  curl -X POST http://localhost:8080/api/mykey -d 'myvalue'\n");
    printf("  curl http://localhost:8080/api/mykey\n");
    printf("  curl -X DELETE http://localhost:8080/api/mykey\n");
}

int main(int argc, char *argv[]) {
    int port = 8080; // 默认端口
    int arg_index = 1;

    // 解析命令行参数
    while (arg_index < argc) {
        if (strcmp(argv[arg_index], "-h") == 0 || strcmp(argv[arg_index], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[arg_index], "-v") == 0 || strcmp(argv[arg_index], "--verbose") == 0) {
            g_verbose = true;
            arg_index++;
        } else {
            // 尝试解析为端口号
            char *endptr;
            long parsed_port = strtol(argv[arg_index], &endptr, 10);

            if (*endptr != '\0' || parsed_port <= 0 || parsed_port > 65535) {
                fprintf(stderr, "错误: 无效的端口号 '%s'\n", argv[arg_index]);
                fprintf(stderr, "端口号必须是 1-65535 之间的整数\n");
                return 1;
            }

            port = (int)parsed_port;
            arg_index++;
        }
    }

    printf("=== KV 存储服务器 ===\n");
    printf("基于 kqueue 的高性能内存键值存储服务\n");
    printf("支持 HTTP 协议的 GET、POST、DELETE 操作\n");
    printf("========================\n\n");

    // 创建服务器
    g_server = server_create(port);
    if (!g_server) {
        fprintf(stderr, "错误: 无法创建服务器\n");
        return 1;
    }

    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN); // 忽略 SIGPIPE 信号

    // 启动服务器
    if (!server_start(g_server)) {
        fprintf(stderr, "错误: 无法启动服务器\n");
        server_destroy(g_server);
        return 1;
    }

    // 运行服务器事件循环
    server_run(g_server);

    // 清理资源
    server_destroy(g_server);
    g_server = NULL;

    printf("服务器已正常退出\n");
    return 0;
}