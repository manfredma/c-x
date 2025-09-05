# C-X

一个跨平台的C项目模板。

## 特性

- 跨平台支持 (Windows, macOS, Linux)
- 现代CMake构建系统
- 单元测试框架
- 版本控制

## 构建

### 前提条件

- CMake 3.10+
- C编译器 (GCC, Clang, MSVC等)

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

### 运行

```bash
./c_x
```

### 测试

```bash
mkdir build
cd build
cmake -DBUILD_TESTS=ON ..
make
ctest
```

## 许可证

MIT

