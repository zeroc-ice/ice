//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

RetryException::RetryException(exception_ptr ex)
{
    _ex = ex;
}

RetryException::RetryException(const RetryException& ex)
{
    _ex = ex.get();
}

exception_ptr
RetryException::get() const
{
    return _ex;
}

RequestHandler::RequestHandler(const ReferencePtr& reference) :
    _reference(reference),
    _response(reference->getMode() == Reference::ModeTwoway)
{
}
