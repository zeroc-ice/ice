// Copyright (c) ZeroC, Inc.

#include "ObserverMiddleware.h"
#include "Ice/UserException.h"

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

ObserverMiddleware::ObserverMiddleware(Ice::ObjectPtr next, CommunicatorObserverPtr communicatorObserver)
    : _next(std::move(next)),
      _communicatorObserver(std::move(communicatorObserver))
{
    assert(_next);
    assert(_communicatorObserver);
}

void
ObserverMiddleware::dispatch(Ice::IncomingRequest& request, function<void(OutgoingResponse)> sendResponse)
{
    DispatchObserverPtr observerPtr = _communicatorObserver->getDispatchObserver(request.current(), request.size());
    if (observerPtr)
    {
        observerPtr->attach();
        exception_ptr exception;

        try
        {
            // We can't move sendResponse as we need it when we catch an exception; so we copy it.
            _next->dispatch(
                request,
                [sendResponse, observerPtr](OutgoingResponse response)
                {
                    switch (response.replyStatus())
                    {
                        case ReplyStatus::Ok:
                            // don't do anything
                            break;
                        case ReplyStatus::UserException:
                            observerPtr->userException();
                            break;
                        default:
                            observerPtr->failed(response.exceptionId());
                            break;
                    }

                    if (response.current().requestId != 0)
                    {
                        observerPtr->reply(response.size());
                    }

                    observerPtr->detach();
                    sendResponse(std::move(response));
                });
        }
        catch (const UserException&)
        {
            observerPtr->userException();
            exception = current_exception();
        }
        catch (const Ice::Exception& ex)
        {
            observerPtr->failed(ex.ice_id());
            exception = current_exception();
        }
        catch (const std::exception& ex)
        {
            observerPtr->failed(ex.what());
            exception = current_exception();
        }
        catch (...)
        {
            observerPtr->failed("unknown");
            exception = current_exception();
        }

        if (exception)
        {
            OutgoingResponse response = makeOutgoingResponse(exception, request.current());
            if (response.current().requestId != 0)
            {
                observerPtr->reply(response.size());
            }
            observerPtr->detach();

            // If we get a synchronous exception, sendResponse was not called.
            sendResponse(std::move(response));
        }
    }
    else
    {
        // Not observing dispatches.
        _next->dispatch(request, std::move(sendResponse));
    }
}
