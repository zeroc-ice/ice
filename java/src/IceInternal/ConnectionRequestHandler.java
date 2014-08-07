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
    prepareBatchRequest(BasicStream out) throws RetryException {
        _connection.prepareBatchRequest(out);
    }

    @Override
    public void
    finishBatchRequest(BasicStream out) {
        _connection.finishBatchRequest(out, _compress);
    }

    @Override
    public void
    abortBatchRequest() {
        _connection.abortBatchRequest();
    }

    @Override
    public boolean
    sendRequest(OutgoingMessageCallback out)
            throws RetryException {
        //
        // Finished if sent and no response.
        //
        return out.send(_connection, _compress, _response) && !_response;
    }

    @Override
    public int
    sendAsyncRequest(OutgoingAsyncMessageCallback out)
            throws RetryException {
        return out.__send(_connection, _compress, _response);
    }

    @Override
    public boolean
    requestCanceled(OutgoingMessageCallback out, Ice.LocalException ex)
    {
        return _connection.requestCanceled(out, ex);
    }

    @Override
    public boolean
    asyncRequestCanceled(OutgoingAsyncMessageCallback outgoingAsync, Ice.LocalException ex)
    {
        return _connection.asyncRequestCanceled(outgoingAsync, ex);
    }

    @Override
    public Reference
    getReference()
    {
        return _reference;
    }

    @Override
    public Ice.ConnectionI
    getConnection()
    {
        return _connection;
    }

    @Override
    public Ice.ConnectionI
    waitForConnection()
    {
        return _connection;
    }

    public ConnectionRequestHandler(Reference ref, Ice.ConnectionI connection,
            boolean compress) {
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
