#!/bin/bash

# 模拟浏览器请求的测试脚本

SERVER_URL="http://localhost:8080"

echo "=== 模拟浏览器请求测试 ==="
echo

echo "1. 模拟 Chrome 浏览器访问根路径"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/139.0.0.0 Safari/537.36" \
  -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7" \
  -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8" \
  -H "Accept-Encoding: gzip, deflate, br" \
  -H "Connection: keep-alive" \
  -H "Upgrade-Insecure-Requests: 1" \
  -H "sec-ch-ua: \"Not;A=Brand\";v=\"99\", \"Google Chrome\";v=\"139\", \"Chromium\";v=\"139\"" \
  -H "sec-ch-ua-mobile: ?0" \
  -H "sec-ch-ua-platform: \"macOS\"" \
  $SERVER_URL/ | head -1
echo
sleep 1

echo "2. 模拟浏览器访问 /web/ 页面"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36" \
  -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" \
  -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8" \
  -H "Connection: keep-alive" \
  $SERVER_URL/web/ | head -1
echo
sleep 1

echo "3. 模拟浏览器请求 favicon.ico"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36" \
  -H "Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8" \
  $SERVER_URL/favicon.ico
echo
sleep 1

echo "4. 模拟 Safari 浏览器访问"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15" \
  -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" \
  -H "Accept-Language: zh-CN,zh-Hans;q=0.9" \
  -H "Accept-Encoding: gzip, deflate" \
  -H "Connection: keep-alive" \
  $SERVER_URL/ | head -1
echo
sleep 1

echo "5. 模拟 Firefox 浏览器访问"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/119.0" \
  -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8" \
  -H "Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2" \
  -H "Accept-Encoding: gzip, deflate" \
  -H "Connection: keep-alive" \
  $SERVER_URL/ | head -1
echo
sleep 1

echo "6. 测试长请求头"
curl -s -w "HTTP状态码: %{http_code}\n" \
  -H "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/139.0.0.0 Safari/537.36" \
  -H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7" \
  -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,ja;q=0.7,ko;q=0.6,fr;q=0.5,de;q=0.4,es;q=0.3,it;q=0.2,pt;q=0.1" \
  -H "Accept-Encoding: gzip, deflate, br, zstd" \
  -H "Connection: keep-alive" \
  -H "Upgrade-Insecure-Requests: 1" \
  -H "sec-ch-ua: \"Not;A=Brand\";v=\"99\", \"Google Chrome\";v=\"139\", \"Chromium\";v=\"139\"" \
  -H "sec-ch-ua-mobile: ?0" \
  -H "sec-ch-ua-platform: \"macOS\"" \
  -H "sec-fetch-site: none" \
  -H "sec-fetch-mode: navigate" \
  -H "sec-fetch-user: ?1" \
  -H "sec-fetch-dest: document" \
  -H "Custom-Header-1: This is a very long custom header value that might cause issues with buffer overflow or memory allocation problems in poorly written HTTP parsers" \
  -H "Custom-Header-2: Another long header with lots of data to test the robustness of the HTTP request parsing implementation" \
  $SERVER_URL/api/test_long_headers
echo
sleep 1

echo "7. 测试并发请求"
for i in {1..5}; do
  curl -s -w "请求$i HTTP状态码: %{http_code}\n" \
    -H "User-Agent: TestClient-$i" \
    $SERVER_URL/api/concurrent_test_$i &
done
wait
echo

echo "=== 测试完成 ==="

