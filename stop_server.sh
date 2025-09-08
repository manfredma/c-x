#!/bin/bash

# KV 存储服务器停止脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

print_info "=== 停止 KV 存储服务器 ==="

# 查找正在运行的服务器进程
PIDS=$(pgrep -f "c_x" 2>/dev/null || true)

if [ -z "$PIDS" ]; then
    print_warning "没有找到正在运行的 KV 存储服务器"
    exit 0
fi

print_info "找到正在运行的服务器进程: $PIDS"

# 优雅地停止服务器
print_info "正在停止服务器..."
pkill -TERM -f "c_x" 2>/dev/null || true

# 等待进程结束
sleep 2

# 检查是否还有进程在运行
REMAINING_PIDS=$(pgrep -f "c_x" 2>/dev/null || true)

if [ -n "$REMAINING_PIDS" ]; then
    print_warning "进程仍在运行，强制终止..."
    pkill -KILL -f "c_x" 2>/dev/null || true
    sleep 1
fi

# 最终检查
FINAL_PIDS=$(pgrep -f "c_x" 2>/dev/null || true)

if [ -z "$FINAL_PIDS" ]; then
    print_success "✅ KV 存储服务器已成功停止"
else
    print_error "❌ 无法停止服务器，请手动终止进程: $FINAL_PIDS"
    exit 1
fi

