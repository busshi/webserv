#include "Timer.hpp"
#include <ctime>

/* Inspired from
 * https://stackoverflow.com/questions/2962785/c-using-clock-to-measure-time-in-multi-threaded-programs/2962914#2962914
 */

Timer::Timer(void)
{
    clock_gettime(CLOCK_MONOTONIC, &_st);
}

Timer::Timer(const Timer& rhs)
{
    *this = rhs;
}

Timer&
Timer::operator=(const Timer& rhs)
{
    if (this != &rhs) {
        _st = rhs._st;
    }

    return *this;
}

void
Timer::reset(void)
{
    clock_gettime(CLOCK_MONOTONIC, &_st);
}

double
Timer::getElapsed(void) const
{
    timespec cur;

    clock_gettime(CLOCK_MONOTONIC, &cur);

    double elapsed = (cur.tv_sec - _st.tv_sec) * 1e+3; /* seconds in ms */

    elapsed += (cur.tv_nsec - _st.tv_nsec) / 1e+6; /* nanoseconds in ms */

    return elapsed;
}
