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
    public int invokeRemote(ConnectionI connection, boolean compress, boolean response)
            throws RetryException {
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
