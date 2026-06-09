#pragma once
#include "fields.hpp"
#include <functional>
#include <memory>

namespace bitrpc {
class BaseMessage { // 基础消息类
public:
  using ptr = std::shared_ptr<BaseMessage>;
  virtual ~BaseMessage() {}                                // 析构函数
  virtual void setId(const std::string &id) { _rid = id; } // 设置消息ID
  virtual std::string rid() { return _rid; }               // 获取消息ID
  virtual void setMType(MType mtype) { _mtype = mtype; }   // 设置消息类型
  virtual MType mtype() { return _mtype; }                 // 获取消息类型
  virtual std::string serialize() = 0;                     // 序列化消息
  virtual bool unserialize(const std::string &msg) = 0;    // 反序列化消息
  virtual bool check() = 0;                                // 检查消息是否有效

private:
  MType _mtype;     // 消息类型
  std::string _rid; // 消息ID
};

class BaseBuffer { // 基础缓冲区类
public:
  using ptr = std::shared_ptr<BaseBuffer>;              // 基础缓冲区类指针类型
  virtual size_t readableSize() = 0;                    // 可读字节数
  virtual int32_t peekInt32() = 0;                      // 查看32位整数
  virtual void retrieveInt32() = 0;                     // 检索32位整数
  virtual int32_t readInt32() = 0;                      // 读取32位整数
  virtual std::string retrieveAsString(size_t len) = 0; // 检索字符串
};

class BaseProtocol { // 基础协议类
public:
  using ptr = std::shared_ptr<BaseProtocol>; // 基础协议类指针类型
  virtual bool canProcessed(const BaseBuffer::ptr &buf) = 0; // 是否可以处理
  virtual bool onMessage(const BaseBuffer::ptr &buf,
                         BaseMessage::ptr &msg) = 0;              // 处理消息
  virtual std::string serialize(const BaseMessage::ptr &msg) = 0; // 序列化消息
};

class BaseConnection { // 基础连接类
public:
  using ptr = std::shared_ptr<BaseConnection>;        // 基础连接类指针类型
  virtual void send(const BaseMessage::ptr &msg) = 0; // 发送消息
  virtual void shutdown() = 0;                        // 关闭连接
  virtual bool connected() = 0;                       // 是否已连接
};

using ConnectionCallback =
    std::function<void(const BaseConnection::ptr &)>; // 连接回调函数
using CloseCallback =
    std::function<void(const BaseConnection::ptr &)>; // 关闭回调函数
using MessageCallback = std::function<void(const BaseConnection::ptr &,
                                           BaseMessage::ptr &)>; // 消息回调函数
class BaseServer {                                               // 基础服务器类
public:
  using ptr = std::shared_ptr<BaseServer>; // 基础服务器类指针类型
  virtual void setConnectionCallback(const ConnectionCallback &cb) {
    _cb_connection = cb; // 设置连接回调函数
  }
  virtual void setCloseCallback(const CloseCallback &cb) {
    _cb_close = cb; // 设置关闭回调函数
  }
  virtual void setMessageCallback(const MessageCallback &cb) {
    _cb_message = cb; // 设置消息回调函数
  }
  virtual void start() = 0;
                    // 启动服务器
protected:
    ConnectionCallback _cb_connection; // 连接回调函数
    CloseCallback _cb_close;           // 关闭回调函数
    MessageCallback _cb_message;       // 消息回调函数

};
  class BaseClient { // 基础客户端类
  public:
    using ptr = std::shared_ptr<BaseClient>; // 基础客户端类指针类型
    virtual void setConnectionCallback(const ConnectionCallback &cb) {
      _cb_connection = cb; // 设置连接回调函数
    }
    virtual void setCloseCallback(const CloseCallback &cb) {
      _cb_close = cb; // 设置关闭回调函数
    }
    virtual void setMessageCallback(const MessageCallback &cb) {
      _cb_message = cb; // 设置消息回调函数
    }
    virtual void connect() = 0;                      // 连接服务器
    virtual void shutdown() = 0;                     // 关闭连接
    virtual bool send(const BaseMessage::ptr &) = 0; // 发送消息
    virtual BaseConnection::ptr connection() = 0;    // 获取连接
    virtual bool connected() = 0;                    // 是否已连接

  protected:
    ConnectionCallback _cb_connection; // 连接回调函数
    CloseCallback _cb_close;           // 关闭回调函数
    MessageCallback _cb_message;       // 消息回调函数
  };
} // namespace bitrpc