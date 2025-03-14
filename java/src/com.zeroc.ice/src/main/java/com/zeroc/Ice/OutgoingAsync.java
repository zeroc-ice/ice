// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;

/**
 * @hidden Public because it's used by the generated code.
 */
public class OutgoingAsync<T> extends ProxyOutgoingAsyncBase<T> {
    @FunctionalInterface
    public static interface Unmarshaler<V> {
        V unmarshal(InputStream istr);
    }

    public OutgoingAsync(
            ObjectPrx prx,
            String operation,
            OperationMode mode,
            boolean synchronous,
            Class<?>[] userExceptions) {
        super((_ObjectPrxI) prx, operation, null);
        _mode = mode == null ? OperationMode.Normal : mode;
        _synchronous = synchronous;
        _userExceptions = userExceptions;
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());

        if (_instance.cacheMessageBuffers() > 0) {
            _ObjectPrxI.StreamPair p = _proxy._getCachedMessageBuffers();
            if (p != null) {
                _is = p.is;
                _os = p.os;
            }
        }
        if (_os == null) {
            _os =
                    new OutputStream(
                            Protocol.currentProtocolEncoding,
                            _instance.defaultsAndOverrides().defaultFormat,
                            _instance.cacheMessageBuffers() > 1);
        }
    }

    public void invoke(
            boolean twowayOnly,
            java.util.Map<String, String> ctx,
            FormatType format,
            OutputStream.Marshaler marshal,
            Unmarshaler<T> unmarshal) {
        _unmarshal = unmarshal;

        if (twowayOnly && !_proxy.ice_isTwoway()) {
            throw new TwowayOnlyException(_operation);
        }

        try {
            prepare(ctx);

            if (marshal == null) {
                writeEmptyParams();
            } else {
                marshal.marshal(startWriteParams(format));
                endWriteParams();
            }

            if (isBatch()) {
                // NOTE: we don't call sent/completed callbacks for batch AMI requests
                _sentSynchronously = true;
                _proxy._getReference()
                        .getBatchRequestQueue()
                        .finishBatchRequest(_os, _proxy, _operation);
                finished(true, false);
            } else {
                // invokeImpl can throw; we handle this exception by calling abort
                invokeImpl(true); // userThread = true
            }
        } catch (LocalException ex) {
            abort(ex);
        }
    }

    @Override
    public T waitForResponse() {
        if (isBatch()) {
            return null; // The future will not be completed for a batch invocation.
        }

        try {
            return waitForResponseOrUserEx();
        } catch (UserException ex) {
            throw UnknownUserException.fromTypeId(ex.ice_id());
        }
    }

    public T waitForResponseOrUserEx() throws UserException {
        try {
            return get();
        } catch (InterruptedException ex) {
            throw new OperationInterruptedException(ex);
        } catch (java.util.concurrent.ExecutionException ee) {
            try {
                throw ee.getCause().fillInStackTrace();
            } catch (RuntimeException ex) // Includes LocalException
            {
                throw ex;
            } catch (UserException ex) {
                throw ex;
            } catch (Throwable ex) {
                throw new UnknownException(ex);
            }
        }
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
        // timeout set.
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
        try {
            if (!_proxy.ice_isTwoway()) {
                //
                // For a non-twoway proxy, the invocation is completed after it is sent.
                //
                complete(null);
            } else if ((_state & StateOK) > 0) {
                T r = null;
                try {
                    if (_unmarshal != null) {
                        //
                        // The Unmarshaler callback unmarshals and returns the results.
                        //
                        r = _unmarshal.unmarshal(startReadParams());
                        endReadParams();
                    } else {
                        readEmptyParams();
                    }
                } catch (LocalException ex) {
                    completeExceptionally(ex);
                    return;
                }
                complete(r);
            } else {
                //
                // Handle user exception.
                //
                try {
                    throwUserException();
                } catch (UserException ex) {
                    if (_userExceptions != null) {
                        for (int i = 0; i < _userExceptions.length; ++i) {
                            if (_userExceptions[i].isInstance(ex)) {
                                completeExceptionally(ex);
                                return;
                            }
                        }
                    }
                    completeExceptionally(UnknownUserException.fromTypeId(ex.ice_id()));
                } catch (Throwable ex) {
                    completeExceptionally(ex);
                }
            }
        } finally {
            cacheMessageBuffers();
        }
    }

    @Override
    public final boolean completed(InputStream is) {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke any user callbacks.
        //

        // _is can already be initialized if the invocation is retried
        if (_is == null) {
            _is =
                    new InputStream(
                            _instance,
                            Protocol.currentProtocolEncoding,
                            _instance.cacheMessageBuffers() > 1);
        }
        _is.swap(is);

        return super.completed(_is);
    }

    private OutputStream startWriteParams(FormatType format) {
        _os.startEncapsulation(_encoding, format);
        return _os;
    }

    private void endWriteParams() {
        _os.endEncapsulation();
    }

    private void writeEmptyParams() {
        _os.writeEmptyEncapsulation(_encoding);
    }

    private InputStream startReadParams() {
        _is.startEncapsulation();
        return _is;
    }

    private void endReadParams() {
        _is.endEncapsulation();
    }

    private void readEmptyParams() {
        _is.skipEmptyEncapsulation();
    }

    private final void throwUserException() throws UserException {
        try {
            _is.startEncapsulation();
            _is.throwException(null);
        } catch (UserException ex) {
            _is.endEncapsulation();
            throw ex;
        }
    }

    @Override
    protected void cacheMessageBuffers() {
        if (_instance.cacheMessageBuffers() > 0) {
            synchronized (this) {
                if ((_state & StateCachedBuffers) > 0) {
                    return;
                }
                _state |= StateCachedBuffers;
            }

            if (_is != null) {
                _is.reset();
            }
            _os.reset();

            _proxy._cacheMessageBuffers(_is, _os);

            _is = null;
            _os = null;
        }
    }

    private final EncodingVersion _encoding;
    private InputStream _is;

    private Class<?>[] _userExceptions; // Valid user exceptions.
    private Unmarshaler<T> _unmarshal;
}
