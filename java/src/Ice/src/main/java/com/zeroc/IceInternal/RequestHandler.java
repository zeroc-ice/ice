//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

interface RequestHandler extends CancellationHandler {
    int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException;

    ConnectionI getConnection();
}
