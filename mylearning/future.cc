#include <iostream>
#include <future>

// 异步任务，计算并返回斐波那契数列的第n项
int fibonacci(int n) {
    if(n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int main() {
    // 启动异步任务
    std::future<int> fut = std::async(fibonacci, 10);

    // 在异步任务运行的同时，我们可以在主线程中做其他事情
    std::cout << "Doing some work in main thread...\n";

    // 获取异步任务的结果。如果结果还没有准备好，这会阻塞
    int result = fut.get();

    std::cout << "The 10th Fibonacci number is " << result << "\n";

    return 0;
}
/*
    cd ./mylearning
    g++ -o future future.cc -std=c++11 -lpthread
    ./future
*/