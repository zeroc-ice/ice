// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_MANAGER_I_H
#define SESSION_MANAGER_I_H

#include <IceUtil/Thread.h>
#include <Session.h>

class SessionManagerI;
typedef IceUtil::Handle<SessionManagerI> SessionManagerIPtr;

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread(const SessionManagerIPtr&, const IceUtil::Time&);
    ~ReapThread();

    virtual void run();
    void destroy();

private:

    bool _destroy;
    const IceUtil::Time _timeout;
    const SessionManagerIPtr _manager;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

//
// Users wanting to create their own session manager specialization
// should inherit off this class and implement create(). create() must
// call add() with the creation session proxy.
// XXX Not nice! It shouldn't be necessary for the derived class to do this. Instead, create() should be implemented here, and call another virtual function. XXX
//
class SessionManagerI : public Demo::SessionManager, public IceUtil::Mutex
{
public:

    SessionManagerI(const Ice::CommunicatorPtr&);
    ~SessionManagerI(); // XXX virtual XXX

    virtual Demo::SessionPrx create(const Ice::Current&) = 0;
    virtual void shutdown(const Ice::Current&);

// XXX Why are these not protected? XXX Get rid off add(), see comment above. XXX Get rid of remove(), too! This is evil, session are not supposed to remove themselves, this only leads to deadlocks. They must get reaped. XXX Get rid of refresh() - why is this needed? XXX
    void destroy();
    void add(const Demo::SessionPrx&);
    void remove(const Ice::Identity&);
    void refresh(const Ice::Identity&);
    void reap();

private:

    const IceUtil::Time _timeout;
    ReapThreadPtr _reapThread;
    const Ice::LoggerPtr _logger;
    std::map<Ice::Identity, std::pair<IceUtil::Time, Demo::SessionPrx> > _sessions;
    bool _destroy;
};

typedef IceUtil::Handle<SessionManagerI> SessionManagerIPtr;

#endif
