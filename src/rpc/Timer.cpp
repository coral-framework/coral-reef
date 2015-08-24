#include "Timer.h"

double Timer::_counterFrequency = 0.0;

Timer::Timer()
{
    QueryPerformanceCounter (&_lastTime);

    if (_counterFrequency == 0LL)
    {
        LARGE_INTEGER temp;

        QueryPerformanceFrequency (&temp);

        _counterFrequency = static_cast<double>(temp.QuadPart);
    }
}

Timer::~Timer()
{
    // empty destructor
}

double Timer::elapsed()
{
    LARGE_INTEGER currentTime;

    QueryPerformanceCounter (&currentTime);

    long long delta = currentTime.QuadPart - _lastTime.QuadPart;

    _lastTime = currentTime;

    double ret = ( delta / _counterFrequency );

    return ret;
}


