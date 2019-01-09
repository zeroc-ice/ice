// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
     * Returns the marshaled size of the request.
     *
     * @return The marshaled size.
     **/
    int getSize();

    /**
     * Returns the name of the operation.
     *
     * @return The name of the operation.
     **/
    String getOperation();

    /**
     * Returns the proxy used to invoke the batch request.
     *
     * @return The proxy used to invoke the batch request.
     **/
    Ice.ObjectPrx getProxy();
};
