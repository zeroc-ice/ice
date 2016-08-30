// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IncomingAsync.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ReplyStatus.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(IncomingAsync* p) { return p; }
IceUtil::Shared* Ice::upCast(AMD_Object_ice_invoke* p) { return p; }

Ice::AMDCallback::~AMDCallback()
{
    // Out of line to avoid weak vtable
}

Ice::AMD_Object_ice_invoke::~AMD_Object_ice_invoke()
{
    // Out of line to avoid weak vtable
}
#endif

namespace
{

IceUtil::Mutex* globalMutex = 0;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _responseSent(false),
    _responseHandlerCopy(ICE_GET_SHARED_FROM_THIS(_responseHandler))
{
#ifndef ICE_CPP11_MAPPING
    in.setAsync(this);
#endif
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<IncomingAsync>
IceInternal::IncomingAsync::create(Incoming& in)
{
    auto async = make_shared<IncomingAsync>(in);
    in.setAsync(async);
    return async;
}
#endif

#ifndef ICE_CPP11_MAPPING
void
IceInternal::IncomingAsync::ice_exception(const ::std::exception& exc)
{
    try
    {
        for(DispatchInterceptorCallbacks::iterator p = _interceptorCBs.begin(); p != _interceptorCBs.end(); ++p)
        {
            if(!(*p)->exception(exc))
            {
                return;
            }
        }
    }
    catch(...)
    {
        return;
    }

    checkResponseSent();
    IncomingBase::exception(exc, true); // User thread
}

void
IceInternal::IncomingAsync::ice_exception()
{
    try
    {
        for(DispatchInterceptorCallbacks::iterator p = _interceptorCBs.begin(); p != _interceptorCBs.end(); ++p)
        {
            if(!(*p)->exception())
            {
                return;
            }
        }
    }
    catch(...)
    {
        return;
    }

    checkResponseSent();
    IncomingBase::exception("unknown c++ exception", true); // User thread
}

#endif

void
IceInternal::IncomingAsync::kill(Incoming& in)
{
    checkResponseSent();
    in._observer.adopt(_observer); // Give back the observer to incoming.
}

void
IceInternal::IncomingAsync::completed()
{
    for(DispatchInterceptorCallbacks::iterator p = _interceptorCBs.begin(); p != _interceptorCBs.end(); ++p)
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            if(p->first && !p->first())
#else
            if(!(*p)->response())
#endif
            {
                return;
            }
        }
        catch(...)
        {
        }
    }

    checkResponseSent();
    IncomingBase::response(true); // User thread
}

#ifdef ICE_CPP11_MAPPING
void
IceInternal::IncomingAsync::completed(exception_ptr ex)
{
    for(DispatchInterceptorCallbacks::iterator p = _interceptorCBs.begin(); p != _interceptorCBs.end(); ++p)
    {
        try
        {
            if(p->second && !p->second(ex))
            {
                return;
            }
        }
        catch(...)
        {
        }
    }

    checkResponseSent();
    try
    {
        rethrow_exception(ex);
    }
    catch(const std::exception& exc)
    {
        IncomingBase::exception(exc, true); // User thread
    }
    catch(...)
    {
        IncomingBase::exception("unknown c++ exception", true); // User thread
    }
}
#endif

void
IceInternal::IncomingAsync::checkResponseSent()
{
    IceUtil::Mutex::Lock sync(*globalMutex);
    if(_responseSent)
    {
        throw ResponseSentException(__FILE__, __LINE__);
    }
    _responseSent = true;
}

#ifndef ICE_CPP11_MAPPING
IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) : IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const vector<Byte>& outEncaps)
{
    if(outEncaps.empty())
    {
        writeParamEncaps(0, 0, ok);
    }
    else
    {
        writeParamEncaps(&outEncaps[0], static_cast<Int>(outEncaps.size()), ok);
    }
    completed();
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const pair<const Byte*, const Byte*>& outEncaps)
{
    writeParamEncaps(outEncaps.first, static_cast<Int>(outEncaps.second - outEncaps.first), ok);
    completed();
}
#endif
