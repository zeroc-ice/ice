// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

interface RequestHandler extends CancellationHandler {
    int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException;

    ConnectionI getConnection();
}
