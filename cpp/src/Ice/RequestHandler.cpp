// Copyright (c) ZeroC, Inc.

#include "RequestHandler.h"
#include "Reference.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

RetryException::RetryException(exception_ptr ex) { _ex = ex; }

RetryException::RetryException(const RetryException& ex) { _ex = ex.get(); }

exception_ptr
RetryException::get() const
{
    return _ex;
}

CancellationHandler::~CancellationHandler() = default; // avoid weak vtable

RequestHandler::RequestHandler(const ReferencePtr& reference)
    : _reference(reference),
      _response(reference->getMode() == Reference::ModeTwoway)
{
}

RequestHandler::~RequestHandler() = default; // avoid weak vtable
