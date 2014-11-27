// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProxyFlushBatch extends ProxyOutgoingAsyncBase
{
    public static ProxyFlushBatch check(Ice.AsyncResult r, Ice.ObjectPrx prx, String operation)
    {
        ProxyOutgoingAsyncBase.checkImpl(r, prx, operation);
        try
        {
            return (ProxyFlushBatch)r;
        }
        catch(ClassCastException ex)
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
    }

    public ProxyFlushBatch(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase callback)
    {
        super(prx, operation, callback);
        _observer = ObserverHelper.get(prx, operation);
    }

    @Override
    public int send(Ice.ConnectionI connection, boolean compress, boolean response)
    {
        _cachedConnection = connection;
        return connection.flushAsyncBatchRequests(this);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        return handler.invokeAsyncBatchRequests(this);
    }

    public void invoke()
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy.__reference().getProtocol()));
        invokeImpl(true); // userThread = true
    }
    
    @Override
    protected void handleRetryException(Ice.Exception exc)
    {
        _proxy.__setRequestHandler(_handler, null); // Clear request handler
        throw exc; // No retries, we want to notify the user of potentially lost batch requests
    }
    
    @Override
    protected int handleException(Ice.Exception exc)
    {
        _proxy.__setRequestHandler(_handler, null); // Clear request handler
        throw exc; // No retries, we want to notify the user of potentially lost batch requests
    }
}
