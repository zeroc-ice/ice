// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.InvocationObserver;

public class ConnectionRequestHandler implements RequestHandler
{
    public void
    prepareBatchRequest(BasicStream out)
        throws LocalExceptionWrapper
    {
        _connection.prepareBatchRequest(out);
    }

    public void
    finishBatchRequest(BasicStream out)
    {
        _connection.finishBatchRequest(out, _compress);
    }

    public void
    abortBatchRequest()
    {
        _connection.abortBatchRequest();
    }

    public Ice.ConnectionI
    sendRequest(Outgoing out)
        throws LocalExceptionWrapper
    {
        if(!_connection.sendRequest(out, _compress, _response) || _response)
        {
            return _connection; // The request has been sent or we're expecting a response.
        }
        else
        {
            return null; // The request has been sent.
        }
    }

    public int
    sendAsyncRequest(OutgoingAsync out)
        throws LocalExceptionWrapper
    {
        return _connection.sendAsyncRequest(out, _compress, _response);
    }

    public boolean
    flushBatchRequests(BatchOutgoing out)
    {
        return _connection.flushBatchRequests(out);
    }

    public int
    flushAsyncBatchRequests(BatchOutgoingAsync out)
    {
        return _connection.flushAsyncBatchRequests(out);
    }

    public Outgoing
    getOutgoing(String operation, Ice.OperationMode mode, java.util.Map<String, String> context, 
                InvocationObserver observer)
        throws LocalExceptionWrapper
    {
        return _connection.getOutgoing(this, operation, mode, context, observer);
    }

    public void
    reclaimOutgoing(Outgoing out)
    {
        _connection.reclaimOutgoing(out);
    }

    public Reference 
    getReference()
    {
        return _reference;
    }

    public Ice.ConnectionI
    getConnection(boolean wait)
    {
        return _connection;
    }

    public 
    ConnectionRequestHandler(Reference ref, Ice.ObjectPrx proxy)
    {
        _reference = ref;
        _response = _reference.getMode() == Reference.ModeTwoway;

        Ice.BooleanHolder compress = new Ice.BooleanHolder();
        _connection = _reference.getConnection(compress);
        _compress = compress.value;

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        IceInternal.RouterInfo ri = _reference.getRouterInfo();
        if(ri != null)
        {
            ri.addProxy(proxy);
        }
    }

    public
    ConnectionRequestHandler(Reference ref, Ice.ConnectionI connection, boolean compress)
    {
        _reference = ref;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _connection = connection;
        _compress = compress;
    }

    private final Reference _reference;
    private final boolean _response;
    private final Ice.ConnectionI _connection;
    private final boolean _compress;
}
