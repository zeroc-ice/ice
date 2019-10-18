//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>

using namespace std;
using namespace IceInternal;

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(RequestHandler* p) { return p; }
IceUtil::Shared* IceInternal::upCast(CancellationHandler* p) { return p; }
#endif

RetryException::RetryException(const Ice::LocalException& ex)
{
    ICE_SET_EXCEPTION_FROM_CLONE(_ex, ex.ice_clone());
}

RetryException::RetryException(const RetryException& ex)
{
    ICE_SET_EXCEPTION_FROM_CLONE(_ex, ex.get()->ice_clone());
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
