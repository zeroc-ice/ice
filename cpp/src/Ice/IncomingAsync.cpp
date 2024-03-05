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

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
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
    checkResponseSent();
    IncomingBase::response(true); // User thread
}

void
IceInternal::IncomingAsync::completed(exception_ptr ex)
{
    checkResponseSent();
    IncomingBase::exception(ex, true); // true = amd
}

void
IceInternal::IncomingAsync::checkResponseSent()
{
    if (_responseSent.test_and_set())
    {
        throw ResponseSentException(__FILE__, __LINE__);
    }
}
