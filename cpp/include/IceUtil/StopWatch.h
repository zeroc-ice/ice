//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_STOPWATCH_H
#define ICE_UTIL_STOPWATCH_H

#include <IceUtil/Config.h>

#include <chrono>

namespace IceUtilInternal
{
    class StopWatch
    {
    public:
        StopWatch() {}

        void start() { _start = std::chrono::steady_clock::now(); }

        std::chrono::microseconds stop()
        {
            assert(isStarted());
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _start);
            _start = std::chrono::steady_clock::time_point();
            return duration;
        }

        bool isStarted() const { return _start != std::chrono::steady_clock::time_point(); }

        std::chrono::microseconds delay()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _start);
        }

    private:
        std::chrono::steady_clock::time_point _start;
    };

} // End namespace IceUtilInternal

#endif
