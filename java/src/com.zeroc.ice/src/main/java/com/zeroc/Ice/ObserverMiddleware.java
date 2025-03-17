// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.DispatchObserver;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

final class ObserverMiddleware implements Object {
    private final Object _next;
    private final CommunicatorObserver _communicatorObserver;

    public ObserverMiddleware(Object next, CommunicatorObserver communicatorObserver) {
        _next = next;
        _communicatorObserver = communicatorObserver;
    }

    @Override
    public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
        DispatchObserver observer =
                _communicatorObserver.getDispatchObserver(request.current, request.size);

        if (observer != null) {
            observer.attach();
            try {
                return _next.dispatch(request)
                        .handle(
                                (response, exception) -> {
                                    if (exception != null) {
                                        // We need to marshal the exception into the response
                                        // immediately to observe the response size. TODO: should we
                                        // really marshal/handle errors here?
                                        response =
                                                request.current.createOutgoingResponse(exception);
                                    }

                                    observeResponse(
                                            response, observer, request.current.requestId != 0);
                                    observer.detach();
                                    return response;
                                });
            } catch (UserException | RuntimeException | Error ex) {
                // Synchronous exception
                OutgoingResponse response = request.current.createOutgoingResponse(ex);
                observeResponse(response, observer, request.current.requestId != 0);
                observer.detach();
                return CompletableFuture.completedStage(response);
            }
        } else {
            return _next.dispatch(request);
        }
    }

    private void observeResponse(
            OutgoingResponse response, DispatchObserver observer, boolean isTwoWay) {
        var replyStatus = ReplyStatus.valueOf(response.replyStatus);
        if (replyStatus != null) {
            switch (replyStatus) {
                case Ok:
                    // don't do anything
                    break;

                case UserException:
                    observer.userException();
                    break;

                default:
                    observer.failed(response.exceptionId);
                    break;
            }
        } else {
            // Unknown reply status, like default case above.
            observer.failed(response.exceptionId);
        }

        if (isTwoWay) {
            observer.reply(response.size);
        }
    }
}
