// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REAP_TASK_H
#define REAP_TASK_H

#include <IceUtil/IceUtil.h>
#include <SessionI.h>
#include <list>

class ReapTask : public IceUtil::TimerTask, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapTask();

    virtual void runTimerTask();
    void terminate();
    void add(const Demo::SessionPrx&, const SessionIPtr&);

private:

    const IceUtil::Time _timeout;
    struct SessionProxyPair
    {
        SessionProxyPair(const Demo::SessionPrx& p, const SessionIPtr& s) :
            proxy(p), session(s) { }
        const Demo::SessionPrx proxy;
        const SessionIPtr session;
    };
    std::list<SessionProxyPair> _sessions;
};
typedef IceUtil::Handle<ReapTask> ReapTaskPtr;

#endif
