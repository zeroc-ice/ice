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

IceUtil::Shared* IceInternal::upCast(IncomingAsync* p) { return p; }
IceUtil::Shared* Ice::upCast(AMD_Object_ice_invoke* p) { return p; }

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
    _instanceCopy(_os.instance()),
    _responseHandlerCopy(_responseHandler),
    _retriable(in.isRetriable()),
    _active(true)
{
    if(_retriable)
    {
        in.setActive(*this);
    }
}

void
IceInternal::IncomingAsync::__deactivate(Incoming& in)
{
    assert(_retriable);
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
        if(!_active)
        {
            //
            // Since _deactivate can only be called on an active object,
            // this means the response has already been sent (see __validateXXX below)
            //
            throw ResponseSentException(__FILE__, __LINE__);
        }
        _active = false;
    }

    in.__adopt(*this);
}

void
IceInternal::IncomingAsync::ice_exception(const ::std::exception& ex)
{
    //
    // Only call __exception if this incoming is not retriable or if
    // all the interceptors return true and no response has been sent
    // yet.
    //

    if(_retriable)
    {
        try
        {
            for(deque<Ice::DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
                p != _interceptorAsyncCallbackQueue.end(); ++p)
            {
                if((*p)->exception(ex) == false)
                {
                    return;
                }
            }
        }
        catch(...)
        {
            return;
        }
    
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
        if(!_active)
        {
            return;
        }
        _active = false;
    }

    if(_responseHandler)
    {
        __exception(ex);
    }
    else
    {
        //
        // Response has already been sent.
        //
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex.what());
        }
    }
}

void
IceInternal::IncomingAsync::ice_exception()
{
    //
    // Only call __exception if this incoming is not retriable or if
    // all the interceptors return true and no response has been sent
    // yet.
    //

    if(_retriable)
    {
        try
        {
            for(deque<Ice::DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
                p != _interceptorAsyncCallbackQueue.end(); ++p)
            {
                if((*p)->exception() == false)
                {
                    return;
                }
            }
        }
        catch(...)
        {
            return;
        }
    
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
        if(!_active)
        {
            return;
        }
        _active = false;
    }

    if(_responseHandler)
    {
        __exception();
    }
    else
    {
        //
        // Response has already been sent.
        //
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning("unknown exception");
        }
    }
}

void
IceInternal::IncomingAsync::__response()
{
    try
    {
        if(_locator && !__servantLocatorFinished(true))
        {
            return;
        }

        assert(_responseHandler);

        if(_response)
        {
            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, true);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }

        _observer.detach();
        _responseHandler = 0;
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, true); // Fatal invocation exception
    }
}

void
IceInternal::IncomingAsync::__exception(const std::exception& exc)
{
    try
    {
        if(_locator && !__servantLocatorFinished(true))
        {
            return;
        }

        __handleException(exc, true);
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, true);  // Fatal invocation exception
    }
}

void
IceInternal::IncomingAsync::__exception()
{
    try
    {
        if(_locator && !__servantLocatorFinished(true))
        {
            return;
        }

        __handleException(true);
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, true);  // Fatal invocation exception
    }
}

bool
IceInternal::IncomingAsync::__validateResponse(bool ok)
{
    //
    // Only returns true if this incoming is not retriable or if all
    // the interceptors return true and no response has been sent
    // yet. Upon getting a true return value, the caller should send
    // the response.
    //

    if(_retriable)
    {
        try
        {
            for(deque<DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
                p != _interceptorAsyncCallbackQueue.end(); ++p)
            {
                if((*p)->response(ok) == false)
                {
                    return false;
                }
            }
        }
        catch(...)
        {
            return false;
        }
        
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);
        if(!_active)
        {
            return false;
        }
        _active = false;
    }
    return true;
}

IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const vector<Byte>& outEncaps)
{
    if(__validateResponse(ok))
    {
        try
        {
            if(outEncaps.empty())
            {
                __writeParamEncaps(0, 0, ok);
            }
            else
            {
                __writeParamEncaps(&outEncaps[0], static_cast< ::Ice::Int>(outEncaps.size()), ok);
            }
        }
        catch(const LocalException& ex)
        {
            __exception(ex);
            return;
        }
        __response();
    }
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const pair<const Byte*, const Byte*>& outEncaps)
{
    if(__validateResponse(ok))
    {
        try
        {
            __writeParamEncaps(outEncaps.first, static_cast<Int>(outEncaps.second - outEncaps.first), ok);
        }
        catch(const LocalException& ex)
        {
            __exception(ex);
            return;
        }
        __response();
    }
}
