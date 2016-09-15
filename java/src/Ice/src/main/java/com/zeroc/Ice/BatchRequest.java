// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

public interface BatchRequest
{
    /**
     * Confirms the queuing of the batch request.
     **/
    void enqueue();

    /**
     * The marshaled size of the request.
     * @return The marshaled size.
     **/
    int getSize();

    /**
     * The name of the operation.
     * @return The operation name.
     **/
    String getOperation();

    /**
     * The proxy used to invoke the batch request.
     * @return The proxy.
     **/
    ObjectPrx getProxy();
}
