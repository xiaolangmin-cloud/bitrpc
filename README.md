# bitrpc

一个基于 C++11、muduo 和 jsoncpp 实现的轻量级 RPC 框架。

支持同步调用、异步调用、回调调用三种通信模式，实现服务注册发现与发布订阅（Pub/Sub）功能，并通过自定义 LVProtocol 协议解决 TCP 粘包拆包问题。

---

## 项目简介

RPC（Remote Procedure Call）允许程序像调用本地函数一样调用远程服务。

本项目从零实现 RPC 核心功能，包括：

- 服务注册与发现
- 同步 / 异步 / 回调调用
- 发布订阅系统
- 自定义通信协议
- 请求响应匹配
- 消息路由分发

---

## 技术栈

- C++11
- muduo
- jsoncpp
- Linux
- TCP Socket
- 多线程
- std::future / std::promise

---

## 整体架构

```text
Client
 │
 ▼
Requestor
 │
 ▼
LVProtocol
 │
 ▼
TCP Connection
 │
 ▼
Provider
 │
 ▼
Service
```

---

## 核心功能

### RPC 调用

支持三种调用模式：

#### 同步调用

```cpp
auto rsp = client.call(req);
```

#### 异步调用

```cpp
auto future = client.async_call(req);
auto rsp = future.get();
```

#### 回调调用

```cpp
client.call(req, callback);
```

---

### 服务注册与发现

Provider 启动后自动注册服务。

Discoverer 负责：

- 服务查询
- 节点发现
- 上下线通知

实现服务动态管理。

---

### 发布订阅系统

支持：

- 创建 Topic
- 删除 Topic
- 订阅 Topic
- 取消订阅
- 消息广播

实现轻量级消息总线功能。

---

### 自定义 LVProtocol

协议格式：

```text
+----------+----------+------------+----------+
| Length   | Type     | RequestID  |  Body    |
| 4 Bytes  | 4 Bytes  |  4 Bytes   |  JSON    |
+----------+----------+------------+----------+
```

特点：

- 解决 TCP 粘包问题
- 解决 TCP 半包问题
- 支持多种消息类型扩展

---

## 项目亮点

### 请求响应匹配机制

通过 RequestID 实现请求与响应的精准匹配。

支持多个 RPC 请求并发执行。

---

### future / promise

利用：

```cpp
std::future
std::promise
```

实现同步调用与异步调用统一封装。

---

### Dispatcher 消息分发器

根据消息类型自动路由：

```text
MessageType
    ↓
 Dispatcher
    ↓
 Handler
```

实现网络层与业务层解耦。

---

## 编译

```bash
mkdir build
cd build

cmake ..
make -j
```

---

## 运行示例

启动 Provider：

```bash
./provider
```

启动 Client：

```bash
./client
```

---

## 项目收获

- 深入理解 RPC 框架设计思想
- 掌握 TCP 通信协议设计
- 理解服务注册发现机制
- 熟悉 future/promise 异步编程模型
- 学习 muduo 网络库核心设计思想

---

## 作者

王忆康

Software Engineering Student | C++ Backend Developer
