// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef MISSIVE_H
#define MISSIVE_H

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>

namespace Glacier
{

class Missive;
typedef IceUtil::Handle<Missive> MissivePtr;

class Missive : virtual public IceUtil::Shared
{
public:

    Missive(const Ice::ObjectPrx&, const std::vector<Ice::Byte>&, const Ice::Current&);
    
    virtual Ice::ObjectPrx invoke();
    virtual bool override(const MissivePtr&);

private:

    const Ice::ObjectPrx _proxy;
    const std::vector<Ice::Byte>& _inParams;
    Ice::Current _current;
};

class MissiveQueue;
typedef IceUtil::Handle<MissiveQueue> MissiveQueuePtr;

class MissiveQueue : virtual public IceUtil::Thread, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    MissiveQueue();
    virtual ~MissiveQueue();
    
    void destroy();
    void add(const MissivePtr&);

    virtual void run();

protected:

    Ice::ObjectPrx _proxy;
    std::vector<MissivePtr> _missives;
    std::auto_ptr<Ice::Exception> _exception;
    bool _destroy;
};

};

#endif

