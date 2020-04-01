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
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

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
    catch(const std::exception& exc)
    {
        IncomingBase::exception(exc, true); // User thread
    }
    catch(...)
    {
        IncomingBase::exception("unknown c++ exception", true); // User thread
    }
}

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
