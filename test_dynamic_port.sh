#!/bin/bash

# 测试动态端口功能的脚本

echo "=== 动态端口功能测试 ==="
echo

echo "1. 测试端口 9000 的健康检查"
curl -s -w "HTTP状态码: %{http_code}\n" http://localhost:9000/health
echo

echo "2. 测试端口 9000 的 Web 页面"
curl -s -o /dev/null -w "HTTP状态码: %{http_code}\n" http://localhost:9000/web/
echo

echo "3. 测试端口 9000 的 API 功能"
curl -s -w "HTTP状态码: %{http_code}\n" -X POST http://localhost:9000/api/dynamic_test -d 'dynamic port test'
echo

echo "4. 验证设置的值"
curl -s -w "HTTP状态码: %{http_code}\n" http://localhost:9000/api/dynamic_test
echo

echo "5. 测试自动检测功能（模拟）"
echo "常见端口检测顺序: 8080, 3000, 8000, 9000, 8888, 5000, 8081, 8082"
for port in 8080 3000 8000 9000 8888 5000 8081 8082; do
    echo -n "检测端口 $port: "
    if curl -s --connect-timeout 1 http://localhost:$port/health > /dev/null 2>&1; then
        echo "✅ 服务器运行中"
        break
    else
        echo "❌ 无响应"
    fi
done
echo

echo "=== 使用说明 ==="
echo "1. 访问 http://localhost:9000/web/"
echo "2. 在'服务器配置'区域："
echo "   - 修改端口为 9000"
echo "   - 点击'更新配置'按钮"
echo "   - 或点击'自动检测'按钮"
echo "3. 页面将自动连接到新端口的服务器"
echo "4. 配置会保存到浏览器本地存储"

