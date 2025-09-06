# KV 存储服务器

基于 kqueue 的高性能内存键值存储服务，专为 macOS 系统优化。

## 特性

- **高性能**: 使用 kqueue 实现 IO 多路复用，支持高并发连接
- **HTTP 协议**: 基于标准 HTTP 协议，易于集成和测试
- **内存存储**: 使用哈希表实现快速的键值存储和检索
- **macOS 优化**: 充分利用 macOS 系统特性，避免 Linux 特有扩展
- **轻量级**: 纯 C 实现，资源占用少，启动速度快

## 系统要求

- macOS 系统
- clang 或 gcc 编译器
- CMake 3.10 或更高版本
- curl（用于测试）

## 构建和运行

### 1. 构建项目

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make
```

### 2. 运行服务器

```bash
# 使用默认端口 8080
./c_x

# 指定端口
./c_x 9000

# 查看帮助
./c_x --help
```

### 3. 测试服务器

在另一个终端窗口中运行测试脚本：

```bash
./test_kv_server.sh
```

## API 使用

### 设置键值对

```bash
curl -X POST http://localhost:8080/mykey -d 'myvalue'
```

响应：`HTTP 201 Created`

### 获取键值

```bash
curl http://localhost:8080/mykey
```

响应：`myvalue` (HTTP 200 OK)

### 删除键值对

```bash
curl -X DELETE http://localhost:8080/mykey
```

响应：`HTTP 204 No Content`

### 错误响应

- `HTTP 404 Not Found`: 键不存在
- `HTTP 400 Bad Request`: 请求格式错误
- `HTTP 405 Method Not Allowed`: 不支持的 HTTP 方法
- `HTTP 500 Internal Server Error`: 服务器内部错误

## 架构设计

### 模块组织

```
include/
├── kv_store.h      # KV 存储引擎接口
├── http_parser.h   # HTTP 协议解析器接口
└── kqueue_net.h    # kqueue 网络服务器接口

src/
├── main.c          # 主程序入口
├── kv_store.c      # 哈希表实现的 KV 存储引擎
├── http_parser.c   # HTTP 请求解析和响应构建
└── kqueue_net.c    # 基于 kqueue 的网络事件处理
```

### 核心组件

1. **KV 存储引擎** (`kv_store.c`)
   - 使用哈希表实现高效的键值存储
   - 支持链地址法解决哈希冲突
   - 提供 set、get、delete 基本操作

2. **HTTP 解析器** (`http_parser.c`)
   - 解析 HTTP 请求行和请求头
   - 支持 GET、POST、DELETE 方法
   - 构建标准 HTTP 响应

3. **网络服务器** (`kqueue_net.c`)
   - 使用 kqueue 实现事件驱动的网络 IO
   - 支持多客户端并发连接
   - 非阻塞 socket 处理

## 性能特点

- **事件驱动**: kqueue 提供高效的事件通知机制
- **非阻塞 IO**: 所有网络操作都是非阻塞的
- **内存高效**: 哈希表提供 O(1) 平均时间复杂度的操作
- **并发支持**: 单线程事件循环处理多个并发连接

## 限制和注意事项

1. **内存存储**: 数据仅存储在内存中，服务器重启后数据丢失
2. **单线程**: 当前实现为单线程事件循环
3. **macOS 专用**: 使用 kqueue，不支持其他操作系统
4. **HTTP/1.0 风格**: 每个请求后关闭连接，不支持持久连接

## 扩展建议

1. **持久化**: 添加数据持久化到磁盘的功能
2. **多线程**: 实现多线程处理提高性能
3. **连接池**: 支持 HTTP/1.1 持久连接
4. **监控**: 添加性能监控和统计功能
5. **配置**: 支持配置文件和更多运行时选项

## 故障排除

### 端口被占用

```bash
# 查看端口占用情况
lsof -i :8080

# 杀死占用端口的进程
kill -9 <PID>
```

### 编译错误

确保系统已安装 Xcode Command Line Tools：

```bash
xcode-select --install
```

### 权限问题

如果使用 1024 以下的端口，可能需要管理员权限：

```bash
sudo ./c_x 80
```

## 许可证

本项目遵循项目根目录下的 LICENSE 文件中指定的许可证。

