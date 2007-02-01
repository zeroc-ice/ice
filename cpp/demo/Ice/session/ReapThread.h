// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REAP_THREAD_H
#define REAP_THREAD_H

#include <IceUtil/Thread.h>
#include <SessionI.h>
#include <list>

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread();

    virtual void run();
    void terminate();
    void add(const Demo::SessionPrx&, const SessionIPtr&);

private:

    const IceUtil::Time _timeout;
    bool _terminated;
    struct SessionProxyPair
    {
        SessionProxyPair(const Demo::SessionPrx& p, const SessionIPtr& s) :
            proxy(p), session(s) { }
        const Demo::SessionPrx proxy;
        const SessionIPtr session;
    };
    std::list<SessionProxyPair> _sessions;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

#endif
