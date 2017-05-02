// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PriorityInversion.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/RecMutex.h>

#include <sstream>
#include <TestCommon.h>
#include <vector>
#include <map>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace IceUtil;

class TaskCollector : public IceUtil::Shared
{
public:

    TaskCollector(int cores, int high, int medium, int low, Monitor<Mutex>& monitor) :
        _lowBegin(0),
        _lowEnd(0),
        _mediumBegin(0),
        _mediumEnd(0),
        _highBegin(0),
        _cores(cores),
        _high(high),
        _medium(medium),
        _low(low),
        _acquired(0),
        _monitor(monitor)
    {
    }

    void waitAcquired()
    {
        Monitor<Mutex>::Lock lock(_monitor);
        while(_acquired == 0)
        {
            _monitor.wait();
        }
    }

    void acquired()
    {
        Monitor<Mutex>::Lock lock(_monitor);
        ++_acquired;
        _monitor.notifyAll();
    }

    void waitAll()
    {
        Monitor<Mutex>::Lock lock(_monitor);
        while(_mediumBegin < _cores || _highBegin  == 0)
        {
            //Wait until all task are ready to compete by processors
            _monitor.wait();
        }
    }

    void taskBegin(int priority)
    {
        Monitor<Mutex>::Lock lock(_monitor);
        if(priority == _low)
        {
            _lowBegin++;
        }
        else if(priority == _medium)
        {
            _mediumBegin++;
        }
        else if(priority == _high)
        {
            _highBegin++;
        }
        _monitor.notifyAll();
    }

    void taskEnd(int priority)
    {
        Monitor<Mutex>::Lock lock(_monitor);
        //
        // Test all task begin run before any task ends.
        //
        test(_lowBegin == 1);
        test(_highBegin == 1);
        test(_mediumBegin == _cores);
        if(priority == _low)
        {
            //
            // Low priority thread should end before all medium priority threads.
            //
            test(_mediumEnd == 0);
            _lowEnd++;
        }
        else if(priority == _medium)
        {
            //
            // When the first medium priority task end the
            // low priority task completed.
            //
            test(_lowEnd > 0);
            _mediumEnd++;
        }
    }

private:

    int _lowBegin;
    int _lowEnd;
    int _mediumBegin;
    int _mediumEnd;
    int _highBegin;
    int _cores;
    int _high;
    int _medium;
    int _low;
    int _acquired;
    Monitor<Mutex>& _monitor;
    IceUtil::Mutex _mutex;
};
typedef IceUtil::Handle<TaskCollector> TaskCollectorPtr;


class SharedResource : public IceUtil::Shared
{
public:

    SharedResource(const TaskCollectorPtr& taskCollector) :
        _taskCollector(taskCollector)
    {
    }

    TaskCollectorPtr taskCollector() const { return _taskCollector; }

    virtual void run(int priority) = 0;

private:

    TaskCollectorPtr _taskCollector;
};
typedef IceUtil::Handle<SharedResource> SharedResourcePtr;

class SharedResourceMutex : public SharedResource
{
public:

    SharedResourceMutex(const TaskCollectorPtr& taskCollector) :
        SharedResource(taskCollector)
    {
    }

    virtual void run(int priority)
    {
        taskCollector()->taskBegin(priority);
        Mutex::Lock lock(_mutex);
        taskCollector()->acquired();
        taskCollector()->waitAll();
        //
        // If this is the low priority thread we ensure the test runs at least timeout
        // seconds this ensure that it doesn't terminate righ away and medium priority
        // threads will take over all available cores
        //
        IceUtil::Time t = IceUtil::Time::now(IceUtil::Time::Monotonic);
        IceUtil::Time timeout = IceUtil::Time::seconds(2);
        if(priority == 1)
        {
            while(true)
            {
                if(IceUtil::Time::now(IceUtil::Time::Monotonic) - t > timeout)
                {
                    break;
                }
            }
        }
        taskCollector()->taskEnd(priority);
    }

private:

    IceUtil::Mutex _mutex;
};


class SharedResourceRecMutex : public SharedResource
{
public:

    SharedResourceRecMutex(const TaskCollectorPtr& taskCollector) :
        SharedResource(taskCollector)
    {
    }

    void run(int priority)
    {
        taskCollector()->taskBegin(priority);
        RecMutex::Lock lock(_mutex);
        taskCollector()->acquired();
        taskCollector()->waitAll();
        //
        // If this is the low priority thread we ensure the test runs at least timeout
        // seconds this ensure that it doesn't terminate righ away and medium priority
        // threads will take over all available cores
        //
        IceUtil::Time t = IceUtil::Time::now(IceUtil::Time::Monotonic);
        IceUtil::Time timeout = IceUtil::Time::seconds(2);
        if(priority == 1)
        {
            while(true)
            {
                if(IceUtil::Time::now(IceUtil::Time::Monotonic) - t > timeout)
                {
                    break;
                }
            }
        }
        taskCollector()->taskEnd(priority);
    }

private:

    IceUtil::RecMutex _mutex;
};

class ThreadCommon : public IceUtil::Thread
{
public:

    virtual void run() = 0;
    int getPriority()
    {
#ifdef _WIN32_WCE
        return CeGetThreadPriority(GetCurrentThread());
#elif defined _WIN32
        return GetThreadPriority(GetCurrentThread());
#else
        sched_param param;
        int sched_policy;
        pthread_t thread = pthread_self();
        pthread_getschedparam(thread, &sched_policy, &param);
        return param.sched_priority;
#endif
    }
};

class Task : public ThreadCommon
{
public:

    Task(const SharedResourcePtr& shared) :
        _shared(shared)
    {
    }

    virtual void run()
    {
        _shared->run(getPriority());
    }

    void waitAcquired()
    {
        _shared->taskCollector()->waitAcquired();
    }

private:

    SharedResourcePtr _shared;
};
typedef IceUtil::Handle<Task> TaskPtr;

class MediumPriorityThread : public ThreadCommon
{
public:

    MediumPriorityThread(const TaskCollectorPtr& taskCollector, const ThreadPtr& highPriorityThread, int timeout) :
        _taskCollector(taskCollector),
        _highPriorityThread(highPriorityThread),
        _timeout(IceUtil::Time::seconds(timeout))
    {
    }

    virtual void run()
    {
        IceUtil::Time timestamp = IceUtil::Time::now(IceUtil::Time::Monotonic);
        _taskCollector->taskBegin(getPriority());
        while(true)
        {
            if(IceUtil::Time::now(IceUtil::Time::Monotonic) - timestamp > _timeout)
            {
                // If high priority task do not end with the specific timeout means
                // that the low priority task priority was not bosted so we are having
                // the clasic priority inversion issue.
                test(false);
            }
            if(!_highPriorityThread->isAlive())
            {
                break;
            }
        }
        _taskCollector->taskEnd(getPriority());
    }

private:

    const TaskCollectorPtr _taskCollector;
    const ThreadPtr _highPriorityThread;
    const IceUtil::Time _timeout;
};

static const string priorityTestName("priority inversion");

PriorityInversionTest::PriorityInversionTest() :
    TestBase(priorityTestName)
{
}

void
PriorityInversionTest::run()
{
    int cores, high, medium, low, timeout;
    timeout = 30;
#ifdef _WIN32
    return; //Priority inversion is not supported by WIN32
#else
    try
    {
        IceUtil::Mutex m;
    }
    catch(const IceUtil::ThreadSyscallException&)
    {
        return; // Mutex protocol PrioInherit not supported
    }
    cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
    high = 45;
    medium = 35;
    low = 1;
#endif

    {
        Monitor<Mutex> monitor;
        TaskCollectorPtr collector = new TaskCollector(cores, high, medium, low, monitor);
        vector<ThreadControl> threads;

        SharedResourcePtr shared = new SharedResourceMutex(collector);

        //
        // Create one low priority thread.
        //
        TaskPtr lowThread = new Task(shared);
        threads.push_back(lowThread->start(128, low));
        lowThread->waitAcquired();

        //
        // Create one high priority thread that use the same shared resource
        // as the previous low priority thread
        //
        TaskPtr highThread = new Task(shared);
        threads.push_back(highThread->start(128, high));

        //
        // Create one medium priority thread per core.
        //
        for(int cont = 0; cont < cores; ++cont)
        {
            ThreadPtr t = new MediumPriorityThread(collector, highThread, timeout);
            threads.push_back(t->start(128, medium));
        }
        test(lowThread->isAlive());
        test(highThread->isAlive());
        //
        // Join with all the threads.
        //
        vector<ThreadControl>::iterator it;
        for(it = threads.begin(); it != threads.end(); ++it)
        {
            try
            {
                (*it).join();
            }
            catch(...)
            {
            }
        }
    }

    //
    // Same test with a recursive mutex.
    //
    {
        Monitor<Mutex> monitor;
        TaskCollectorPtr collector = new TaskCollector(cores, high, medium, low, monitor);

        SharedResourcePtr shared = new SharedResourceRecMutex(collector);

        vector<ThreadControl> threads;

        //
        // Create one low priority thread.
        //
        TaskPtr lowThread = new Task(shared);
        threads.push_back(lowThread->start(128, low));
        lowThread->waitAcquired();

        //
        // Create one high priority thread that use the same shared resource
        // as the previous low priority thread.
        //
        ThreadPtr highThread = new Task(shared);
        threads.push_back(highThread->start(128, high));

        //
        // Create one medium priority tasks per core that runs until
        // the high priority thread is running.
        //
        for(int cont = 0; cont < cores; ++cont)
        {
            ThreadPtr t = new MediumPriorityThread(collector, highThread, timeout);
            threads.push_back(t->start(128, medium));
        }
        test(lowThread->isAlive());
        test(highThread->isAlive());

        //
        // Join with all the threads.
        //
        vector<ThreadControl>::iterator it;
        for(it = threads.begin(); it != threads.end(); ++it)
        {
            try
            {
                (*it).join();
            }
            catch(...)
            {
            }
        }
    }
}
