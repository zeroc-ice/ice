//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.time.Duration;

class ProxyIceInvoke extends ProxyOutgoingAsyncBase<Object.Ice_invokeResult> {
    public ProxyIceInvoke(
            ObjectPrx prx, String operation, OperationMode mode, boolean synchronous) {
        super((_ObjectPrxI) prx, operation);
        _mode = mode == null ? OperationMode.Normal : mode;
        _synchronous = synchronous;
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());
        _is = null;
    }

    public void invoke(byte[] inParams, java.util.Map<String, String> ctx) {
        try {
            prepare(ctx);
            writeParamEncaps(inParams);

            if (isBatch()) {
                //
                // NOTE: we don't call sent/completed callbacks for batch AMI requests
                //
                _sentSynchronously = true;
                _proxy._getReference()
                        .getBatchRequestQueue()
                        .finishBatchRequest(_os, _proxy, _operation);
                finished(true, false);
            } else {
                //
                // NOTE: invokeImpl doesn't throw so this can be called from the
                // try block with the catch block calling abort() in case of an
                // exception.
                //
                invokeImpl(true); // userThread = true
            }
        } catch (LocalException ex) {
            abort(ex);
        }
    }

    public Object.Ice_invokeResult waitForResponse() {
        if (isBatch()) {
            //
            // The future will not be completed for a batch invocation.
            //
            return new Object.Ice_invokeResult(true, new byte[0]);
        }
        return super.waitForResponse();
    }

    @Override
    public boolean sent() {
        return sent(!_proxy.ice_isTwoway()); // done = true if not a two-way proxy (no response
        // expected)
    }

    @Override
    public int invokeRemote(ConnectionI connection, boolean compress, boolean response)
            throws RetryException {
        _cachedConnection = connection;
        return connection.sendAsyncRequest(this, compress, response, 0);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler) {
        // The stream cannot be cached if the proxy is not a twoway or there is an invocation
        // timeout
        // set.
        if (!_proxy.ice_isTwoway()
                || _proxy._getReference().getInvocationTimeout().compareTo(Duration.ZERO) > 0) {
            // Disable caching by marking the streams as cached!
            _state |= StateCachedBuffers;
        }
        return handler.invokeAsyncRequest(this, 0, _synchronous);
    }

    @Override
    public void abort(LocalException ex) {
        if (isBatch()) {
            //
            // If we didn't finish a batch oneway or datagram request, we
            // must notify the connection about that we give up ownership
            // of the batch stream.
            //
            _proxy._getReference().getBatchRequestQueue().abortBatchRequest(_os);
        }

        super.abort(ex);
    }

    @Override
    protected void markCompleted() {
        if (!_proxy.ice_isTwoway()) {
            //
            // For a non-twoway proxy, the invocation is completed after it is sent.
            //
            complete(new Object.Ice_invokeResult(true, new byte[0]));
        } else {
            Object.Ice_invokeResult r = new Object.Ice_invokeResult();
            r.returnValue = (_state & StateOK) > 0;
            r.outParams = readParamEncaps();
            complete(r);
        }
    }

    @Override
    public final boolean completed(InputStream is) {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //

        // _is can already be initialized if the invocation is retried
        if (_is == null) {
            _is = new InputStream(_instance);
        }
        _is.swap(is);

        return super.completed(_is);
    }

    private void writeParamEncaps(byte[] encaps) {
        if (encaps == null || encaps.length == 0) {
            _os.writeEmptyEncapsulation(_encoding);
        } else {
            _os.writeEncapsulation(encaps);
        }
    }

    private byte[] readParamEncaps() {
        return _is.readEncapsulation(null);
    }

    private final EncodingVersion _encoding;
    private InputStream _is;

    // True if this AMI request is being used for a generated synchronous invocation.
    private boolean _synchronous;
}
