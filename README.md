# C-X

一个跨平台的C项目模板。

## 特性

- 跨平台支持 (Windows, macOS, Linux)
- 现代CMake构建系统
- 单元测试框架
- 版本控制

## 构建与运行

### 前提条件

- CMake 3.10+
- C编译器 (GCC, Clang, MSVC等)

### 使用CMake构建（推荐）

#### 编译

```bash
mkdir -p build
cd build
cmake ..
make
```

#### 运行

```bash
./c_x
```

#### 构建并运行测试

```bash
mkdir -p build-test
cd build-test
cmake -DBUILD_TESTS=ON ..
make
ctest
```

### 使用Makefile构建

项目也提供了一个简单的Makefile，可以直接使用make命令构建：

```bash
# 构建项目
make

# 运行程序
./c_x

# 构建并运行测试
make test

# 清理构建产物
make clean
```

### 直接编译（不使用构建系统）

如果你想直接使用编译器命令编译，可以使用以下命令：

```bash
cc -o main src/main.c src/c_x.c -Iinclude
```

然后运行：

```bash
./main
```

## 项目结构

```
.
├── include/         # 头文件
├── src/            # 源代码
├── tests/          # 测试文件
├── docs/           # 文档
├── CMakeLists.txt  # CMake构建配置
├── Makefile        # 简单的Makefile
├── LICENSE         # 许可证文件
└── README.md       # 项目说明
```

## 在CLion中开发

本项目可以直接在CLion中打开和开发。CLion会自动识别CMake项目并提供相应的构建和运行配置。

## 许可证

MIT

