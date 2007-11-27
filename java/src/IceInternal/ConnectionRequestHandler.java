// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionRequestHandler implements RequestHandler
{
    public void
    prepareBatchRequest(BasicStream out)
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
        return (!_connection.sendRequest(out, _compress, _response) || _response) ? _connection : null;
    }

    public void
    sendAsyncRequest(OutgoingAsync out)
    {
        try
        {
            _connection.sendAsyncRequest(out, _compress, _response);
        }
        catch(LocalExceptionWrapper ex)
        {
            out.__finished(ex);
        }
        catch(Ice.LocalException ex)
        {
            out.__finished(ex);
        }
    }

    public boolean
    flushBatchRequests(BatchOutgoing out)
    {
        return _connection.flushBatchRequests(out);
    }

    public void
    flushAsyncBatchRequests(BatchOutgoingAsync out)
    {
        try
        {
            _connection.flushAsyncBatchRequests(out);
        }
        catch(Ice.LocalException ex)
        {
            out.__finished(ex);
        }
    }

    public Outgoing
    getOutgoing(String operation, Ice.OperationMode mode, java.util.Map context)
        throws LocalExceptionWrapper
    {
        return _connection.getOutgoing(this, operation, mode, context);
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
