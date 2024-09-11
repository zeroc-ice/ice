//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface RequestHandler extends CancellationHandler {
    int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException;

    com.zeroc.Ice.ConnectionI getConnection();
}
