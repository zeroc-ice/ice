// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <ThreadPriority.h>
#include <TestHelper.h>

using namespace std;
using namespace IceUtil;

class PriorityTestThread : public Thread
{
public:

    PriorityTestThread() :
        _priority(-1024) //Initialize to some strange value, so by default is not a valid priority
    {
    }

    virtual void run()
    {
#ifdef _WIN32
        _priority = GetThreadPriority(GetCurrentThread());
#else
        sched_param param;
        int sched_policy;
        pthread_t thread = pthread_self();
        pthread_getschedparam(thread, &sched_policy, &param);
        _priority = param.sched_priority;
#endif
    }

    int getPriority()
    {
        return _priority;
    }

private:

    int _priority;
};

typedef Handle<PriorityTestThread> PriorityTestThreadPtr;

static const string priorityTestName("priority");

ThreadPriorityTest::ThreadPriorityTest() :
    TestBase(priorityTestName)
{
#ifdef _WIN32
    ThreadControl c;
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_IDLE);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_IDLE);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_LOWEST);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_LOWEST);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_BELOW_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_BELOW_NORMAL);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_NORMAL);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_ABOVE_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_ABOVE_NORMAL);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_HIGHEST);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_HIGHEST);

        t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_TIME_CRITICAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_TIME_CRITICAL);
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set invalid priorities too high
    //
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_TIME_CRITICAL + 10);
        test(false);
    }
    catch(const ThreadSyscallException&)
    {
        //Expected
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set invalid priorities too low
    //
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_IDLE - 10);
        test(false);
    }
    catch(const ThreadSyscallException&)
    {
        //Expected
    }
    catch(...)
    {
        test(false);
    }

#else

    ThreadControl c;
    try
    {
        for(int cont = 1; cont < 10; ++cont)
        {
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, cont);
            c.join();
            test(t1->getPriority() == cont);
        }
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set invalid priorities too high
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, 300 * cont);
            test(false);
        }
        catch(const ThreadSyscallException& e)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }

    //
    // Test to set invalid priorities too low
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, -10 * cont);
            test(false);
        }
        catch(const ThreadSyscallException& e)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }

#endif
}

void
ThreadPriorityTest::run()
{
}
