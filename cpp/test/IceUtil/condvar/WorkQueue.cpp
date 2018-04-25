// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Options.h>

#include <iostream>
#include <list>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

class CountDown : public Monitor<Mutex>, public Shared
{
public:

    CountDown(int count) : _count(count) { }

    void decrement()
    {
        Monitor<Mutex>::Lock lock(*this);
        assert(_count > 0);
        --_count;
        if(_count == 0)
        {
            notifyAll();
        }
    }

    void waitZero()
    {
        Monitor<Mutex>::Lock lock(*this);
        while(_count > 0)
        {
            wait();
        }
    }

private:

    int _count;
};
typedef Handle<CountDown> CountDownPtr;

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
        notifyAll();
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
        if(ret % 100 == 0)
        {
            cout << "." << flush;
        }
        _q.pop_front();
        return true;
    }

    int
    get()
    {
        Monitor<Mutex>::Lock lock(*this);
        while(_q.empty() && !_terminate)
        {
            wait();
        }

        // We only report the termination sentinel when the queue is
        // empty.
        if(_q.empty())
        {
            assert(_terminate);
            return -1;
        }

        assert(!_q.empty());
        int ret = _q.front();
        if(ret % 100 == 0)
        {
            cout << "." << flush;
        }
        _q.pop_front();
        return ret;
    }

private:
    const bool _broadcast;
    bool _terminate;
    list<int> _q;
};
typedef Handle<Queue> QueuePtr;

class TestThread : public Thread
{
public:

    TestThread(const CountDownPtr& cd, const QueuePtr& q, bool poll) :
        _cd(cd), _q(q), _poll(poll)
    {
    }
    virtual void
    run()
    {
        _cd->decrement();
        while(true)
        {
            int res = 0;
            if(_poll)
            {
                _q->timedGet(res, Time::milliSeconds(10));
            }
            else
            {
                res = _q->get();
            }
            if(res == -1)
            {
                return;
            }
        }
    }

private:
    const CountDownPtr _cd;
    const QueuePtr _q;
    const bool _poll;
};
typedef Handle<TestThread> TestThreadPtr;

class EnqueueThread : public Thread
{
public:

    EnqueueThread(const CountDownPtr& cd, const QueuePtr& q, int v) :
        _cd(cd), _q(q), _v(v)
    {
    }

    virtual void
    run()
    {
        _cd->decrement();
        // Forever
        if(_v == 0)
        {
            while(true)
            {
                _q->put(_v++);
                ThreadControl::yield();
            }
        }
        else
        {
            while(_v > 0)
            {
                _q->put(_v);
                --_v;
                ThreadControl::yield();
            }
        }
    }

private:

    const CountDownPtr _cd;
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
    catch(const IceUtilInternal::BadOptException& e)
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

    cout << "running signal/broadcast timeout test" << flush;
    QueuePtr signalQ = new Queue(false);

    QueuePtr broadcastQ = new Queue(true);

    CountDownPtr cd = new CountDown(210);
    list<TestThreadPtr> testThreads;
    list<EnqueueThreadPtr> enqThreads;
    int i;

    for(i = 0; i < 100; i++)
    {
        TestThreadPtr p = new TestThread(cd, signalQ, i % 2 != 0);
        p->start();
        testThreads.push_back(p);
    }
    for(i = 0; i < 5; i++)
    {
        EnqueueThreadPtr p = new EnqueueThread(cd, signalQ, n);
        p->start();
        enqThreads.push_back(p);
    }

    for(i = 0; i < 100; i++)
    {
        TestThreadPtr p = new TestThread(cd, broadcastQ, i % 2 != 0);
        p->start();
        testThreads.push_back(p);
    }
    for(i = 0; i < 5; i++)
    {
        EnqueueThreadPtr p = new EnqueueThread(cd, broadcastQ, n);
        p->start();
        enqThreads.push_back(p);
    }
    cd->waitZero();

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

    cout << " ok" << endl;

    return 0;
}
