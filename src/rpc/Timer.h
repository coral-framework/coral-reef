#include <windows.h>

class Timer
{
public:
	Timer();

	virtual ~Timer();

	double elapsed();

private:
    LARGE_INTEGER _lastTime;

    static double _counterFrequency;
};
