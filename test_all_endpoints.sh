#!/bin/bash

# 测试所有端点的脚本

SERVER_URL="http://localhost:8080"

echo "=== KV 存储服务器端点测试 ==="
echo

echo "1. 测试根路径重定向"
curl -s -w "HTTP状态码: %{http_code}\n" $SERVER_URL/ | head -1
echo

echo "2. 测试健康检查端点"
curl -s -w "HTTP状态码: %{http_code}\n" $SERVER_URL/health
echo

echo "3. 测试连接测试端点"
curl -s -w "HTTP状态码: %{http_code}\n" $SERVER_URL/test_connection
echo

echo "4. 测试 Web 页面"
curl -s -o /dev/null -w "HTTP状态码: %{http_code}\n" $SERVER_URL/web/
echo

echo "5. 测试 API 端点 - POST"
curl -s -w "HTTP状态码: %{http_code}\n" -X POST $SERVER_URL/api/test_key -d 'test_value'
echo

echo "6. 测试 API 端点 - GET"
curl -s -w "HTTP状态码: %{http_code}\n" $SERVER_URL/api/test_key
echo

echo "7. 测试 API 端点 - DELETE"
curl -s -w "HTTP状态码: %{http_code}\n" -X DELETE $SERVER_URL/api/test_key
echo

echo "8. 测试无效路径 (应该返回 404)"
curl -s -w "HTTP状态码: %{http_code}\n" $SERVER_URL/invalid_path
echo

echo "9. 测试不支持的方法 (应该返回 405)"
curl -s -w "HTTP状态码: %{http_code}\n" -X PUT $SERVER_URL/api/test -d 'data'
echo

echo "10. 测试空键名 (应该返回 400)"
curl -s -w "HTTP状态码: %{http_code}\n" -X POST $SERVER_URL/api/ -d 'data'
echo

echo "=== 测试完成 ==="
echo
echo "预期结果:"
echo "  1. 根路径重定向: 302"
echo "  2. 健康检查: 200"
echo "  3. 连接测试: 200"
echo "  4. Web 页面: 200"
echo "  5. API POST: 201"
echo "  6. API GET: 200"
echo "  7. API DELETE: 204"
echo "  8. 无效路径: 404"
echo "  9. 不支持的方法: 405"
echo "  10. 空键名: 400"

