//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public final class ConnectionRequestHandler implements RequestHandler {
    @Override
    public int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException {
        return out.invokeRemote(_connection, _compress, _response);
    }

    @Override
    public void asyncRequestCanceled(
            OutgoingAsyncBase outgoingAsync, com.zeroc.Ice.LocalException ex) {
        _connection.asyncRequestCanceled(outgoingAsync, ex);
    }

    @Override
    public com.zeroc.Ice.ConnectionI getConnection() {
        return _connection;
    }

    public ConnectionRequestHandler(
            Reference reference, com.zeroc.Ice.ConnectionI connection, boolean compress) {
        _reference = reference;
        _response = _reference.isTwoway();
        _connection = connection;
        _compress = compress;
    }

    private final Reference _reference;
    private final boolean _response;
    private final com.zeroc.Ice.ConnectionI _connection;
    private final boolean _compress;
}
