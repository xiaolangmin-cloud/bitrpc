#pragma once
#include <string>
#include <unordered_map>
//各种枚举常量
namespace bitrpc {
#define KEY_METHOD "method"       // 方法名称
#define KEY_PARAMS "parameters"   // 方法参数
#define KEY_TOPIC_KEY "topic_key" // 主题名称
#define KEY_TOPIC_MSG "topic_msg" // 主题消息
#define KEY_OPTYPE "optype"       // 操作类型
#define KEY_HOST "host"           // 主机信息
#define KEY_HOST_IP "ip"          // 主机IP
#define KEY_HOST_PORT "port"      // 主机端口
#define KEY_RCODE "rcode"         // 错误码
#define KEY_RESULT "result"       // 结果

enum class MType { // 消息类型
  REQ_RPC = 0,     // Rpc请求
  RSP_RPC,         // Rpc响应
  REQ_TOPIC,       // 主题请求
  RSP_TOPIC,       // 主题响应
  REQ_SERVICE,     // 请求服务
  RSP_SERVICE      // 服务响应
};

enum class RCode {         // 错误码
  RCODE_OK = 0,            // 成功处理
  RCODE_PARSE_FAILED,      // 消息解析失败
  RCODE_ERROR_MSGTYPE,     // 消息类型错误
  RCODE_INVALID_MSG,       // 无效消息
  RCODE_DISCONNECTED,      // 连接已断开
  RCODE_INVALID_PARAMS,    // 无效的Rpc参数
  RCODE_NOT_FOUND_SERVICE, // 没有找到对应的服务
  RCODE_INVALID_OPTYPE,    // 无效的操作类型
  RCODE_NOT_FOUND_TOPIC,   // 没有找到对应的主题
  RCODE_INTERNAL_ERROR     // 内部错误
};
static std::string errReason(RCode code) {
  static std::unordered_map<RCode, std::string> err_map = {
      {RCode::RCODE_OK, "成功处理！"},
      {RCode::RCODE_PARSE_FAILED, "消息解析失败！"},
      {RCode::RCODE_ERROR_MSGTYPE, "消息类型错误！"},
      {RCode::RCODE_INVALID_MSG, "无效消息"},
      {RCode::RCODE_DISCONNECTED, "连接已断开！"},
      {RCode::RCODE_INVALID_PARAMS, "无效的Rpc参数！"},
      {RCode::RCODE_NOT_FOUND_SERVICE, "没有找到对应的服务！"},
      {RCode::RCODE_INVALID_OPTYPE, "无效的操作类型"},
      {RCode::RCODE_NOT_FOUND_TOPIC, "没有找到对应的主题！"},
      {RCode::RCODE_INTERNAL_ERROR, "内部错误！"}};
  auto it = err_map.find(code);
  if (it == err_map.end()) {
    return "未知错误！";
  }
  return it->second;
}

enum class RType { REQ_ASYNC = 0, REQ_CALLBACK };

enum class TopicOptype { // 主题操作类型
  TOPIC_CREATE = 0,      // 创建主题
  TOPIC_REMOVE,          // 删除主题
  TOPIC_SUBSCRIBE,       // 订阅主题
  TOPIC_CANCEL,          // 取消订阅主题
  TOPIC_PUBLISH          // 发布主题
};

enum class ServiceOptype { // 服务操作类型
  SERVICE_REGISTRY = 0,    // 注册服务
  SERVICE_DISCOVERY,       // 发现服务
  SERVICE_ONLINE,          // 服务上线
  SERVICE_OFFLINE,         // 服务下线
  SERVICE_UNKNOW           // 未知服务
};
} // namespace bitrpc