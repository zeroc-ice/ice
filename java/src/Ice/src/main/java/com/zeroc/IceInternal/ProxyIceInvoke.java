// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.FormatType;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.OperationInterruptedException;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.UserException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownUserException;

public class ProxyIceInvoke extends ProxyOutgoingAsyncBaseI<com.zeroc.Ice.Object.Ice_invokeResult>
{
    public ProxyIceInvoke(com.zeroc.Ice.ObjectPrx prx, String operation, com.zeroc.Ice.OperationMode mode,
                          boolean synchronous)
    {
        super((com.zeroc.Ice._ObjectPrxI)prx, operation);
        _mode = mode == null ? com.zeroc.Ice.OperationMode.Normal : mode;
        _synchronous = synchronous;
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());
        _is = null;
    }

    public void invoke(byte[] inParams, java.util.Map<String, String> ctx)
    {
        try
        {
            prepare(ctx);
            writeParamEncaps(inParams);

            if(isBatch())
            {
                //
                // NOTE: we don't call sent/completed callbacks for batch AMI requests
                //
                _sentSynchronously = true;
                _proxy._getBatchRequestQueue().finishBatchRequest(_os, _proxy, _operation);
                finished(true);
            }
            else
            {
                //
                // NOTE: invokeImpl doesn't throw so this can be called from the
                // try block with the catch block calling abort() in case of an
                // exception.
                //
                invokeImpl(true); // userThread = true
            }
        }
        catch(com.zeroc.Ice.Exception ex)
        {
            abort(ex);
        }
    }

    public com.zeroc.Ice.Object.Ice_invokeResult waitForResponse()
    {
        if(isBatch())
        {
            //
            // The future will not be completed for a batch invocation.
            //
            return new com.zeroc.Ice.Object.Ice_invokeResult(true, new byte[0]);
        }

        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        try
        {
            return get();
        }
        catch(InterruptedException ex)
        {
            throw new OperationInterruptedException();
        }
        catch(java.util.concurrent.ExecutionException ee)
        {
            try
            {
                throw ee.getCause();
            }
            catch(RuntimeException ex) // Includes LocalException
            {
                throw ex;
            }
            catch(Throwable ex)
            {
                throw new UnknownException(ex);
            }
        }
    }

    @Override
    public boolean sent()
    {
        return sent(!_proxy.ice_isTwoway()); // done = true if not a two-way proxy (no response expected)
    }

    @Override
    public int invokeRemote(com.zeroc.Ice.ConnectionI connection, boolean compress, boolean response)
        throws RetryException
    {
        _cachedConnection = connection;
        return connection.sendAsyncRequest(this, compress, response, 0);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
        if(!_proxy.ice_isTwoway() || _proxy._getReference().getInvocationTimeout() > 0)
        {
            // Disable caching by marking the streams as cached!
            _state |= StateCachedBuffers;
        }
        return handler.invokeAsyncRequest(this, 0, _synchronous);
    }

    @Override
    public void abort(com.zeroc.Ice.Exception ex)
    {
        if(isBatch())
        {
            //
            // If we didn't finish a batch oneway or datagram request, we
            // must notify the connection about that we give up ownership
            // of the batch stream.
            //
            _proxy._getBatchRequestQueue().abortBatchRequest(_os);
        }

        super.abort(ex);
    }

    @Override
    protected void markCompleted()
    {
        if(!_proxy.ice_isTwoway())
        {
            //
            // For a non-twoway proxy, the invocation is completed after it is sent.
            //
            complete(new com.zeroc.Ice.Object.Ice_invokeResult(true, new byte[0]));
        }
        else
        {
            com.zeroc.Ice.Object.Ice_invokeResult r = new com.zeroc.Ice.Object.Ice_invokeResult();
            r.returnValue = (_state & StateOK) > 0;
            r.outParams = readParamEncaps();
            complete(r);
        }
    }

    @Override
    public final boolean completed(com.zeroc.Ice.InputStream is)
    {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //

        // _is can already be initialized if the invocation is retried
        if(_is == null)
        {
            _is = new com.zeroc.Ice.InputStream(_instance, Protocol.currentProtocolEncoding);
        }
        _is.swap(is);

        return super.completed(_is);
    }

    private void writeParamEncaps(byte[] encaps)
    {
        if(encaps == null || encaps.length == 0)
        {
            _os.writeEmptyEncapsulation(_encoding);
        }
        else
        {
            _os.writeEncapsulation(encaps);
        }
    }

    private byte[] readParamEncaps()
    {
        return _is.readEncapsulation(null);
    }

    final private com.zeroc.Ice.EncodingVersion _encoding;
    private com.zeroc.Ice.InputStream _is;

    private boolean _synchronous; // True if this AMI request is being used for a generated synchronous invocation.
}
