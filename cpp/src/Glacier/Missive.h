// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
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

    Missive(const Ice::ObjectPrx&, const std::vector<Ice::Byte>&, const Ice::Current&, bool);
    
    void invoke();
    bool override(const MissivePtr&);
    const Ice::ObjectPrx& getProxy() const;
    const Ice::Current& getCurrent() const;

private:

    Ice::ObjectPrx _proxy;
    std::vector<Ice::Byte> _inParams;
    Ice::Current _current;
    bool _forwardContext;
    std::string _override;
};

class MissiveQueue;
typedef IceUtil::Handle<MissiveQueue> MissiveQueuePtr;

class MissiveQueue : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    MissiveQueue(const Ice::CommunicatorPtr&, int, bool, const IceUtil::Time&);
    virtual ~MissiveQueue();
    
    void destroy();
    void add(const MissivePtr&);

    virtual void run();

private:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;
    int _traceLevel;
    bool _reverse;
    IceUtil::Time _sleepTime;

    std::vector<MissivePtr> _missives;
    bool _destroy;
};

};

#endif

