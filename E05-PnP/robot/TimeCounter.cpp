#include "TimeCounter.h"

TimeCounter::TimeCounter() {

}

TimeCounter::TimeCounter(int time) {
    timeOut = time;
}

void TimeCounter::MarkStartPoint() {
    timeStartPoint = high_resolution_clock::now();
}

void TimeCounter::SetTimeOut(int time) {
    timeOut = time;
}

bool TimeCounter::TimeOutChecking() {
    auto currentTimePoint = high_resolution_clock::now();
    int durationTime = duration_cast<milliseconds>(currentTimePoint - timeStartPoint).count();
    if(durationTime >= timeOut) {
        return true;
    }
    return false;
}

bool TimeCounter::TimeOutCheckingCylic() {
    auto currentTimePoint = high_resolution_clock::now();
    int durationTime = duration_cast<milliseconds>(currentTimePoint - timeStartPoint).count();
    if(durationTime >= timeOut) {
        timeStartPoint = high_resolution_clock::now();
        return true;
    }
    return false;
}
