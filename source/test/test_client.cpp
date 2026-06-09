#include "../client/requestor.hpp"
#include "../client/rpc_caller.hpp"
#include "../common/dispatcher.hpp"
#include "../common/message.hpp"
#include "../common/net.hpp"
#include "../common/detail.hpp"

void callback(const Json::Value &result) {
  std::cout << "Add result3:" << result << std::endl;
}

int main() {

  auto requestor = std::make_shared<bitrpc::client::Requestor>();
  auto callee = std::make_shared<bitrpc::client::RpcCaller>(requestor);

  auto dispatcher = std::make_shared<bitrpc::Dispatcher>();
  auto rsp_cb =
      std::bind(&bitrpc::client::Requestor::onResponse, requestor.get(),
                std::placeholders::_1, std::placeholders::_2);
  dispatcher->registerHandler<bitrpc::BaseMessage>(bitrpc::MType::RSP_RPC,
                                                   rsp_cb);

  auto message_cb = std::bind(&bitrpc::Dispatcher::onMessage, dispatcher.get(),
                              std::placeholders::_1, std::placeholders::_2);

  auto client = bitrpc::ClientFactory::create("127.0.0.1", 9090);

  client->setMessageCallback(message_cb);
  client->connect();

  auto conn = client->connection();
  Json::Value params, result;
  params["num1"] = 1;
  params["num2"] = 2;
  bool ret = callee->call(conn, "Add", params, result);
  if (ret) {
    std::cout << "Add result1:" << result << std::endl;
  }

  bitrpc::client::RpcCaller::JsonAsyncResponse res_future;
  params["num1"] = 3;
  params["num2"] = 4;
  ret = callee->call(conn, "Add", params, res_future);
  if (ret) {
    result = res_future.get();
    std::cout << "Add result2:" << result << std::endl;
  }

  params["num1"] = 5;
  params["num2"] = 6;
  ret = callee->call(conn, "Add", params, callback);
  

  client->shutdown();
  return 0;
}