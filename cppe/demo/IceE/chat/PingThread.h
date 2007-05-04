// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CHAT_PING_THREAD_H
#define CHAT_PING_THREAD_H

#include <IceE/IceE.h>
#include <Router.h>

class SessionPingThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionPingThread(const Glacier2::SessionPrx& session, long timeout);

    virtual void run();
    void destroy();

private:

    const Glacier2::SessionPrx _session;
    const IceUtil::Time _timeout;
    bool _destroy;
};
typedef IceUtil::Handle<SessionPingThread> SessionPingThreadPtr;

#endif
