// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public interface BatchRequest
{
    /**
     * Confirms the queuing of the batch request.
     **/
    void enqueue();

    /**
     * The marshalled size of the request.
     **/
    int getSize();

    /**
     * The name of the operation
     **/
    String getOperation();

    /**
     * The proxy used to invoke the batch request.
     **/
    Ice.ObjectPrx getProxy();
};
