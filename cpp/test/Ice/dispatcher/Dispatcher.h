//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef DISPATCHER_I_H
#define DISPATCHER_I_H

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/Dispatcher.h>
#include <Ice/Connection.h>

#include <deque>

class DispatcherCall
{
public:

    DispatcherCall(std::function<void()> call) :
        _call(std::move(call))
    {
    }

    void run()
    {
        _call();
    }

private:

    std::function<void()> _call;
};

class Dispatcher : public IceUtil::Thread
{
public:

    static std::shared_ptr<Dispatcher> create();
    void dispatch(const std::shared_ptr<DispatcherCall>&, const std::shared_ptr<Ice::Connection>&);

    void run();
    void terminate();

    static bool isDispatcherThread();

private:

    Dispatcher();
    static std::shared_ptr<Dispatcher> _instance;
    std::deque<std::shared_ptr<DispatcherCall>> _calls;
    bool _terminated;
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
};

#endif
