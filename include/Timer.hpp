#pragma once
#include <ctime>

class Timer
{
    timespec _st, _ft;

  public:
    Timer(void);
    Timer(Timer const& rhs);

    Timer& operator=(const Timer& rhs);

    void start(void);
    void finish(void);

    double getElapsed(void) const; // expressed in ms
};
