//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Invoker.h>
#include <Ice/Reference.h>
#include <Ice/ConnectionIF.h>
#include <Ice/RequestHandler.h>
#include <Ice/Instance.h>
#include <Ice/ProxyFactory.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ProxyInvoker::ProxyInvoker(const ReferencePtr& reference) : _reference(reference)
{
}

pair<AsyncStatus, RequestHandlerPtr>
ProxyInvoker::invoke(const ProxyOutgoingAsyncBasePtr& outgoingAsync)
{
    RequestHandlerPtr handler;
    if(_reference->getCacheConnection())
    {
        lock_guard lock(_mutex);
        if(!_requestHandler)
        {
            _requestHandler = _reference->getRequestHandler(); // TODO: is it safe to call within mutex lock?
        }
        handler = _requestHandler;
    }
    else
    {
        handler = _reference->getRequestHandler();
    }

    AsyncStatus status = handler->sendAsyncRequest(outgoingAsync);
    return pair(status, handler);
}

RequestHandlerPtr
ProxyInvoker::getRequestHandler()
{
    lock_guard lock(_mutex);
    return _requestHandler;
}

void
ProxyInvoker::clearRequestHandler(const RequestHandlerPtr& previous)
{
    if(_reference->getCacheConnection())
    {
        lock_guard lock(_mutex);
        if(_requestHandler && _requestHandler.get() == previous.get())
        {
            _requestHandler = nullptr;
        }
    }
}

int
ProxyInvoker::handleException(const Exception& ex,
                              const RequestHandlerPtr& handler,
                              OperationMode mode,
                              bool sent,
                              int& cnt)
{
    clearRequestHandler(handler); // Clear the request handler

    //
    // We only retry local exception, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    //  of the ProxyFactory class for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    const LocalException* localEx = dynamic_cast<const LocalException*>(&ex);
    if(localEx && (!sent ||
                   mode == OperationMode::Nonmutating || mode == OperationMode::Idempotent ||
                   dynamic_cast<const CloseConnectionException*>(&ex) ||
                   dynamic_cast<const ObjectNotExistException*>(&ex)))
    {
        try
        {
            return _reference->getInstance()->proxyFactory()->checkRetryAfterException(*localEx, _reference, cnt);
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // The communicator is already destroyed, so we cannot retry.
            //
            ex.ice_throw();
        }
    }
    else
    {
        ex.ice_throw(); // Retry could break at-most-once semantics, don't retry.
    }
    return 0; // Keep the compiler happy.
}
