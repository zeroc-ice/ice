//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>

using namespace std;
using namespace IceInternal;

RetryException::RetryException(const Ice::LocalException& ex)
{
    _ex = ex.ice_clone();
}

RetryException::RetryException(const RetryException& ex)
{
    _ex = ex.get()->ice_clone();
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
