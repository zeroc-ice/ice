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

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IncomingAsync* p) { p->__incRef(); }
void IceInternal::decRef(IncomingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Object_ice_invoke* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Array_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Array_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _instanceCopy(_os.instance()),
    _connectionCopy(_connection)
{
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
                *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchOK);
            }
            else
            {
                *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchUserException);
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

IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const vector<Byte>& outParams)
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

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const Exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception()
{
    __exception();
}

IceAsync::Ice::AMD_Array_Object_ice_invoke::AMD_Array_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
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

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const Exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception()
{
    __exception();
}
