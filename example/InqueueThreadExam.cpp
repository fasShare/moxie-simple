#include <functional>
#include <iostream>
#include <InqueueThread.h>

using namespace moxie;
using namespace std;

void test(int i) {
    std::cout << "i = " << i << std::endl;
}

int main () {
    auto thread = InqueueThread::CreateInqueueThread();

    thread->Start();
    for (int i = 0; i < 20; ++i) {
        if (i % 2 == 0) {
            thread->InqueueTaskWithDelayTime(std::bind(test, i), 500);
            continue;
        }
        thread->InqueueTask(std::bind(test, i));
    }
    
    sleep(2);
    thread->Stop();
    sleep(1);
    std::cout << "--------end--------" << std::endl;
}