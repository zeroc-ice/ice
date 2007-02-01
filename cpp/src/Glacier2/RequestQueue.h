// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class RequestQueue;
typedef IceUtil::Handle<RequestQueue> RequestQueuePtr;

class Request : public IceUtil::Shared
{
public:

    Request(const Ice::ObjectPrx&, const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&, bool,
            const Ice::Context&, const Ice::AMD_Array_Object_ice_invokePtr&);
    
    bool invoke(const RequestQueuePtr&);
    bool override(const RequestPtr&) const;
    bool isBatch() const;
    Ice::ConnectionPtr getConnection() const;

private:

    const Ice::ObjectPrx _proxy;
    const Ice::ByteSeq _inParams;
    const Ice::Current _current;
    const bool _forwardContext;
    const Ice::Context _sslContext;
    const std::string _override;
    const Ice::AMD_Array_Object_ice_invokePtr _amdCB;
};

class Response : public IceUtil::Shared
{
public:

    Response(const Ice::AMD_Array_Object_ice_invokePtr&, bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&);
    Response(const Ice::AMD_Array_Object_ice_invokePtr&, const Ice::Exception&);

    void invoke();
    
private:

    const Ice::AMD_Array_Object_ice_invokePtr _amdCB;
    const bool _ok;
    const Ice::ByteSeq _outParams;
    const std::auto_ptr<Ice::Exception> _exception;
};

class Response;
typedef IceUtil::Handle<Response> ResponsePtr;

class RequestQueue : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RequestQueue(const IceUtil::Time&);
    virtual ~RequestQueue();
    
    void destroy();
    bool addRequest(const RequestPtr&);
    void addResponse(const ResponsePtr&);

    virtual void run();

private:

    const IceUtil::Time _sleepTime;
    std::vector<RequestPtr> _requests;
    std::vector<ResponsePtr> _responses;
    bool _destroy;
    bool _sleep;
    IceUtil::Time _sleepDuration;
};

}

#endif
