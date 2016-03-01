// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>

using namespace std;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(RequestHandler* p) { return p; }
IceUtil::Shared* IceInternal::upCast(CancellationHandler* p) { return p; }

RetryException::RetryException(const Ice::LocalException& ex)
{
    _ex.reset(ex.ice_clone());
}

RetryException::RetryException(const RetryException& ex)
{
    _ex.reset(ex.get()->ice_clone());
}

const Ice::LocalException*
RetryException::get() const
{
    assert(_ex.get());
    return _ex.get();
}

RequestHandler::RequestHandler(const ReferencePtr& reference) :
    _reference(reference),
    _response(reference->getMode() == Reference::ModeTwoway)
{
}
