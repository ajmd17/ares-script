#ifndef TIMER_H
#define TIMER_H

#ifdef WIN32
#define WIN32_TIMER 1
#endif

#if WIN32_TIMER
#include <tchar.h>
#include <windows.h>
#else
#include <time.h>
#endif

namespace avm {
class Timer {
public:
  void start() {
#if WIN32_TIMER
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
      throw "QueryPerformanceFrequency failed!";

    PCFreq = double(li.QuadPart);

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
#else
    beginning = clock();
#endif
  }

  double elapsed() const {
#if WIN32_TIMER
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - CounterStart) / PCFreq;
#else
    return ((double)(clock() - beginning)) / CLOCKS_PER_SEC;
#endif
  }

private:
#if WIN32_TIMER
  double PCFreq = 0.0;
  __int64 CounterStart = 0;
#else
  unsigned long beginning;
#endif
};
} // namespace avm

#endif