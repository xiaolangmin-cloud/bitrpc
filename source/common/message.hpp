#pragma once
#include "abstract.hpp"
#include "detail.hpp"
#include "fields.hpp"

namespace bitrpc {
class JsonMessage : public BaseMessage {
public:
  using ptr = std::shared_ptr<JsonMessage>;
  std::string serialize() override { // 序列化
    std ::string body;
    bool ret = JSON::serialize(_body, body);
    if (ret) {
      return body;
    }
    return ""; // 序列化失败,返回空字符串
  }
  bool unserialize(const std::string &msg) override { // 反序列化
    return JSON::unserialize(msg, _body);
  }

protected:
  Json::Value _body;
}; // namespace bitrpc
class JsonRequest : public JsonMessage {
public:
  using ptr = std::shared_ptr<JsonRequest>;
};
class JsonResponse : public JsonMessage {
public:
  using ptr = std::shared_ptr<JsonResponse>;
  virtual bool check() override {
    // 在响应中，大部分的响应都只有响应状态码
    // 所以只要判断响应状态码是否存在，以及是否为整数即可
    if (_body[KEY_RCODE].isNull() == true) {
      ELOG("响应状态码不存在");
      return false;
    }
    if (_body[KEY_RCODE].isIntegral() == false) {
      ELOG("响应状态码不是整数");
      return false;
    }
    return true;
  }
  virtual RCode rcode() { return (RCode)_body[KEY_RCODE].asInt(); }
  virtual void setRCode(RCode rcode) { _body[KEY_RCODE] = (int)rcode; }
};
class RpcRequest : public JsonRequest { // 给出请求方法和请求参数
public:
  using ptr = std::shared_ptr<RpcRequest>;
  virtual bool check() override {
    // 包含请求方法和请求参数
    if (_body[KEY_METHOD].isNull() == true ||
        _body[KEY_METHOD].isString() == false) {
      ELOG("请求方法不存在或不是字符串类型");
      return false;
    }
    if (_body[KEY_PARAMS].isNull() == true ||
        _body[KEY_PARAMS].isObject() == false) {
      ELOG("请求参数不存在或不是对象类型");
      return false;
    }
    return true;
  }
  // 请求方法的获取和设置
  std::string method() { return _body[KEY_METHOD].asString(); }
  void setMethod(const std::string &method) { _body[KEY_METHOD] = method; }
  // 请求参数的获取和设置
  Json::Value params() { return _body[KEY_PARAMS]; }
  void setParams(const Json::Value &params) { _body[KEY_PARAMS] = params; }
};
class TopicRequest : public JsonRequest { // 给出主题名称和消息类型以及消息内容
public:
  using ptr = std::shared_ptr<TopicRequest>;
  virtual bool check() override {
    // 包含主题名称和消息类型和消息内容
    if (_body[KEY_TOPIC_KEY].isNull() == true ||
        _body[KEY_TOPIC_KEY].isString() == false) {
      ELOG("主题名称不存在或不是字符串类型");
      return false;
    }
    if (_body[KEY_OPTYPE].isNull() == true ||
        _body[KEY_OPTYPE].isIntegral() == false) {
      ELOG("消息类型不存在或不是整数");
      return false;
    }
    if (_body[KEY_OPTYPE].asInt() ==
            (int)TopicOptype::TOPIC_PUBLISH && // 消息类型为发布类型
        (_body[KEY_TOPIC_MSG].isNull() == true ||
         _body[KEY_TOPIC_MSG].isString() == false)) {
      ELOG("消息内容不存在或不是字符串类型");
      return false;
    }
    return true;
  }
  // 主题名称的获取和设置
  std::string topicKey() { return _body[KEY_TOPIC_KEY].asString(); }
  void setTopicKey(const std::string &topic_key) { _body[KEY_TOPIC_KEY] = topic_key; }
  // 消息类型的获取和设置
  TopicOptype topicOptype() {
    return (TopicOptype)_body[KEY_OPTYPE].asInt();
  } // 消息类型
  void setTopicOptype(TopicOptype optype) {
    _body[KEY_OPTYPE] = (int32_t)optype;
  }
  // 消息内容的获取和设置
  std::string topicMsg() { return _body[KEY_TOPIC_MSG].asString(); } // 消息内容
  void setTopicMsg(const std::string &msg) { _body[KEY_TOPIC_MSG] = msg; }
};
typedef std::pair<std::string, int> Address;
class ServiceRequest
    : public JsonRequest { // 给出请求方法，主机信息和服务操作类型
public:
  using ptr = std::shared_ptr<ServiceRequest>;
  // 包含方法名称method，主机信息host，操作类型optype
  virtual bool check() override {
    if (_body[KEY_METHOD].isNull() == true ||
        _body[KEY_METHOD].isString() == false) {
      ELOG("请求方法不存在或不是字符串类型");
      return false;
    }
    if (_body[KEY_OPTYPE].asInt() !=
            (int)(ServiceOptype::
                      SERVICE_DISCOVERY) && // 操作类型不是服务发现类型
        (_body[KEY_HOST].isNull() == true ||
         _body[KEY_HOST].isObject() == false ||
         _body[KEY_HOST][KEY_HOST_IP].isNull() == true ||
         _body[KEY_HOST][KEY_HOST_IP].isString() == false ||
         _body[KEY_HOST][KEY_HOST_PORT].isNull() == true ||
         _body[KEY_HOST][KEY_HOST_PORT].isIntegral() == false)) {
      ELOG("服务请求中主机信息不存在或类型错误");
      return false;
    }
    if (_body[KEY_OPTYPE].isNull() == true ||
        _body[KEY_OPTYPE].isIntegral() == false) {
      ELOG("操作类型不存在或不是整数");
      return false;
    }
    return true;
  }
  // 请求方法的获取和设置
  std::string method() { return _body[KEY_METHOD].asString(); }
  void setMethod(const std::string &method) { _body[KEY_METHOD] = method; }
  // 主机信息的获取和设置
  Address host() {
    Address addr;
    addr.first = _body[KEY_HOST][KEY_HOST_IP].asString();
    addr.second = _body[KEY_HOST][KEY_HOST_PORT].asInt();
    return addr;
  }
  void setHost(const Address &host) {
    Json::Value val; // 因为主机信息是一个对象，所以需要创建一个新的对象
    val[KEY_HOST_IP] = host.first;
    val[KEY_HOST_PORT] = host.second;
    _body[KEY_HOST] = val;
  }
  // 操作类型的获取和设置
  ServiceOptype optype() { return (ServiceOptype)_body[KEY_OPTYPE].asInt(); }
  void setOptype(ServiceOptype optype) { _body[KEY_OPTYPE] = (int32_t)optype; }
};

class RpcResponse
    : public JsonResponse { // 返回请求状态（成功：ok，失败：....）以及结果
public:
  using ptr = std::shared_ptr<RpcResponse>;
  virtual bool check() override {
    // 包含响应结果result
    if (_body[KEY_RCODE].isNull() == true ||
        _body[KEY_RCODE].isIntegral() == false) {
      ELOG("响应码不存在或不是整数类型");
      return false;
    }
    if (_body[KEY_RESULT].isNull() == true) {
      ELOG("响应结果不存在"); // 结果可能是各种类型的，int
                              // ，string，甚至是一个对象，所以这里不做限制
      return false;
    }
    return true;
  }
  // 响应结果的获取和设置
  Json::Value result() { return _body[KEY_RESULT]; }
  void setResult(const Json::Value &result) { _body[KEY_RESULT] = result; }
};
class TopicResponse
    : public JsonResponse { // 返回操作结果（成功：ok，失败：....）
public:
  using ptr = std::shared_ptr<TopicResponse>;
};

class ServiceResponse : public JsonResponse {
public:
  using ptr = std::shared_ptr<ServiceResponse>;
  virtual bool check() override {
    // 包含响应结果result
    if (_body[KEY_RCODE].isNull() == true ||
        _body[KEY_RCODE].isIntegral() == false) {
      ELOG("响应码不存在或不是整数类型");
      return false;
    }
    if (_body[KEY_OPTYPE].isNull() == true ||
        _body[KEY_OPTYPE].isIntegral() == false) {
      ELOG("操作类型不存在或不是整数类型");
      return false;
    }
    if (_body[KEY_OPTYPE].asInt() ==
            (int)ServiceOptype::SERVICE_DISCOVERY && // 消息类型为调用类型
        (_body[KEY_METHOD].isNull() == true ||
         _body[KEY_METHOD].isString() == false ||
         _body[KEY_HOST].isNull() == true ||
         _body[KEY_HOST].isArray() == false)) {
      ELOG("主机信息不存在或不是数组类型");
      return false;
    }
    return true;
  }
  ServiceOptype optype() { return (ServiceOptype)_body[KEY_OPTYPE].asInt(); }
  void setOptype(ServiceOptype optype) { _body[KEY_OPTYPE] = (int)optype; }
  std::string method() { return _body[KEY_METHOD].asString(); }
  void setMethod(const std::string &method) { _body[KEY_METHOD] = method; }
  void setHost(std::vector<Address> addrs) { // 设置主机信息
    for (auto &addr : addrs) {
      Json::Value val; // 因为主机信息是一个对象，所以需要创建一个新的对象
      val[KEY_HOST_IP] = addr.first;
      val[KEY_HOST_PORT] = addr.second;
      _body[KEY_HOST].append(val);
    }
  }
  std::vector<Address> hosts() { // 获取某一个服务的所有主机信息
    std::vector<Address> addrs;
    int sz = _body[KEY_HOST].size();
    for (int i = 0; i < sz; i++) {
      Address addr;
      addr.first =
          _body[KEY_HOST][i][KEY_HOST_IP]
              .asString(); // 因为KEY_HOST是一个数组，所以需要遍历数组中的每个元素
      addr.second = _body[KEY_HOST][i][KEY_HOST_PORT].asInt();
      addrs.push_back(addr);
    }
    return addrs;
  }
};

// 实现消息对象的生产工厂
class MessageFactory {
public:
  static BaseMessage::ptr create(MType mtype) {
    switch (mtype) {
    case MType::REQ_RPC:
      return std::make_shared<RpcRequest>();
    case MType::RSP_RPC:
      return std::make_shared<RpcResponse>();
    case MType::REQ_TOPIC:
      return std::make_shared<TopicRequest>();
    case MType::RSP_TOPIC:
      return std::make_shared<TopicResponse>();
    case MType::REQ_SERVICE:
      return std::make_shared<ServiceRequest>();
    case MType::RSP_SERVICE:
      return std::make_shared<ServiceResponse>();
    }
    return BaseMessage::ptr();
  }

  template <typename T, typename... Args>
  static std::shared_ptr<T> create(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
};
} // namespace bitrpc