// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PACK_WAIT_QUEUE_H
#define ICE_PACK_WAIT_QUEUE_H

#include <IceUtil/Thread.h>

#include <list>

namespace IcePack
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

};

#endif
