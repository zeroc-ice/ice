// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public interface RequestHandler extends CancellationHandler
{
    RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

    int sendAsyncRequest(ProxyOutgoingAsyncBase out)
        throws RetryException;

    Reference getReference();

    Ice.ConnectionI getConnection();
}
