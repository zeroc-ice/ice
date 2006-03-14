// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include <IceUtil/Thread.h>
#include <Glacier2/SessionF.h>
#include <IceGrid/SessionI.h>
#include <list>

namespace IceGrid
{

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread(int);

    virtual void run();
    void terminate();
    void add(const Glacier2::SessionPrx&, const SessionIPtr&);

private:

    const IceUtil::Time _timeout;
    bool _terminated;
    std::list<std::pair<SessionIPtr, Glacier2::SessionPrx> > _sessions;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

};

#endif
