#include <iostream>
#include <future>

int Add(int num1, int num2)
{
    std::cout << "ok" << std::endl;
    return num1 + num2;
}

int main()
{
    //std::launch::async策略：内部创建一个线程执行函数，函数的结果通过future获取,而且他本身回返回一个future对象
    //std::launch::deferred策略:函数调用被延迟到wait或get被调用时才执行，函数在调用线程中同步执行
   // std::future<int> res=std::async(std::launch::deferred, Add, 1, 2);//用future获取执行结果,这里进行了一个异步非阻塞调用
    std::future<int> res=std::async(std::launch::async, Add, 1, 2);//用future获取执行结果,这里进行了一个异步非阻塞调用
   // std::this_thread::sleep_for(std::chrono::seconds(1));//命当前进程休眠1秒
    std::cout << res.get() << std::endl;//获取结果
    return 0;
}