#include <iostream>
#include <jsoncpp/json/json.h>
#include <memory>
#include <string>

// 实现json的序列化，将json::value（json串）实例化的结果放进str中
// 返回值：true表示成功，false表示失败
bool serialization(const Json::Value &value, std::string &str) {
  std::stringstream ss;
  // 先用工厂类创建一个对象出来
  Json::StreamWriterBuilder swb;
  swb["emitUTF8"] =
      true; // 这个是为了让json串中的中文能够正常显示，而不是Unicode 表示法
  std::unique_ptr<Json::StreamWriter> sw(
      swb.newStreamWriter()); // 用智能指针是因为这个sw是new出来的
                              // 的对象，使用智能指针可以自动释放内存，避免内存泄漏
  // 将json串实例化写入到str中
  int ret = sw->write(value, &ss);
  if (ret !=
      0) // 这里是因为这个write无论正常或者错误都会返回0，一般出错不是因为它而是因为我们的输出流：ss或者josn串的问题
  {
    std::cout << "json串实例化失败!" << std::endl;
    return false;
  }
  str = ss.str();
  return true;
}
bool unserialization(const std::string &str, Json::Value &value) {
  // 创建用于接收反序列化后的字符串的字符串
  std::string errs;
  // 同样要做到反序列化就要先创建工厂类
  Json::CharReaderBuilder crb;
  // 创建一个反序列化对象
  std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
  int ret = cr->parse(
      str.c_str(), str.c_str() + str.size(), &value,
      &errs); // parse接收的是一个字符串的开始地址和结束地址，然后将反序列化后的给value，将反序列化后的字符串给到errs
  if (!ret) {
    std::cout << "json串反序列化失败!" << std::endl;
    return false;
  }
  return true;
}
int main() {
  // 构建一个json串
  Json::Value valude;
  valude["姓名"] = "张三";
  valude["年龄"] = 20;
  valude["性别"] = "男";

  int num[3] = {88, 79, 90};
  valude["成绩"].append(num[0]);
  valude["成绩"].append(num[1]);
  valude["成绩"].append(num[2]);
  Json::Value value1;
  value1["运动"].append("篮球");
  value1["运动"].append("足球");
  value1["书籍"] = "python";
  valude["爱好"] = value1;
  std::string str;
  serialization(valude, str);
  std::cout << str << std::endl;

  unserialization(str, valude);
  std::cout << valude["姓名"].asString() << std::endl;
  std::cout << valude["年龄"].asInt() << std::endl;
  std::cout << valude["性别"].asString() << std::endl;
  std::cout << valude["成绩"][0].asInt() << std::endl;
  std::cout << valude["成绩"][1].asInt() << std::endl;
  std::cout << valude["成绩"][2].asInt() << std::endl;
  std::cout << valude["爱好"]["运动"][0].asString() << std::endl;
  std::cout << valude["爱好"]["运动"][1].asString() << std::endl;
  std::cout << valude["爱好"]["书籍"].asString() << std::endl;
  return 0;
}
