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

    Request(const Ice::ObjectPrx&, const std::vector<Ice::Byte>&, const Ice::Current&,
	    const Ice::AMI_Object_ice_invokePtr&);
    
    void invoke(bool);
    bool override(const RequestPtr&) const;
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

    RequestQueue(const Ice::CommunicatorPtr&, bool);
    virtual ~RequestQueue();
    
    void destroy();
    void addRequest(const RequestPtr&);

    virtual void run();

private:

    void traceRequest(const RequestPtr&, const std::string&) const;

    const Ice::LoggerPtr _logger;
    const bool _reverse;
    const int _traceLevelRequest;
    const int _traceLevelOverride;
    const bool _forwardContext;
    const IceUtil::Time _sleepTime;

    std::vector<RequestPtr> _requests;
    bool _destroy;
};

}

#endif
