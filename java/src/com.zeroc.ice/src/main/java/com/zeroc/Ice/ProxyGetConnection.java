// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class ProxyGetConnection extends ProxyOutgoingAsyncBase<Connection> {
    public ProxyGetConnection(_ObjectPrxI prx) {
        super(prx, "ice_getConnection");
        _observer = ObserverHelper.get(prx, "ice_getConnection");
    }

    @Override
    protected void markCompleted() {
        complete(_cachedConnection);
    }

    @Override
    public boolean completed(InputStream is) {
        assert false;
        return false;
    }

    @Override
    public int invokeRemote(ConnectionI connection, boolean compress, boolean response) throws RetryException {
        // A fixed proxy is bound to a single connection: return this connection as is, even when it's closed. Only
        // a non-fixed proxy can establish a replacement connection.
        if (!_proxy.ice_isFixed()) {
            try {
                connection.throwException();
            } catch (LocalException ex) {
                // The connection is closed: throw RetryException so that the caller clears the cached request
                // handler and calls invokeRemote again with a new connection.
                throw new RetryException(ex);
            }
        }
        _cachedConnection = connection;
        if (finished(true, true)) {
            invokeCompletedAsync();
        }
        return AsyncStatus.Sent;
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler) {
        if (finished(true, true)) {
            invokeCompletedAsync();
        }
        return AsyncStatus.Sent;
    }

    public void invoke() {
        try {
            invokeImpl(true); // userThread = true
        } catch (LocalException ex) {
            abort(ex);
        }
    }
}
