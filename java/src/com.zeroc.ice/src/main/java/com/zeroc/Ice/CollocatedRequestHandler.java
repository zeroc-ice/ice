//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.time.Duration;
import java.util.concurrent.CompletionStage;

final class CollocatedRequestHandler implements RequestHandler {
    private class InvokeAllAsync extends RunnableThreadPoolWorkItem {
        private InvokeAllAsync(
                OutgoingAsyncBase outAsync, OutputStream os, int requestId, int batchRequestNum) {
            _outAsync = outAsync;
            _os = os;
            _requestId = requestId;
            _batchRequestNum = batchRequestNum;
        }

        @Override
        public void run() {
            if (sentAsync(_outAsync)) {
                dispatchAll(_os, _requestId, _batchRequestNum);
            }
        }

        private final OutgoingAsyncBase _outAsync;
        private OutputStream _os;
        private final int _requestId;
        private final int _batchRequestNum;
    }

    public CollocatedRequestHandler(Reference reference, ObjectAdapter adapter) {
        _reference = reference;
        _executor = reference.getInstance().initializationData().executor != null;
        _adapter = adapter;
        _response = _reference.isTwoway();

        _logger =
                _reference
                        .getInstance()
                        .initializationData()
                        .logger; // Cached for better performance.
        _traceLevels = _reference.getInstance().traceLevels(); // Cached for better performance.
        _requestId = 0;
    }

    @Override
    public int sendAsyncRequest(ProxyOutgoingAsyncBase outAsync) {
        return outAsync.invokeCollocated(this);
    }

    @Override
    public synchronized void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex) {
        Integer requestId = _sendAsyncRequests.remove(outAsync);
        if (requestId != null) {
            if (requestId > 0) {
                _asyncRequests.remove(requestId);
            }
            if (outAsync.completed(ex)) {
                outAsync.invokeCompletedAsync();
            }
            _adapter.decDirectCount(); // dispatchAll won't be called, decrease the direct count.
            return;
        }

        if (outAsync instanceof OutgoingAsync) {
            for (java.util.Map.Entry<Integer, OutgoingAsyncBase> e : _asyncRequests.entrySet()) {
                if (e.getValue() == outAsync) {
                    _asyncRequests.remove(e.getKey());
                    if (outAsync.completed(ex)) {
                        outAsync.invokeCompletedAsync();
                    }
                    return;
                }
            }
        }
    }

    @Override
    public ConnectionI getConnection() {
        return null;
    }

    int invokeAsyncRequest(OutgoingAsyncBase outAsync, int batchRequestNum, boolean sync) {
        //
        // Increase the direct count to prevent the thread pool from being destroyed before
        // dispatchAll is called. This will also throw if the object adapter has been deactivated.
        //
        _adapter.incDirectCount();

        int requestId = 0;
        try {
            synchronized (this) {
                outAsync.cancelable(this); // This will throw if the request is canceled

                if (_response) {
                    requestId = ++_requestId;
                    _asyncRequests.put(requestId, outAsync);
                }

                _sendAsyncRequests.put(outAsync, requestId);
            }

            outAsync.attachCollocatedObserver(_adapter, requestId);

            if (!sync
                    || !_response
                    || _reference.getInvocationTimeout().compareTo(Duration.ZERO) > 0) {
                _adapter.getThreadPool()
                        .dispatch(
                                new InvokeAllAsync(
                                        outAsync, outAsync.getOs(), requestId, batchRequestNum));
            } else if (_executor) {
                _adapter.getThreadPool()
                        .executeFromThisThread(
                                new InvokeAllAsync(
                                        outAsync, outAsync.getOs(), requestId, batchRequestNum));
            } else {
                // Optimization: directly call dispatchAll if there's no executor.
                if (sentAsync(outAsync)) {
                    dispatchAll(outAsync.getOs(), requestId, batchRequestNum);
                }
            }
        } catch (Exception ex) {
            // Decrement the direct count if any exception is thrown synchronously.
            _adapter.decDirectCount();
            throw ex;
        }
        return AsyncStatus.Queued;
    }

    private boolean sentAsync(final OutgoingAsyncBase outAsync) {
        synchronized (this) {
            if (_sendAsyncRequests.remove(outAsync) == null) {
                return false; // The request timed-out.
            }

            //
            // This must be called within the synchronization to
            // ensure completed(ex) can't be called concurrently if
            // the request is canceled.
            //
            if (!outAsync.sent()) {
                return true;
            }
        }

        outAsync.invokeSent();
        return true;
    }

    private void dispatchAll(OutputStream os, int requestId, int requestCount) {
        if (_traceLevels.protocol >= 1) {
            fillInValue(os, 10, os.size());
            if (requestId > 0) {
                fillInValue(os, Protocol.headerSize, requestId);
            } else if (requestCount > 0) {
                fillInValue(os, Protocol.headerSize, requestCount);
            }
            TraceUtil.traceSend(os, _reference.getInstance(), null, _logger, _traceLevels);
        }

        var is = new InputStream(_reference.getInstance(), os.getEncoding(), os.getBuffer(), false);

        if (requestCount > 0) {
            is.pos(Protocol.requestBatchHdr.length);
        } else {
            is.pos(Protocol.requestHdr.length);
        }

        int dispatchCount = requestCount > 0 ? requestCount : 1;
        assert !_response || dispatchCount == 1;

        com.zeroc.Ice.Object dispatcher = _adapter.dispatchPipeline();
        assert dispatcher != null;

        try {
            while (dispatchCount > 0) {
                //
                // Increase the direct count for the dispatch. We increase it again here for
                // each dispatch. It's important for the direct count to be > 0 until the last
                // collocated request response is sent to make sure the thread pool isn't
                // destroyed before.
                //
                try {
                    _adapter.incDirectCount();
                } catch (ObjectAdapterDestroyedException ex) {
                    handleException(ex, requestId, false);
                    break;
                }

                var request = new IncomingRequest(requestId, null, _adapter, is);
                CompletionStage<OutgoingResponse> response = null;
                try {
                    response = dispatcher.dispatch(request);
                } catch (Throwable ex) { // UserException or an unchecked exception
                    sendResponse(request.current.createOutgoingResponse(ex), requestId, false);
                }

                if (response != null) {
                    response.whenComplete(
                            (result, exception) -> {
                                if (exception != null) {
                                    sendResponse(
                                            request.current.createOutgoingResponse(exception),
                                            requestId,
                                            true);
                                } else {
                                    sendResponse(result, requestId, true);
                                }
                                // Any exception thrown by this closure is effectively ignored.
                            });
                }

                --dispatchCount;
            }
            is.clear();
        } catch (LocalException ex) {
            dispatchException(ex, requestId, false); // Fatal dispatch exception
        } catch (RuntimeException | Error ex) {
            // A runtime exception or an error was thrown outside of servant code (i.e., by Ice
            // code).
            // Note that this code does NOT send a response to the client.
            var uex = new UnknownException(ex);
            var sw = new java.io.StringWriter();
            var pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();

            _logger.error(sw.toString());
            dispatchException(uex, requestId, false);
        } finally {
            _adapter.decDirectCount();
        }
    }

    private void sendResponse(OutgoingResponse response, int requestId, boolean amd) {
        if (_response) {
            OutgoingAsyncBase outAsync = null;
            OutputStream outputStream = response.outputStream;

            synchronized (this) {
                if (_traceLevels.protocol >= 1) {
                    fillInValue(outputStream, 10, outputStream.size());
                }

                // Adopt the OutputStream's buffer.
                var inputStream =
                        new InputStream(
                                _reference.getInstance(),
                                outputStream.getEncoding(),
                                outputStream.getBuffer(),
                                true); // adopt: true

                inputStream.pos(Protocol.replyHdr.length + 4);

                if (_traceLevels.protocol >= 1) {
                    TraceUtil.traceRecv(inputStream, null, _logger, _traceLevels);
                }

                outAsync = _asyncRequests.remove(requestId);
                if (outAsync != null && !outAsync.completed(inputStream)) {
                    outAsync = null;
                }
            }

            if (outAsync != null) {
                //
                // If called from an AMD dispatch, invoke asynchronously
                // the completion callback since this might be called from
                // the user code.
                //
                if (amd) {
                    outAsync.invokeCompletedAsync();
                } else {
                    outAsync.invokeCompleted();
                }
            }
        }
        _adapter.decDirectCount();
    }

    private void dispatchException(LocalException ex, int requestId, boolean amd) {
        handleException(ex, requestId, amd);
        _adapter.decDirectCount();
    }

    private void handleException(LocalException ex, int requestId, boolean amd) {
        if (requestId == 0) {
            return; // Ignore exception for oneway messages.
        }

        OutgoingAsyncBase outAsync = null;
        synchronized (this) {
            outAsync = _asyncRequests.remove(requestId);
            if (outAsync != null && !outAsync.completed(ex)) {
                outAsync = null;
            }
        }

        if (outAsync != null) {
            //
            // If called from an AMD dispatch, invoke asynchronously
            // the completion callback since this might be called from
            // the user code.
            //
            if (amd) {
                outAsync.invokeCompletedAsync();
            } else {
                outAsync.invokeCompleted();
            }
        }
    }

    private static void fillInValue(OutputStream os, int pos, int value) {
        os.rewriteInt(value, pos);
    }

    private final Reference _reference;
    private final boolean _executor;
    private final boolean _response;
    private final ObjectAdapter _adapter;
    private final Logger _logger;
    private final TraceLevels _traceLevels;

    private int _requestId;

    // A map of outstanding requests that can be canceled. A request
    // can be canceled if it has an invocation timeout, or we support
    // interrupts.
    private final java.util.Map<OutgoingAsyncBase, Integer> _sendAsyncRequests =
            new java.util.HashMap<>();

    private final java.util.Map<Integer, OutgoingAsyncBase> _asyncRequests =
            new java.util.HashMap<>();
}
