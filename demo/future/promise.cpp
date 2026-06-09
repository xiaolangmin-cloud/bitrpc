#include <iostream>
#include <future>

int Add(int num1, int num2)
{
    std::cout << "ok" << std::endl;
    return num1 + num2;
}

int main()
{
    // 先创建个promise对象
    std::promise<int> pro;
    // 然后关联一个future对象
    std::future<int> res = pro.get_future();
    // 创建一个线程来执行任务
    std::thread thr([&pro]()
                    {
                        int num1 = Add(1, 2);
                        pro.set_value(num1); // 把结果放到promise对象中
                    });
    std::cout << res.get() << std::endl; // 获取结果
    thr.join();                          // 等待线程结束

    return 0;
}