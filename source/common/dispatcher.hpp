#pragma once
#include "message.hpp"
#include "net.hpp"

namespace bitrpc {

/*
 * Dispatcher 模块: 消息类型路由分发
 *
 * 解决的问题:
 *   MuduoServer/MuduoClient 在收到数据后只知道"有一条消息到了",
 *   不知道应该交给谁处理. Dispatcher 根据消息的 MType 把它路由到正确的业务函数.
 *
 * 三个类的协作:
 *   Callback      — 抽象基类, 统一存储不同类型回调的容器(多态)
 *   CallbackT<T>  — 模板类, 自动把 BaseMessage::ptr 转型为 shared_ptr<T>,
 *                   让用户拿到的直接是 RpcRequest/RpcResponse 等具体类型
 *   Dispatcher    — 消息路由器, 维护 MType→Callback 的注册表,
 *                   onMessage 时查表→调对应回调
 *
 * 用法:
 *   Dispatcher disp;
 *   disp.registerHandler<RpcRequest>(MType::REQ_RPC,
 *       [](conn, std::shared_ptr<RpcRequest> &req) { ... });
 *   server->setMessageCallback(std::bind(&Dispatcher::onMessage, &disp, _1, _2));
 */

/*
 * Callback: 回调抽象基类
 *
 * 作用: 让 Dispatcher 的 map 可以统一存储不同类型的回调对象.
 *       所有具体回调都继承它, 通过多态调用.
 */
class Callback {
public:
  using ptr = std::shared_ptr<Callback>;
  /*
   * onMessage: 收到消息时的处理入口(纯虚)
   * 参数 msg 类型为 BaseMessage::ptr, 表示"任意消息"
   */
  virtual void onMessage(const BaseConnection::ptr &conn,
                         BaseMessage::ptr &msg) = 0;
};

/*
 * CallbackT<T>: 带类型自动转换的回调包装
 *
 * 模板参数 T: 消息的具体类型, 如 RpcRequest / RpcResponse / ServiceRequest 等
 *
 * 核心价值:
 *   用户注册时传的 handler 签名是 void(conn, shared_ptr<T>&),
 *   CallbackT 负责把 BaseMessage::ptr 通过 dynamic_pointer_cast<T> 转型,
 *   然后再调用户的 handler. 用户完全不用手动 cast.
 *
 * 例子:
 *   CallbackT<RpcRequest> 包装用户 handler →
 *     收到消息 → dynamic_pointer_cast<RpcRequest>(msg) → 调 handler(conn, req)
 */
template <typename T>
class CallbackT : public Callback {
public:
  using ptr = std::shared_ptr<CallbackT<T>>;

  /*
   * MessageCallback: 用户传入的业务回调类型
   * 第一个参数: 连接对象
   * 第二个参数: 已经转型完的具体消息指针(如 shared_ptr<RpcRequest>)
   */
  using MessageCallback =
      std::function<void(const BaseConnection::ptr &conn,
                         std::shared_ptr<T> &msg)>;

  /*
   * 构造: 保存用户的业务回调函数
   */
  CallbackT(const MessageCallback &handler) : _handler(handler) {}

  /*
   * onMessage: 基类虚函数实现
   *
   * 流程:
   *   1. dynamic_pointer_cast<T>(msg) — 把基类指针转为具体类型 T
   *   2. _handler(conn, type_msg)     — 调用户的业务函数
   *
   * 注意: 如果 dynamic_pointer_cast 失败(类型不匹配),
   *       type_msg 为 nullptr, 由用户的 handler 自行处理.
   */
  void onMessage(const BaseConnection::ptr &conn,
                 BaseMessage::ptr &msg) override {
    auto type_msg = std::dynamic_pointer_cast<T>(msg);  // 基类 → 派生类
    _handler(conn, type_msg);                            // 调业务回调
  }

private:
  MessageCallback _handler;  // 用户传入的业务处理函数
};

/*
 * Dispatcher: 消息路由器
 *
 * 核心数据结构: map<MType, Callback::ptr> _handlers
 *   枚举值 REQ_RPC/REQ_SERVICE 等作为 key, 对应业务回调作为 value.
 *
 * 两个入口:
 *   registerHandler<T>(MType, handler)  — 注册: 把某个消息类型绑定到业务函数
 *   onMessage(conn, msg)                — 分发: 收到消息时按类型路由
 *
 * 线程安全: _mutex 保护 _handlers 的并发访问.
 *
 * 完整生命周期:
 *   初始化阶段:
 *     disp.registerHandler<RpcRequest>(REQ_RPC, onRpcRequest);
 *     disp.registerHandler<ServiceRequest>(REQ_SERVICE, onServiceRequest);
 *     server->setMessageCallback(bind(&Dispatcher::onMessage, &disp, _1, _2));
 *
 *   运行阶段 (每次收到消息):
 *     MuduoServer::onMessage → 解包 → Dispatcher::onMessage → 查 _handlers
 *       → 找到了: CallbackT<T>::onMessage → dynamic_pointer_cast → 业务函数
 *       → 没找到: 日志 + shutdown
 */
class Dispatcher {
public:
  using ptr = std::shared_ptr<Dispatcher>;

  /*
   * registerHandler<T>: 注册消息处理回调
   *
   * 模板参数 T: 消息的具体类型(RpcRequest / ServiceRequest / TopicRequest 等)
   * 参数 mtype:   消息类型枚举值(REQ_RPC / REQ_SERVICE 等)
   * 参数 handler: 业务处理函数, 签名: void(conn, shared_ptr<T>&)
   *
   * 内部操作:
   *   1. 用 handler 构造 CallbackT<T> 对象(存入时会自动做类型转换)
   *   2. 以 mtype 为 key 插入 _handlers map
   */
  template <typename T>
  void registerHandler(MType mtype,
                       const typename CallbackT<T>::MessageCallback &handler) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto cb = std::make_shared<CallbackT<T>>(handler);  // 包装成带转型的回调
    _handlers.insert(std::make_pair(mtype, cb));         // mtype → 回调
  }

  /*
   * onMessage: 消息分发入口
   *
   * 由 Server/Client 的 _cb_message 钩子间接调用, 在 EventLoop 线程中执行.
   *
   * 流程:
   *   1. 取 msg->mtype() 获取消息类型
   *   2. 在 _handlers 中查找对应的回调
   *   3. 找到了 → 调 Callback::onMessage(conn, msg) 交给业务处理
   *   4. 没找到 → 日志告警 + conn->shutdown() 关闭连接
   *
   * 为什么找不到要 shutdown?
   *   正常情况下不会发生(所有消息类型都应该注册),
   *   如果发生了说明有人发了非法消息, 最安全的做法是断连.
   */
  void onMessage(const BaseConnection::ptr &conn, BaseMessage::ptr &msg) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto it = _handlers.find(msg->mtype());                // 按消息类型查表
    if (it != _handlers.end()) {
      return it->second->onMessage(conn, msg);             // 找到 → 调业务回调
    }
    // 未注册的消息类型 — 防御性处理
    ELOG("收到未知类型的消息: %d！", (int)msg->mtype());
    conn->shutdown();
  }

private:
  std::mutex _mutex;
  /*
   * _handlers: 消息类型 → 回调的注册表
   *
   * key:   MType 枚举(REQ_RPC / RSP_RPC / REQ_SERVICE / ...)
   * value: Callback::ptr (多态指针, 实际指向 CallbackT<T>)
   *
   * 通过多态实现: 同一个 map 可以存储处理不同类型的回调对象.
   */
  std::unordered_map<MType, Callback::ptr> _handlers;
};

} // namespace bitrpc
