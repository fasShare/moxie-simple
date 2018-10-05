#include <functional>
#include <iostream>
#include <EventLoopThread.h>

using namespace moxie;
using namespace std;

void test(EventLoop *loop, int i) {
    std::cout << "i = " << i << std::endl;
}

int main () {
    EventLoopThread thread;

    thread.Start();
    for (int i = 0; i < 20; ++i) {
        thread.PushTask(std::bind(test, std::placeholders::_1, i));
    }
    
    sleep(2);
    thread.Stop();
    std::cout << "--------end--------" << std::endl;
    sleep(2);
}