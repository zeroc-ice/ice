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

#include <Session.h>
#include <ReapThread.h>

class SessionFactoryI : public ::Demo::SessionFactory, public IceUtil::Mutex
{
public:

    SessionFactoryI(const ReapThreadPtr&);
    virtual ~SessionFactoryI();

    virtual ::Demo::SessionPrx create(const ::Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    // XXX Why does the factory have to know the reaper thread? The
    // sessions should know, they can register themselves directly
    // with the reaper thread.
    //
    // If I do this it means that either the reap thread must be a
    // singleton, or I need to pass the reap thread to session and
    // tell the session its proxy which it currently does not
    // know. Since the session factory knows both, this seems like a
    // better solution.
    //
    const ReapThreadPtr _reapThread;
};

#endif
