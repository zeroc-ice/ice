// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_WAIT_QUEUE_H
#define ICE_GRID_WAIT_QUEUE_H

#include <IceUtil/Thread.h>

#include <list>

namespace IceGrid
{

class WaitItem : public ::IceUtil::SimpleShared
{
public:
    
    WaitItem(const Ice::ObjectPtr&);
    virtual ~WaitItem();

    virtual void execute() = 0;
    virtual void expired(bool) = 0;
    
    bool isWaitingOn(const Ice::ObjectPtr& obj) { return obj == _object; }

    const IceUtil::Time& getExpirationTime();
    void setExpirationTime(const IceUtil::Time&);

private:

    Ice::ObjectPtr _object;
    IceUtil::Time _expiration;
};

typedef IceUtil::Handle<WaitItem> WaitItemPtr;

class WaitQueue : public IceUtil::Thread, public IceUtil::Monitor< IceUtil::Mutex>
{
public:

    WaitQueue();

    virtual void run();
    void destroy();

    void add(const WaitItemPtr&, const IceUtil::Time&);

    void notifyAllWaitingOn(const Ice::ObjectPtr&);

private:
    
    std::list<WaitItemPtr> _waitQueue;
    std::list<WaitItemPtr> _workQueue;
    bool _destroyed;
};

typedef IceUtil::Handle<WaitQueue> WaitQueuePtr;

}

#endif
