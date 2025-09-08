#!/bin/bash

# 测试 CORS 和多主机支持的脚本

echo "=== CORS 和多主机支持测试 ==="
echo

# 测试不同主机地址
HOSTS=("localhost" "127.0.0.1")
PORT=8080

echo "1. 测试不同主机地址的健康检查"
for host in "${HOSTS[@]}"; do
    echo -n "测试 $host:$PORT - "
    if curl -s --connect-timeout 3 http://$host:$PORT/health > /dev/null 2>&1; then
        echo "✅ 可访问"
    else
        echo "❌ 无法访问"
    fi
done
echo

echo "2. 测试 CORS 头部"
for host in "${HOSTS[@]}"; do
    echo "测试 $host:$PORT 的 CORS 头部:"
    curl -s -v http://$host:$PORT/health 2>&1 | grep -i "access-control" || echo "  未找到 CORS 头部"
    echo
done

echo "3. 测试 OPTIONS 预检请求"
for host in "${HOSTS[@]}"; do
    echo "测试 $host:$PORT 的 OPTIONS 请求:"
    curl -s -X OPTIONS -I http://$host:$PORT/api/test | head -10
    echo
done

echo "4. 测试跨域 API 请求"
for host in "${HOSTS[@]}"; do
    echo "测试 $host:$PORT 的 API 请求:"

    # POST 请求
    echo -n "  POST: "
    curl -s -w "%{http_code}" -X POST http://$host:$PORT/api/cors_test -d 'cross-origin test' | tail -1
    echo

    # GET 请求
    echo -n "  GET: "
    curl -s -w "%{http_code}" http://$host:$PORT/api/cors_test | tail -1
    echo

    # DELETE 请求
    echo -n "  DELETE: "
    curl -s -w "%{http_code}" -X DELETE http://$host:$PORT/api/cors_test | tail -1
    echo
    echo
done

echo "5. 测试主机候选生成（模拟）"
echo "输入 'localhost' 应该生成候选: localhost, 127.0.0.1, 0.0.0.0, ::1"
echo "输入 '127.0.0.1' 应该生成候选: 127.0.0.1, localhost, 0.0.0.0, ::1"
echo "输入 'example.com' 应该生成候选: example.com, www.example.com"
echo

echo "6. 验证 CORS 响应头"
echo "检查 API 响应是否包含必要的 CORS 头部:"
response_headers=$(curl -s -v http://localhost:$PORT/api/test_cors 2>&1)
if echo "$response_headers" | grep -q "Access-Control-Allow-Origin"; then
    echo "✅ Access-Control-Allow-Origin 头部存在"
else
    echo "❌ Access-Control-Allow-Origin 头部缺失"
fi

if echo "$response_headers" | grep -q "Access-Control-Allow-Methods"; then
    echo "✅ Access-Control-Allow-Methods 头部存在"
else
    echo "❌ Access-Control-Allow-Methods 头部缺失"
fi

if echo "$response_headers" | grep -q "Access-Control-Allow-Headers"; then
    echo "✅ Access-Control-Allow-Headers 头部存在"
else
    echo "❌ Access-Control-Allow-Headers 头部缺失"
fi
echo

echo "=== 测试完成 ==="
echo
echo "📋 使用说明:"
echo "1. 页面现在支持 localhost 和 127.0.0.1 互换访问"
echo "2. 自动检测功能会尝试多个主机地址"
echo "3. 所有 API 响应都包含 CORS 头部"
echo "4. 支持 OPTIONS 预检请求"
echo "5. 可以从任何域名访问 API（CORS: *）"

