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

#ifndef MISSIVE_H
#define MISSIVE_H

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>

namespace Glacier
{

class Request;
typedef IceUtil::Handle<Request> RequestPtr;

class Request : virtual public IceUtil::Shared
{
public:

    Request(const Ice::ObjectPrx&, const std::vector<Ice::Byte>&, const Ice::Current&, bool,
	    const Ice::AMI_Object_ice_invokePtr& = 0);
    
    void invoke();
    bool override(const RequestPtr&);
    const Ice::ObjectPrx& getProxy() const;
    const Ice::Current& getCurrent() const;

private:

    Ice::ObjectPrx _proxy;
    std::vector<Ice::Byte> _inParams;
    Ice::Current _current;
    bool _forwardContext;
    Ice::AMI_Object_ice_invokePtr _amiCB;
    std::string _override;
};

class RequestQueue;
typedef IceUtil::Handle<RequestQueue> RequestQueuePtr;

class RequestQueue : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RequestQueue(const Ice::CommunicatorPtr&, int, bool, const IceUtil::Time&);
    virtual ~RequestQueue();
    
    void destroy();
    void addMissive(const RequestPtr&);
    void addRequest(const RequestPtr&);

    virtual void run();

private:

    Ice::CommunicatorPtr _communicator;
    const Ice::LoggerPtr _logger;
    const int _traceLevel;
    const bool _reverse;
    const IceUtil::Time _sleepTime;

    std::vector<RequestPtr> _missives;
    std::vector<RequestPtr> _requests;
    bool _destroy;
};

}

#endif
