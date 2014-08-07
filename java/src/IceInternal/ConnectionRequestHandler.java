// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionRequestHandler implements RequestHandler
{
    @Override
    public void
    prepareBatchRequest(BasicStream out)
        throws RetryException
    {
        _connection.prepareBatchRequest(out);
    }

    @Override
    public void
    finishBatchRequest(BasicStream out)
    {
        _connection.finishBatchRequest(out, _compress);
    }

    @Override
    public void
    abortBatchRequest()
    {
        _connection.abortBatchRequest();
    }

    @Override
    public boolean
    sendRequest(OutgoingMessageCallback out)
        throws RetryException
    {
        return out.send(_connection, _compress, _response) && !_response; // Finished if sent and no response
    }

    @Override
    public int
    sendAsyncRequest(OutgoingAsyncMessageCallback out)
        throws RetryException
    {
        return out.__send(_connection, _compress, _response);
    }

    @Override
    public void
    requestTimedOut(OutgoingMessageCallback out)
    {
        _connection.requestTimedOut(out);
    }

    @Override
    public void
    asyncRequestTimedOut(OutgoingAsyncMessageCallback outAsync)
    {
        _connection.asyncRequestTimedOut(outAsync);
    }

    @Override
    public Reference
    getReference()
    {
        return _reference;
    }

    @Override
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
