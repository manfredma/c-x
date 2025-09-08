#!/bin/bash

# KV 存储服务器启动脚本
# 自动编译、复制文件并启动服务器

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 默认端口
PORT=${1:-8080}

print_info "=== KV 存储服务器启动脚本 ==="
print_info "端口: $PORT"
echo

# 检查是否存在构建目录
if [ ! -d "build" ]; then
    print_info "创建构建目录..."
    mkdir -p build
fi

# 进入构建目录
cd build

# 运行 CMake 配置
print_info "配置项目..."
if ! cmake .. > /dev/null 2>&1; then
    print_error "CMake 配置失败"
    exit 1
fi

# 编译项目
print_info "编译项目..."
if ! make > /dev/null 2>&1; then
    print_error "编译失败"
    exit 1
fi

print_success "编译完成"

# 复制 web 文件
if [ -d "../web" ]; then
    print_info "复制 web 文件..."
    cp -r ../web .
    print_success "web 文件复制完成"
else
    print_warning "web 目录不存在，将无法访问测试页面"
fi

# 检查端口是否被占用
if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; then
    print_warning "端口 $PORT 已被占用，尝试停止现有进程..."
    pkill -f "c_x $PORT" 2>/dev/null || true
    sleep 1
fi

# 启动服务器
print_info "启动服务器..."
echo
print_success "🚀 KV 存储服务器启动成功！"
echo
echo "📋 服务信息:"
echo "   • 服务器地址: http://localhost:$PORT"
echo "   • 测试页面: http://localhost:$PORT/web/"
echo "   • API 端点: http://localhost:$PORT/api/{key}"
echo "   • 健康检查: http://localhost:$PORT/health"
echo "   • 连接测试: http://localhost:$PORT/test_connection"
echo
echo "🔧 API 使用示例:"
echo "   • 设置: curl -X POST http://localhost:$PORT/api/mykey -d 'myvalue'"
echo "   • 获取: curl http://localhost:$PORT/api/mykey"
echo "   • 删除: curl -X DELETE http://localhost:$PORT/api/mykey"
echo "   • 健康检查: curl http://localhost:$PORT/health"
echo
echo "⏹️  停止服务器: 按 Ctrl+C 或运行 'pkill -f c_x'"
echo

# 启动服务器
exec ./c_x $PORT

