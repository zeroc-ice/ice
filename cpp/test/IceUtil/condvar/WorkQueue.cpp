// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/StaticMutex.h>
#include <IceUtil/Options.h>

#include <iostream>
#include <list>

using namespace std;
using namespace IceUtil;

class Queue: public Monitor<Mutex>, public Shared
{
public:

    Queue(bool broadcast) :
        _broadcast(broadcast), _terminate(false)
    {
    }

    void
    put(const int& item)
    {
        Monitor<Mutex>::Lock lock(*this);
        _q.push_back(item);
        if(_broadcast)
        {
            notifyAll();
        }
        else
        {
            notify();
        }
    }

    void
    terminate()
    {
        Monitor<Mutex>::Lock lock(*this);
        _terminate = true;
        if(_broadcast)
        {
            notifyAll();
        }
        else
        {
            notify();
        }
    }

    bool
    timedGet(int& ret, const Time& timeout)
    {
        Monitor<Mutex>::Lock lock(*this);
        if(_q.empty())
        {
            timedWait(timeout);
        }

        // We only report the termination sentinel when the queue is
        // empty.
        if(_q.empty())
        {
            if(_terminate)
            {
                ret = -1;
            }
            return false;
        }

        assert(!_q.empty());
        ret = _q.front();
        _q.pop_front();
        return true;
    }

private:
    const bool _broadcast;
    bool _terminate;
    list<int> _q;
};
typedef Handle<Queue> QueuePtr;

static IceUtil::StaticMutex coutMutex = ICE_STATIC_MUTEX_INITIALIZER;

class WatchDog : public Thread, public Monitor<Mutex>
{
public:

    WatchDog(bool verbose) :
        _verbose(verbose), _terminate(false), _touches(0), _timeout(0), _overallTouches(0), _overallTimeout(0)
    {
    }


    virtual void
    run()
    {
        Monitor<Mutex>::Lock sync(*this);

        while(true)
        {
            timedWait(Time::milliSeconds(500));
            if(_terminate)
            {
                return;
            }
            if(_touches == 0)
            {
                cout << "DEADLOCK DETECTED" << endl;
                abort();
            }

            IceUtil::StaticMutex::Lock outputMutex(coutMutex);
            if(_verbose)
            {
                cout << _touches << "(" << _timeout << ") " << flush;
            }
            _overallTouches += _touches;
            _overallTimeout += _timeout;
            _touches = 0;
            _timeout = 0;
        }
    }

    void
    touch(bool timeout)
    {
        Monitor<Mutex>::Lock sync(*this);
        _touches++;
        if(timeout)
        {
            _timeout++;
        }
    }

    void
    terminate()
    {
        Monitor<Mutex>::Lock sync(*this);
        _terminate = true;
        notify();
    }

    void
    dump()
    {
        cout << _overallTouches << "/" << _overallTimeout;
    }

private:

    bool _verbose;
    bool _terminate;
    int _touches;
    int _timeout;
    long _overallTouches;
    long _overallTimeout;
};
typedef Handle<WatchDog> WatchDogPtr;

class TestThread : public Thread
{
public:

    TestThread(const WatchDogPtr& dog, const QueuePtr& q) :
        _dog(dog), _q(q)
    {
    }
    virtual void
    run()
    {
        while(true)
        {
            int res;
            // This is a poll.
            bool tout = _q->timedGet(res, Time::seconds(0));
            _dog->touch(!tout);
            if(res == -1)
            {
                return;
            }
        }
    }

private:
    const WatchDogPtr _dog;
    const QueuePtr _q;
};
typedef Handle<TestThread> TestThreadPtr;

class EnqueueThread : public Thread
{
public:

    EnqueueThread(const QueuePtr& q, int v) :
        _q(q), _v(v)
    {
    }

    virtual void
    run()
    {
        // Forever
        if(_v == 0)
        {
            while(true)
            {
                _q->put(_v);
            }
            //ThreadControl::sleep(Time::milliSeconds(1));
        }
        else
        {
            while(_v > 0)
            {
                _q->put(_v);
                --_v;
                //ThreadControl::sleep(Time::milliSeconds(1));
            }
        }
    }

private:

    const QueuePtr _q;
    int _v;
};
typedef Handle<EnqueueThread> EnqueueThreadPtr;

int
main(int argc, char** argv)
{
    Options opts;
    opts.addOpt("n", "events", Options::NeedArg, "5000");
    opts.addOpt("v", "verbose");
    try
    {
        opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        return EXIT_FAILURE;
    }

    srand(0);

    //
    // if n = 0 then we'll enqueue forever. Otherwise the test will
    // terminate after n events have been enqueued by each enqueue
    // thread.
    //
    int n = atoi(opts.optArg("n").c_str());
    bool verbose = opts.isSet("v");

    cout << "running signal/broadcast timeout test... " << flush;
    QueuePtr signalQ = new Queue(false);
    WatchDogPtr signalDog = new WatchDog(verbose);

    QueuePtr broadcastQ = new Queue(true);
    WatchDogPtr broadcastDog = new WatchDog(verbose);

    list<TestThreadPtr> testThreads;
    list<EnqueueThreadPtr> enqThreads;
    int i;

    for(i = 0; i < 100; i++)
    {
        TestThreadPtr p = new TestThread(signalDog, signalQ);
        p->start();
        testThreads.push_back(p);
    }
    for(i = 0; i < 5; i++)
    {
        EnqueueThreadPtr p = new EnqueueThread(signalQ, n);
        p->start();
        enqThreads.push_back(p);
    }

    for(i = 0; i < 100; i++)
    {
        TestThreadPtr p = new TestThread(broadcastDog, broadcastQ);
        p->start();
        testThreads.push_back(p);
    }
    for(i = 0; i < 5; i++)
    {
        EnqueueThreadPtr p = new EnqueueThread(broadcastQ, n);
        p->start();
        enqThreads.push_back(p);
    }

    signalDog->start();
    broadcastDog->start();

    while(!enqThreads.empty())
    {
        EnqueueThreadPtr p = enqThreads.front();
        enqThreads.pop_front();
        p->getThreadControl().join();
    }

    signalQ->terminate();
    broadcastQ->terminate();

    while(!testThreads.empty())
    {
        TestThreadPtr p = testThreads.front();
        testThreads.pop_front();
        p->getThreadControl().join();
    }

    if(verbose)
    {
        cout << endl;
    }
    broadcastDog->terminate();
    broadcastDog->getThreadControl().join();

    signalDog->terminate();
    signalDog->getThreadControl().join();

    cout << "broadcast (";
    broadcastDog->dump();

    cout << ") signal (";
    signalDog->dump();
    cout << ") ok" << endl;

    return 0;
}
