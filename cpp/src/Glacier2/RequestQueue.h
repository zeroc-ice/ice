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

class Request : virtual public IceUtil::Shared
{
public:

    Request(const Ice::ObjectPrx&, const std::vector<Ice::Byte>&, const Ice::Current&, bool,
	    const Ice::AMD_Object_ice_invokePtr&);
    
    void invoke();
    bool override(const RequestPtr&) const;
    const Ice::ObjectPrx& getProxy() const;
    const Ice::Current& getCurrent() const;

private:

    const Ice::ObjectPrx _proxy;
    const std::vector<Ice::Byte> _inParams;
    const Ice::Current _current;
    const bool _forwardContext;
    const Ice::AMI_Object_ice_invokePtr _amiCB;
    const std::string _override;
};

class RequestQueue;
typedef IceUtil::Handle<RequestQueue> RequestQueuePtr;

class RequestQueue : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RequestQueue(const Ice::CommunicatorPtr&, bool);
    virtual ~RequestQueue();
    
    void destroy();
    void addRequest(const RequestPtr&);

    virtual void run();

private:

    void traceRequest(const RequestPtr&, const std::string&) const;

    const Ice::LoggerPtr _logger;
    const bool _reverse;
    const IceUtil::Time _sleepTime;
    const int _requestTraceLevel;
    const int _overrideTraceLevel;

    std::vector<RequestPtr> _requests;

    bool _destroy;
};

}

#endif
