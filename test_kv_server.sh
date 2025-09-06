#!/bin/bash

# KV 存储服务器测试脚本
# 使用 curl 测试 GET、POST、DELETE 操作

SERVER_URL="http://localhost:8080"
TEST_KEY="test_key"
TEST_VALUE="Hello, KV Store!"

echo "=== KV 存储服务器测试 ==="
echo "服务器地址: $SERVER_URL"
echo

# 测试 1: 设置键值对
echo "测试 1: 设置键值对"
echo "POST $SERVER_URL/$TEST_KEY"
echo "Body: $TEST_VALUE"
curl -X POST "$SERVER_URL/$TEST_KEY" -d "$TEST_VALUE" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 2: 获取键值
echo "测试 2: 获取键值"
echo "GET $SERVER_URL/$TEST_KEY"
curl "$SERVER_URL/$TEST_KEY" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 3: 获取不存在的键
echo "测试 3: 获取不存在的键"
echo "GET $SERVER_URL/nonexistent_key"
curl "$SERVER_URL/nonexistent_key" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 4: 删除键值对
echo "测试 4: 删除键值对"
echo "DELETE $SERVER_URL/$TEST_KEY"
curl -X DELETE "$SERVER_URL/$TEST_KEY" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 5: 再次获取已删除的键
echo "测试 5: 再次获取已删除的键"
echo "GET $SERVER_URL/$TEST_KEY"
curl "$SERVER_URL/$TEST_KEY" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 6: 删除不存在的键
echo "测试 6: 删除不存在的键"
echo "DELETE $SERVER_URL/nonexistent_key"
curl -X DELETE "$SERVER_URL/nonexistent_key" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

# 测试 7: 测试中文键值
echo "测试 7: 测试中文键值"
CHINESE_KEY="中文键"
CHINESE_VALUE="中文值"
echo "POST $SERVER_URL/$CHINESE_KEY"
echo "Body: $CHINESE_VALUE"
curl -X POST "$SERVER_URL/$CHINESE_KEY" -d "$CHINESE_VALUE" -w "\nHTTP 状态码: %{http_code}\n"
echo
curl "$SERVER_URL/$CHINESE_KEY" -w "\nHTTP 状态码: %{http_code}\n"
echo
echo "---"

echo "测试完成！"

