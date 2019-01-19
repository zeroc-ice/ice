//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface RequestHandler extends CancellationHandler
{
    RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

    int sendAsyncRequest(ProxyOutgoingAsyncBase out)
        throws RetryException;

    Reference getReference();

    com.zeroc.Ice.ConnectionI getConnection();
}
