//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/IncomingAsync.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ReplyStatus.h>

#include <mutex>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

std::mutex globalMutex;

}

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _responseSent(false),
    _responseHandlerCopy(_responseHandler->shared_from_this())
{
}

shared_ptr<IncomingAsync>
IceInternal::IncomingAsync::create(Incoming& in)
{
    auto async = make_shared<IncomingAsync>(in);
    in.setAsync(async);
    return async;
}

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
            if(p->first && !p->first())
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
    catch(const std::exception&)
    {
        IncomingBase::exception(current_exception(), true); // User thread
    }
    catch(...)
    {
        IncomingBase::exception("unknown c++ exception", true); // User thread
    }
}

void
IceInternal::IncomingAsync::checkResponseSent()
{
    lock_guard lock(globalMutex);
    if(_responseSent)
    {
        throw ResponseSentException(__FILE__, __LINE__);
    }
    _responseSent = true;
}
