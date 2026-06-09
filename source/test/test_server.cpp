#include "../common/message.hpp"
#include "../common/net.hpp"
#include "../server/rpc_router.hpp"
#include "../common/dispatcher.hpp"
void Add(const Json::Value &req, Json::Value &rsp) {
  int num1 = req["num1"].asInt();
  int num2 = req["num2"].asInt();
  rsp = num1 + num2;
}

int main() {
  auto router = std::make_shared<bitrpc::server::RpcRouter>();
  std::unique_ptr<bitrpc::server::SDescribeFactory> desc_factory(
      new bitrpc::server::SDescribeFactory());
  desc_factory->setMethodName("Add");
  desc_factory->setParamsDesc(
      "num1", bitrpc::server::VType::INTEGRAL); // 设置第一个参数为整数
  desc_factory->setParamsDesc(
      "num2", bitrpc::server::VType::INTEGRAL); // 设置第二个参数为整数
  desc_factory->setReturnType(
      bitrpc::server::VType::INTEGRAL);          // 设置返回值为整数
  desc_factory->setCallback(Add);                // 设置回调函数为Add
  router->registerMethod(desc_factory->build()); // 注册方法

  auto cb = std::bind(&bitrpc::server::RpcRouter::onRpcRequest, router.get(),
                      std::placeholders::_1,
                      std::placeholders::_2); // 绑定回调函数为onRpcRequest

  auto dispatcher = std::make_shared<bitrpc::Dispatcher>(); // 创建Dispatcher
  dispatcher->registerHandler<bitrpc::RpcRequest>(
      bitrpc::MType::REQ_RPC, cb); // 注册回调函数为onRpcRequest

  auto server = bitrpc::ServerFactory::create(9090); // 创建服务器
  auto message_cb = std::bind(&bitrpc::Dispatcher::onMessage, dispatcher.get(),
                              std::placeholders::_1,
                              std::placeholders::_2); // 绑定回调函数为onMessage
  server->setMessageCallback(message_cb); // 设置消息回调函数为onMessage
  server->start();                        // 启动服务器
  return 0;
}