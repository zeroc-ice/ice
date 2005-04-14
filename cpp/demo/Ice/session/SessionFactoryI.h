// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_FACTORY_I_H
#define SESSION_FACTORY_I_H

#include <IceUtil/Thread.h>
#include <SessionI.h>

#include <list>

class SessionFactoryI;
typedef IceUtil::Handle<SessionFactoryI> SessionFactoryIPtr;

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread(const SessionFactoryIPtr&, const IceUtil::Time&);
    ~ReapThread();

    virtual void run();
    void destroy();

private:

    bool _destroy;
    const IceUtil::Time _timeout;
    SessionFactoryIPtr _factory;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

class SessionFactoryI : public ::Demo::SessionFactory, public IceUtil::Mutex
{
public:

    SessionFactoryI(const Ice::ObjectAdapterPtr&);
    ~SessionFactoryI();

    virtual ::Demo::SessionPrx create(const ::Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    void reap();
    void destroy();

private:

    const Ice::ObjectAdapterPtr _adapter;
    const IceUtil::Time _timeout;
    ReapThreadPtr _reapThread;
    std::list<std::pair<SessionIPtr, ::Ice::Identity> > _sessions;
};

#endif
