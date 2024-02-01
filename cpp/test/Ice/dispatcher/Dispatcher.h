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

class Dispatcher : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Dispatcher();

    void dispatch(const std::shared_ptr<DispatcherCall>&, const std::shared_ptr<Ice::Connection>&);

    void run();

    static void terminate();
    static bool isDispatcherThread();

private:

    static IceUtil::Handle<Dispatcher> _instance;
    std::deque<std::shared_ptr<DispatcherCall>> _calls;
    bool _terminated;
};

#endif
