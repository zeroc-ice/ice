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
import com.zeroc.Ice._ObjectPrxI;
import com.zeroc.Ice.OperationInterruptedException;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.UserException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownUserException;

public class OutgoingAsync<T> extends ProxyOutgoingAsyncBaseI<T>
{
    @FunctionalInterface
    static public interface Unmarshaler<V>
    {
        V unmarshal(com.zeroc.Ice.InputStream istr);
    }

    public OutgoingAsync(com.zeroc.Ice.ObjectPrx prx, String operation, OperationMode mode, boolean synchronous,
                         Class<?>[] userExceptions)
    {
        super((_ObjectPrxI)prx, operation, null);
        _mode = mode == null ? OperationMode.Normal : mode;
        _synchronous = synchronous;
        _userExceptions = userExceptions;
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());

        if(_instance.cacheMessageBuffers() > 0)
        {
            _ObjectPrxI.StreamPair p = _proxy._getCachedMessageBuffers();
            if(p != null)
            {
                _is = p.is;
                _os = p.os;
            }
        }
        if(_os == null)
        {
            _os = new com.zeroc.Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
        }
    }

    public void invoke(boolean twowayOnly, java.util.Map<String, String> ctx, FormatType format,
                       OutputStream.Marshaler marshal, Unmarshaler<T> unmarshal)
    {
        _unmarshal = unmarshal;

        if(twowayOnly && !_proxy.ice_isTwoway())
        {
            throw new com.zeroc.Ice.TwowayOnlyException(_operation);
        }

        if(format == null)
        {
            format = FormatType.DefaultFormat;
        }

        try
        {
            prepare(ctx);

            if(marshal == null)
            {
                writeEmptyParams();
            }
            else
            {
                marshal.marshal(startWriteParams(format));
                endWriteParams();
            }

            if(isBatch())
            {
                //
                // NOTE: we don't call sent/completed callbacks for batch AMI requests
                //
                _sentSynchronously = true;
                _proxy._getBatchRequestQueue().finishBatchRequest(_os, _proxy, _operation);
                finished(true, false);
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

    public T waitForResponse()
    {
        if(isBatch())
        {
            return null; // The future will not be completed for a batch invocation.
        }

        try
        {
            return waitForResponseOrUserEx();
        }
        catch(UserException ex)
        {
            throw new UnknownUserException(ex.ice_id(), ex);
        }
    }

    public T waitForResponseOrUserEx()
        throws UserException
    {
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
            catch(UserException ex)
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
        try
        {
            if(!_proxy.ice_isTwoway())
            {
                //
                // For a non-twoway proxy, the invocation is completed after it is sent.
                //
                complete(null);
            }
            else if((_state & StateOK) > 0)
            {
                T r = null;
                try
                {
                    if(_unmarshal != null)
                    {
                        //
                        // The Unmarshaler callback unmarshals and returns the results.
                        //
                        r = _unmarshal.unmarshal(startReadParams());
                        endReadParams();
                    }
                    else
                    {
                        readEmptyParams();
                    }
                }
                catch(com.zeroc.Ice.LocalException ex)
                {
                    completeExceptionally(ex);
                    return;
                }
                complete(r);
            }
            else
            {
                //
                // Handle user exception.
                //
                try
                {
                    throwUserException();
                }
                catch(UserException ex)
                {
                    if(_userExceptions != null)
                    {
                        for(int i = 0; i < _userExceptions.length; ++i)
                        {
                            if(_userExceptions[i].isInstance(ex))
                            {
                                completeExceptionally(ex);
                                return;
                            }
                        }
                    }
                    completeExceptionally(new com.zeroc.Ice.UnknownUserException(ex.ice_id(), ex));
                }
                catch(Throwable ex)
                {
                    completeExceptionally(ex);
                }
            }
        }
        finally
        {
            cacheMessageBuffers();
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

    private com.zeroc.Ice.OutputStream startWriteParams(FormatType format)
    {
        _os.startEncapsulation(_encoding, format);
        return _os;
    }

    private void endWriteParams()
    {
        _os.endEncapsulation();
    }

    private void writeEmptyParams()
    {
        _os.writeEmptyEncapsulation(_encoding);
    }

    private com.zeroc.Ice.InputStream startReadParams()
    {
        _is.startEncapsulation();
        return _is;
    }

    private void endReadParams()
    {
        _is.endEncapsulation();
    }

    private void readEmptyParams()
    {
        _is.skipEmptyEncapsulation();
    }

    private final void throwUserException()
        throws com.zeroc.Ice.UserException
    {
        try
        {
            _is.startEncapsulation();
            _is.throwException(null);
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            _is.endEncapsulation();
            throw ex;
        }
    }

    @Override
    protected void cacheMessageBuffers()
    {
        if(_instance.cacheMessageBuffers() > 0)
        {
            synchronized(this)
            {
                if((_state & StateCachedBuffers) > 0)
                {
                    return;
                }
                _state |= StateCachedBuffers;
            }

            if(_is != null)
            {
                _is.reset();
            }
            _os.reset();

            _proxy._cacheMessageBuffers(_is, _os);

            _is = null;
            _os = null;
        }
    }

    final private com.zeroc.Ice.EncodingVersion _encoding;
    private com.zeroc.Ice.InputStream _is;

    private Class<?>[] _userExceptions; // Valid user exceptions.
    private Unmarshaler<T> _unmarshal;
}
