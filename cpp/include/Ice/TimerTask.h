// Copyright (c) ZeroC, Inc.

#ifndef ICE_TIMER_TASK_H
#define ICE_TIMER_TASK_H

#include <memory>

namespace IceInternal
{
    class Timer;
    using TimerPtr = std::shared_ptr<Timer>;

    class ICE_API TimerTask
    {
    public:
        virtual ~TimerTask();

        virtual void runTimerTask() = 0;
    };

    using TimerTaskPtr = std::shared_ptr<TimerTask>;
}

#endif
