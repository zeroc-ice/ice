// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Instrumentation;

namespace Ice.Internal;

/// <summary>
/// A middleware that observes dispatches.
/// </summary>
internal sealed class ObserverMiddleware : Object
{
    private readonly Object _next;
    private readonly CommunicatorObserver _communicatorObserver;

    public ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        return _communicatorObserver.getDispatchObserver(request.current, request.size) is DispatchObserver observer ?
            performDispatchAsync(observer) :
            _next.dispatchAsync(request);

        async ValueTask<OutgoingResponse> performDispatchAsync(DispatchObserver observer)
        {
            observer.attach();
            try
            {
                OutgoingResponse response;
                try
                {
                    response = await _next.dispatchAsync(request).ConfigureAwait(false);
                }
                catch (System.Exception ex)
                {
                    // We need to marshal the exception into the response immediately to observe the response size.
                    response = request.current.createOutgoingResponse(ex);
                }

                switch (response.replyStatus)
                {
                    case ReplyStatus.Ok:
                        // don't do anything
                        break;

                    case ReplyStatus.UserException:
                        observer.userException();
                        break;

                    default:
                        observer.failed(response.exceptionId ?? "");
                        break;
                }

                if (request.current.requestId != 0)
                {
                    observer.reply(response.size);
                }

                return response;
            }
            finally
            {
                observer.detach();
            }
        }
    }

    internal ObserverMiddleware(Object next, CommunicatorObserver communicatorObserver)
    {
        _next = next;
        _communicatorObserver = communicatorObserver;
    }
}
