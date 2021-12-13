#pragma once
#include <ctime>

class Timer
{
    timespec _st;

  public:
    Timer(void);
    Timer(Timer const& rhs);

    Timer& operator=(const Timer& rhs);

    void reset(void);

    double getElapsed(void) const; // expressed in ms
};
