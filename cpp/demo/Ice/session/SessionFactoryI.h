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
    virtual ~ReapThread();

    virtual void run();

    // XXX Rename to destroy().
    void terminate();

private:

    bool _terminated;
    const IceUtil::Time _timeout;
    SessionFactoryIPtr _factory;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

class SessionFactoryI : public ::Demo::SessionFactory, public IceUtil::Mutex
{
public:

    SessionFactoryI(const Ice::ObjectAdapterPtr&);
    virtual ~SessionFactoryI();

    virtual ::Demo::SessionPrx create(const ::Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    void reap();
    void destroy();

private:

    const Ice::ObjectAdapterPtr _adapter; // XXX Get rid of this, not needed.
    const IceUtil::Time _timeout; // XXX Get rid of this, see other comments.

    // XXX Why does the factory have to know the reaper thread? The
    // sessions should know, they can register themselves directly
    // with the reaper thread.

    ReapThreadPtr _reapThread;

    // XXX Get rid of this. Use a map<Ice::Identity, SessionIPtr>.
    struct SessionId
    {
    	SessionId(const SessionIPtr& s, const ::Ice::Identity& i) : session(s), id(i) { }
	const SessionIPtr session;
	const ::Ice::Identity id;
    };

    // XXX Why does SessionFactoryI keep a list of sessions? Clearly,
    // only the reaper must know it, which currently simply delegates
    // work to the SessionFactoryI, instead of doing it directly.
    std::list<SessionId> _sessions;
};

#endif
