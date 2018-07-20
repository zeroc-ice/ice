// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <IceUtil/Timer.h>
#include <TimerPriority.h>
#include <TestHelper.h>

using namespace std;
using namespace IceUtil;

static const string priorityTestName("timer priority");

TimerPriorityTest::TimerPriorityTest() :
    TestBase(priorityTestName)
{
}

void
TimerPriorityTest::run()
{
#ifdef _WIN32
    //
    // Test to create a timer with a given priority
    //
    try
    {
        TimerPtr t = new Timer(THREAD_PRIORITY_IDLE);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_LOWEST);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_BELOW_NORMAL);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_NORMAL);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_ABOVE_NORMAL);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_HIGHEST);
        t->destroy();

        t = new Timer(THREAD_PRIORITY_TIME_CRITICAL);
        t->destroy();
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to create a timer with priorities too high
    //
    try
    {
        TimerPtr t = new Timer(THREAD_PRIORITY_TIME_CRITICAL + 10);
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
    // Test to create a timer with priorities too low
    //
    try
    {
        TimerPtr t = new Timer(THREAD_PRIORITY_IDLE - 10);
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

    //
    // Test to create a timer with a given priority
    //
    ThreadControl c;
    try
    {
        for(int cont = 1; cont < 10; ++cont)
        {
            TimerPtr t = new Timer(cont);
        }
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to create a timer with priorities too high
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            TimerPtr t = new Timer(300 * cont);
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
    // Test to create a timer with priorities too low
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            TimerPtr t = new Timer(-10 * cont);
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
