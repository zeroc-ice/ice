// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DISPATCHER_I_H
#define DISPATCHER_I_H

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/Dispatcher.h>
#include <Ice/Connection.h>

#include <deque>

#ifdef ICE_CPP11_MAPPING
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
#endif

class Dispatcher :
#ifndef ICE_CPP11_MAPPING
    public Ice::Dispatcher,
#endif
public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    Dispatcher();

#ifdef ICE_CPP11_MAPPING
    void dispatch(const std::shared_ptr<DispatcherCall>&, const std::shared_ptr<Ice::Connection>&);
#else
    virtual void dispatch(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);
#endif


    void run();

    static void terminate();
    static bool isDispatcherThread();

private:

    static IceUtil::Handle<Dispatcher> _instance;
#ifdef ICE_CPP11_MAPPING
    std::deque<std::shared_ptr<DispatcherCall>> _calls;
#else
    std::deque<Ice::DispatcherCallPtr> _calls;
#endif
    bool _terminated;
};

#endif
