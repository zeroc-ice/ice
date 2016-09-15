// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

//
// This non-generic base interface allows us to pass instances without causing "rawtypes" lint warnings.
//
public interface ProxyOutgoingAsyncBase extends OutgoingAsyncBase
{
    int invokeRemote(com.zeroc.Ice.ConnectionI con, boolean compress, boolean response)
        throws RetryException;

    int invokeCollocated(CollocatedRequestHandler handler);

    void retryException(com.zeroc.Ice.Exception ex);

    void retry();

    void abort(com.zeroc.Ice.Exception ex);
}
