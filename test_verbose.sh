#!/bin/bash

# 测试脚本 - 用于触发各种请求以查看详细日志

SERVER_URL="http://localhost:8080"

echo "=== 开始测试各种请求 ==="
echo

echo "1. 测试根路径重定向"
curl -v $SERVER_URL/ 2>&1 | head -10
echo
sleep 1

echo "2. 测试 Web 页面访问"
curl -s -o /dev/null -w "HTTP状态码: %{http_code}\n" $SERVER_URL/web/
echo
sleep 1

echo "3. 测试 API POST 请求"
curl -X POST $SERVER_URL/api/test_key -d 'test_value' -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "4. 测试 API GET 请求"
curl $SERVER_URL/api/test_key -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "5. 测试 API DELETE 请求"
curl -X DELETE $SERVER_URL/api/test_key -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "6. 测试无效路径"
curl $SERVER_URL/invalid_path -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "7. 测试不支持的方法"
curl -X PUT $SERVER_URL/api/test -d 'data' -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "8. 测试空键名"
curl -X POST $SERVER_URL/api/ -d 'data' -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "9. 测试 favicon.ico 请求"
curl $SERVER_URL/favicon.ico -w " (HTTP: %{http_code})\n"
echo
sleep 1

echo "10. 测试中文路径"
curl $SERVER_URL/中文路径 -w " (HTTP: %{http_code})\n"
echo

echo "=== 测试完成 ==="

