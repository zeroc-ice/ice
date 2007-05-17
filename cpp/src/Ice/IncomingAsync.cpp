// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/StaticMutex.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IncomingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(AMD_Object_ice_invoke* p) { return p; }
IceUtil::Shared* IceInternal::upCast(AMD_Array_Object_ice_invoke* p) { return p; }

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _instanceCopy(_os.instance()),
    _connectionCopy(_connection),
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
        IceUtil::StaticMutex::Lock lock(IceUtil::globalMutex);
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

    in.adopt(*this);
}

void
IceInternal::IncomingAsync::__response(bool ok)
{
    try
    {
        if(!__servantLocatorFinished())
        {
            return;
        }

        if(_response)
        {
            _os.endWriteEncaps();
            
            if(ok)
            {
                *(_os.b.begin() + headerSize + 4) = replyOK;
            }
            else
            {
                *(_os.b.begin() + headerSize + 4) = replyUserException;
            }

            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const LocalException& ex)
    {
        _connection->invokeException(ex, 1); // Fatal invocation exception
    }
}

void
IceInternal::IncomingAsync::__exception(const Exception& exc)
{
    try
    {
        if(!__servantLocatorFinished())
        {
            return;
        }

        __handleException(exc);
    }
    catch(const LocalException& ex)
    {
        _connection->invokeException(ex, 1);  // Fatal invocation exception
    }
}

void
IceInternal::IncomingAsync::__exception(const std::exception& exc)
{
    try
    {
        if(!__servantLocatorFinished())
        {
            return;
        }

        __handleException(exc);
    }
    catch(const LocalException& ex)
    {
        _connection->invokeException(ex, 1);  // Fatal invocation exception
    }
}

void
IceInternal::IncomingAsync::__exception()
{
    try
    {
        if(!__servantLocatorFinished())
        {
            return;
        }

        __handleException();
    }
    catch(const LocalException& ex)
    {
        _connection->invokeException(ex, 1);  // Fatal invocation exception
    }
}

bool
IceInternal::IncomingAsync::__servantLocatorFinished()
{
    try
    {
        if(_locator && _servant)
        {
            _locator->finished(_current, _servant, _cookie);
        }
        return true;
    }
    catch(const Exception& ex)
    {
        __handleException(ex);
        return false;
    }
    catch(const std::exception& ex)
    {
        __handleException(ex);
        return false;
    }
    catch(...)
    {
        __handleException();
        return false;
    }
}

bool
IceInternal::IncomingAsync::__validateResponse(bool ok)
{
    if(!_retriable)
    {
        return true;
    }
    
    try
    {
        for(std::deque<Ice::DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
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
    
    IceUtil::StaticMutex::Lock lock(IceUtil::globalMutex);
    if(_active)
    {
        _active = false;
        return true;
    }
    else
    {
        return false;
    }
}

    
bool 
IceInternal::IncomingAsync::__validateException(const std::exception& ex)
{
    if(!_retriable)
    {
        return true;
    }
    
    try
    {
        for(std::deque<Ice::DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
            p != _interceptorAsyncCallbackQueue.end(); ++p)
        {
            if((*p)->exception(ex) == false)
            {
                return false;
            }
        }
    }
    catch(...)
    {
        return false;
    }
    
    IceUtil::StaticMutex::Lock lock(IceUtil::globalMutex);
    if(_active)
    {
        _active = false;
        return true;
    }
    else
    {
        return false;
    }
}

bool 
IceInternal::IncomingAsync::__validateException()
{
    if(!_retriable)
    {
        return true;
    }
    
    try
    {
        for(std::deque<Ice::DispatchInterceptorAsyncCallbackPtr>::iterator p = _interceptorAsyncCallbackQueue.begin();
            p != _interceptorAsyncCallbackQueue.end(); ++p)
        {
            if((*p)->exception() == false)
            {
                return false;
            }
        }
    }
    catch(...)
    {
        return false;
    }
    
    IceUtil::StaticMutex::Lock lock(IceUtil::globalMutex);
    if(_active)
    {
        _active = false;
        return true;
    }
    else
    {
        return false;
    }
}


IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const vector<Byte>& outParams)
{
    if(__validateResponse(ok))
    {
        try
        {
            __os()->writeBlob(outParams);
        }
        catch(const LocalException& ex)
        {
            __exception(ex);
            return;
        }
        __response(ok);
    }
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const Exception& ex)
{
    if(__validateException(ex))
    {
        __exception(ex);
    }
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const std::exception& ex)
{
    if(__validateException(ex))
    {
        __exception(ex);
    }
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception()
{
    if(__validateException())
    {
        __exception();
    }
}

IceAsync::Ice::AMD_Array_Object_ice_invoke::AMD_Array_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
{
    if(__validateResponse(ok))
    {
        try
        {
            __os()->writeBlob(outParams.first, static_cast<Int>(outParams.second - outParams.first));
        }
        catch(const LocalException& ex)
        {
            __exception(ex);
            return;
        }
        __response(ok);
    }
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const Exception& ex)
{
    if(__validateException(ex))
    {
        __exception(ex);
    }
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const std::exception& ex)
{ 
    if(__validateException(ex))
    {
        __exception(ex);
    }
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception()
{
    if(__validateException())
    {
        __exception();
    }
}
