// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>
#include <IceUtil/Thread.h>
#include <Ice/Ice.h>

class ThreadHookI : public Ice::ThreadNotification, public IceUtil::Mutex
{
public:

    ThreadHookI();

    virtual void start();
    virtual void stop();

    Ice::Int getThreadNum() const;
    int activeThreads() const;

private:

    Ice::Int getThreadNumInternal() const;

    typedef std::vector<std::pair<IceUtil::ThreadControl::ID, Ice::Int> > ThreadList;
    ThreadList _threads;
    Ice::Int _nextThreadId;
};
typedef IceUtil::Handle<ThreadHookI> ThreadHookIPtr;

class AdapterI : virtual public Test::Adapter, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AdapterI(const ThreadHookIPtr&, bool);

    virtual bool isThreadPerConnection(const Ice::Current&);
    virtual Ice::Int getThreadId(const Ice::Current&);
    virtual void reset(const Ice::Current&);
    virtual Ice::Int waitForWakeup(Ice::Int, bool&, const Ice::Current&);
    virtual Ice::Int wakeup(const Ice::Current&);
    virtual void callSelf(const Test::AdapterPrx&, const Ice::Current&);

private:

    ThreadHookIPtr _threadHook;
    bool _threadPerConnection;
    bool _waiting;
    bool _notified;
};

class ServerI : virtual public Test::Server
{
public:

    ServerI(const Ice::CommunicatorPtr&);

    virtual Test::AdapterSeq getAdapters(const Ice::Current&);
    virtual void ping(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    ThreadHookIPtr _hook;
    Test::AdapterSeq _adapters;
};

#endif
