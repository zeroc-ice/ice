// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public interface RequestHandler extends CancellationHandler
{
    RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler);

    int sendAsyncRequest(ProxyOutgoingAsyncBase out)
        throws RetryException;

    Reference getReference();

    com.zeroc.Ice.ConnectionI getConnection();
}
