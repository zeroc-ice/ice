// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REQUEST_H
#define REQUEST_H

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Ice/Ice.h>

namespace Glacier2
{

class Request;
typedef IceUtil::Handle<Request> RequestPtr;

class Request : public IceUtil::Shared
{
public:

    Request(const Ice::ObjectPrx&, const Ice::ByteSeq&, const Ice::Current&, bool,
	    const Ice::AMD_Object_ice_invokePtr&);
    
    void invoke();
    bool override(const RequestPtr&) const;
    bool isBatch() const;
    Ice::ConnectionPtr getConnection() const;

private:

    const Ice::ObjectPrx _proxy;
    const Ice::ByteSeq _inParams;
    const Ice::Current _current;
    const bool _forwardContext;
    const std::string _override;
    const Ice::AMD_Object_ice_invokePtr _amdCB;
};

class RequestQueue;
typedef IceUtil::Handle<RequestQueue> RequestQueuePtr;

class RequestQueue : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RequestQueue(const IceUtil::Time&);
    virtual ~RequestQueue();
    
    void destroy();
    bool addRequest(const RequestPtr&);

    virtual void run();

private:

    const IceUtil::Time _sleepTime;
    std::vector<RequestPtr> _requests;
    bool _destroy;
};

}

#endif
