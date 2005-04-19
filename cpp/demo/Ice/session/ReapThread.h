// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REAP_THREAD_H
#define REAP_THREAD_H

#include <IceUtil/Thread.h>
#include <IceUtil/StaticMutex.h>
#include <SessionI.h>

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;
class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    static ReapThreadPtr& instance();

    virtual ~ReapThread(); // XXX Destructor does nothing, get rid of it.

    virtual void run();

    void terminate();

    void add(const Demo::SessionPrx&, const SessionIPtr&);

private:

    ReapThread();

    const IceUtil::Time _timeout;
    bool _terminated;
    // XXX Why is this a map and not simply a list of
    // pair<Demo::SessionPrx, SessionIPtr> (or a list of structs with
    // these elements)? The sorting of a map is needed nowhere.
    std::map< Demo::SessionPrx, SessionIPtr> _sessions;

    static ReapThreadPtr _instance;
    static IceUtil::StaticMutex _instanceMutex;
};

#endif
