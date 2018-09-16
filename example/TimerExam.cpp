#include <iostream>
#include <atomic>

#include <EventLoop.h>
#include <string.h>
#include <Log.h>
#include <Timer.h>

using namespace moxie;

Timer *timer1, *timer2, *timer3;

int count = 0;
void PrintCountClear(moxie::Timer *timer, moxie::EventLoop *loop) {
    std::cout << "Count = " << count++ << std::endl;
    if (count >= 10) {
        loop->UnregisterTimer(timer);
    }
}

void DelaSecondOnce(moxie::Timer *timer, moxie::EventLoop *loop) {
    std::cout << "DelaSecondOnce" << std::endl;
}

int three = 0;
void DelaSecondThree(moxie::Timer *timer, moxie::EventLoop *loop) {
    if (three++ >= 3) {
        std::cout << "DelaSecondThree will unregistered!" << std::endl;
        loop->UnregisterTimer(timer);
    }
    std::cout << "DelaSecondThree" << std::endl;
    timer->Reset(moxie::AddTime(Timestamp::Now(), 1));
    //loop->UnregisterTimer(timer1);
}

int main() {
    EventLoop *loop = new EventLoop();

    timer1 = new Timer(PrintCountClear, moxie::AddTime(Timestamp::Now(), 1), 1);
    timer2 = new Timer(DelaSecondOnce, moxie::AddTime(Timestamp::Now(), 1));
    timer3 = new Timer(DelaSecondThree, moxie::AddTime(Timestamp::Now(), 1));

    loop->RegisterTimer(timer1);
    loop->RegisterTimer(timer2);
    loop->RegisterTimer(timer3);

    loop->Loop();

    delete loop;
    return 0;
}
