# KV Storage Server

🚀 基于 kqueue 的高性能内存键值存储服务器，专为 macOS 系统优化。

[![Build Status](https://github.com/your-username/kv-storage-server/workflows/KV%20Storage%20Server%20-%20Build%20and%20Test/badge.svg)](https://github.com/your-username/kv-storage-server/actions)
[![Code Quality](https://github.com/your-username/kv-storage-server/workflows/Code%20Quality%20Check/badge.svg)](https://github.com/your-username/kv-storage-server/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 🌟 功能特性

### 核心功能
- ✅ **高性能**: 基于 macOS kqueue 事件驱动架构
- ✅ **内存存储**: 快速的内存键值存储
- ✅ **HTTP API**: RESTful API 接口
- ✅ **Web 界面**: 直观的管理界面
- ✅ **跨域支持**: 完整的 CORS 支持
- ✅ **动态配置**: 支持动态端口和主机配置

### 网络特性
- 🌐 **多主机支持**: localhost, 127.0.0.1, 自定义域名
- 🔄 **自动检测**: 智能端口和主机检测
- 🛡️ **安全**: CORS 安全策略，输入验证
- ⚡ **并发**: 高并发连接处理

### 开发特性
- 📊 **详细日志**: 可选的详细调试日志
- 🧪 **测试覆盖**: 完整的测试套件
- 🔍 **静态分析**: 代码质量保证
- 📦 **CI/CD**: GitHub Actions 自动化

## 🚀 快速开始

### 系统要求

- **操作系统**: macOS 10.15+ (支持 kqueue)
- **编译器**: Clang 或 GCC
- **构建工具**: CMake 3.15+, Ninja (推荐)
- **运行时**: 无外部依赖

### 安装步骤

1. **克隆仓库**
   ```bash
   git clone https://github.com/your-username/kv-storage-server.git
   cd kv-storage-server
   ```

2. **构建项目**
   ```bash
   # 创建构建目录
   mkdir build && cd build

   # 配置 CMake
   cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

   # 编译
   ninja

   # 复制 Web 文件
   cp -r ../web .
   ```

3. **启动服务器**
   ```bash
   # 启动在默认端口 8080
   ./c_x 8080

   # 或使用详细日志模式
   ./c_x -v 8080
   ```

4. **访问服务**
   - **Web 界面**: http://localhost:8080/web/
   - **健康检查**: http://localhost:8080/health
   - **API 端点**: http://localhost:8080/api/{key}

### 使用启动脚本

```bash
# 使用项目提供的启动脚本
chmod +x start_server.sh
./start_server.sh
```

## 📖 API 文档

### 端点概览

| 端点 | 方法 | 描述 |
|------|------|------|
| `/` | GET | 重定向到 Web 界面 |
| `/web/` | GET | Web 管理界面 |
| `/health` | GET | 健康检查 |
| `/test_connection` | GET | 连接测试 |
| `/api/{key}` | GET | 获取键值 |
| `/api/{key}` | POST | 设置键值 |
| `/api/{key}` | DELETE | 删除键值 |
| `/*` | OPTIONS | CORS 预检 |

### API 使用示例

#### 设置键值对
```bash
curl -X POST http://localhost:8080/api/user:123 \
     -H "Content-Type: text/plain" \
     -d "John Doe"
```

#### 获取键值
```bash
curl http://localhost:8080/api/user:123
# 响应: John Doe
```

#### 删除键值对
```bash
curl -X DELETE http://localhost:8080/api/user:123
```

#### 健康检查
```bash
curl http://localhost:8080/health
# 响应: {"status":"ok","service":"KV Storage Server","timestamp":1234567890}
```

### HTTP 状态码

| 状态码 | 描述 |
|--------|------|
| 200 | 成功获取 |
| 201 | 成功创建 |
| 204 | 成功删除 |
| 302 | 重定向 |
| 400 | 请求错误 |
| 404 | 键不存在 |
| 405 | 方法不允许 |
| 500 | 服务器错误 |

## 🎮 Web 界面使用

### 功能介绍

1. **服务器配置**
   - 动态修改服务器地址和端口
   - 自动检测运行中的服务器
   - 配置本地存储

2. **键值操作**
   - 设置键值对
   - 获取键值
   - 删除键值对
   - 批量测试

3. **快速测试**
   - 一键测试所有功能
   - 性能测试
   - 连接测试

### 使用步骤

1. 访问 http://localhost:8080/web/
2. 在"服务器配置"区域设置正确的主机和端口
3. 点击"更新配置"或"自动检测"
4. 使用各种功能测试 KV 存储

## 🧪 测试

### 运行测试

```bash
# 基本功能测试
./test_all_endpoints.sh

# 动态端口测试
./test_dynamic_port.sh

# CORS 和多主机测试
./test_cors_and_hosts.sh

# 浏览器模拟测试
./test_browser_simulation.sh
```

### 测试覆盖

- ✅ HTTP 端点测试
- ✅ API 功能测试
- ✅ CORS 支持测试
- ✅ 多主机访问测试
- ✅ 并发性能测试
- ✅ 错误处理测试

## 🔧 开发

### 项目结构

```
kv-storage-server/
├── src/                    # 源代码
│   ├── main.c             # 主程序入口
│   ├── kqueue_net.c       # 网络和事件处理
│   ├── http_parser.c      # HTTP 协议解析
│   └── kv_store.c         # 键值存储实现
├── include/               # 头文件
│   ├── kqueue_net.h
│   ├── http_parser.h
│   └── kv_store.h
├── web/                   # Web 界面
│   └── index.html
├── .github/workflows/     # CI/CD 配置
├── CMakeLists.txt         # 构建配置
└── README.md             # 项目文档
```

### 编译选项

```bash
# Debug 模式（包含调试信息）
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release 模式（优化性能）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 使用 GCC 编译
cmake .. -DCMAKE_C_COMPILER=gcc-13

# 使用 Clang 编译（默认）
cmake .. -DCMAKE_C_COMPILER=clang
```

### 代码风格

项目使用 clang-format 进行代码格式化：

```bash
# 格式化所有源文件
find src include -name "*.c" -o -name "*.h" | xargs clang-format -i

# 检查格式
find src include -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror
```

## 🚀 部署

### 生产环境部署

1. **编译 Release 版本**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
   cmake --build build --parallel
   ```

2. **创建部署包**
   ```bash
   mkdir -p deploy
   cp build/c_x deploy/
   cp -r web deploy/
   cp start_server.sh deploy/
   ```

3. **启动服务**
   ```bash
   cd deploy
   ./start_server.sh
   ```

### 系统服务配置

创建 macOS LaunchDaemon 配置：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.yourcompany.kvserver</string>
    <key>ProgramArguments</key>
    <array>
        <string>/path/to/c_x</string>
        <string>8080</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    <key>KeepAlive</key>
    <true/>
</dict>
</plist>
```

## 🔍 故障排除

### 常见问题

1. **端口被占用**
   ```bash
   # 检查端口使用情况
   lsof -i :8080

   # 使用其他端口
   ./c_x 9000
   ```

2. **权限问题**
   ```bash
   # 确保可执行文件有执行权限
   chmod +x c_x

   # 确保脚本有执行权限
   chmod +x *.sh
   ```

3. **Web 文件缺失**
   ```bash
   # 确保 web 目录存在
   mkdir -p web
   cp -r ../web/* web/
   ```

### 调试模式

启用详细日志进行调试：

```bash
# 启用详细日志
./c_x -v 8080

# 查看系统日志
tail -f /var/log/system.log | grep c_x
```

### 性能调优

1. **监控连接数**
   ```bash
   # 查看服务器连接
   lsof -p $(pgrep c_x) | grep IPv4
   ```

2. **内存使用**
   ```bash
   # 监控内存使用
   top -pid $(pgrep c_x)
   ```

## 🤝 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

### 开发规范

- 遵循现有代码风格
- 添加适当的注释
- 编写测试用例
- 更新文档

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- 感谢 macOS kqueue 提供的高性能事件处理
- 感谢开源社区的贡献和支持

## 📞 联系

- 项目主页: https://github.com/your-username/kv-storage-server
- 问题反馈: https://github.com/your-username/kv-storage-server/issues
- 邮箱: your-email@example.com

---

⭐ 如果这个项目对你有帮助，请给它一个星标！

