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
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <list>

namespace IceGrid
{

class Reapable : public IceUtil::Shared
{
public:

    virtual ~Reapable() { }

    virtual IceUtil::Time timestamp() const = 0;
    virtual void destroy(bool) = 0;

};
typedef IceUtil::Handle<Reapable> ReapablePtr;

class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReapThread();
    
    virtual void run();
    void terminate();
    void add(const ReapablePtr&, int);

private:

    bool calcWakeInterval();
    
    IceUtil::Time _wakeInterval;
    bool _terminated;
    struct ReapableItem
    {
	ReapablePtr item;
	IceUtil::Time timeout;
    };
    std::list<ReapableItem> _sessions;
};
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

};

#endif
