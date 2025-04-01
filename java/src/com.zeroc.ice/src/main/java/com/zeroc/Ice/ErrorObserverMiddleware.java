// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletionException;
import java.util.concurrent.CompletionStage;
import java.util.function.Consumer;

/**
 * Provides a simple middleware that allows applications to observe java.lang.Error thrown by the
 * dispatch of an incoming request.
 */
public final class ErrorObserverMiddleware implements Object {
    private final Object _next;
    private final Consumer<Error> _errorObserver;

    /**
     * Constructs a new ErrorObserverMiddleware.
     *
     * @param next The next dispatcher in the chain.
     * @param errorObserver The error observer. If error observer throws an exception while
     *     observing an error, this exception replaces the error for the remainder of the dispatch.
     */
    public ErrorObserverMiddleware(Object next, Consumer<Error> errorObserver) {
        _next = next;
        _errorObserver = errorObserver;
    }

    @Override
    public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
        try {
            return _next.dispatch(request)
                    .exceptionally(
                            exception -> {
                                // _errorObserver can throw an exception that effectively replaces
                                // exception for dependent completion stages.
                                if (exception instanceof Error error) {
                                    _errorObserver.accept(error);
                                    throw error;
                                } else if (exception instanceof
                                                CompletionException completionException &&
                                        completionException.getCause() instanceof Error error) {
                                    // This can occur when the closure of a parent completion stage
                                    // throws an error.
                                    _errorObserver.accept(error);
                                }

                                if (exception instanceof RuntimeException runtimeException) {
                                    // Rethrow as-is. Note that Java does not wrap
                                    // CompletionException in CompletionException.
                                    throw runtimeException;
                                }

                                // exception is a Throwable that is not an Error or a
                                // RuntimeException. We can't throw it so we marshal this exception
                                // into a response.
                                return request.current.createOutgoingResponse(exception);
                            });
        } catch (Error error) {
            // synchronous error
            _errorObserver.accept(error);
            throw error;
        }
    }
}
