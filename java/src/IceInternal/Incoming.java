// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.CommunicatorObserver;

final public class Incoming extends IncomingBase implements Ice.Request
{
    public
    Incoming(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress,
             int requestId)
    {
        super(instance, connection, adapter, response, compress, requestId);

        //
        // Prepare the response if necessary.
        //
        if(response)
        {
            _os.writeBlob(IceInternal.Protocol.replyHdr);
            
            //
            // Add the request ID.
            //
            _os.writeInt(requestId);
        }
    }

    //
    // Request implementation
    //
    public boolean 
    isCollocated()
    {
        return false;
    }
    
    public Ice.Current
    getCurrent()
    {
        return _current;
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress,
          int requestId)
    {
        _cb = null;
        _inParamPos = -1;
            
        super.reset(instance, connection, adapter, response, compress, requestId);

        //
        // Prepare the response if necessary.
        //
        if(response)
        {
            _os.writeBlob(IceInternal.Protocol.replyHdr);
            
            //
            // Add the request ID.
            //
            _os.writeInt(requestId);
        }
    }

    public void
    reclaim()
    {
        _cb = null;
        _inParamPos = -1;

        super.reclaim();
    }

    public void
    invoke(ServantManager servantManager, BasicStream stream)
    {
        _is = stream;

        int start = _is.pos();

        //
        // Read the current.
        //
        _current.id.__read(_is);

        //
        // For compatibility with the old FacetPath.
        //
        String[] facetPath = _is.readStringSeq();
        if(facetPath.length > 0)
        {
            if(facetPath.length > 1)
            {
                throw new Ice.MarshalException();
            }
            _current.facet = facetPath[0];
        }
        else
        {
            _current.facet = "";
        }

        _current.operation = _is.readString();
        _current.mode = Ice.OperationMode.values()[_is.readByte()];
        _current.ctx = new java.util.HashMap<String, String>();
        int sz = _is.readSize();
        while(sz-- > 0)
        {
            String first = _is.readString();
            String second = _is.readString();
            _current.ctx.put(first, second);
        }

        CommunicatorObserver obsv = _instance.getObserver();
        if(obsv != null)
        {
            // Read the parameter encapsulation size.
            int size = _is.readInt();
            _is.pos(_is.pos() - 4);

            _observer = obsv.getDispatchObserver(_current, _is.pos() - start + size);
            if(_observer != null)
            {
                _observer.attach();
            }
        }

        //
        // Don't put the code above into the try block below. Exceptions
        // in the code above are considered fatal, and must propagate to
        // the caller of this operation.
        //

        if(servantManager != null)
        {
            _servant = servantManager.findServant(_current.id, _current.facet);
            if(_servant == null)
            {
                _locator = servantManager.findServantLocator(_current.id.category);
                if(_locator == null && _current.id.category.length() > 0)
                {
                    _locator = servantManager.findServantLocator("");
                }

                if(_locator != null)
                {
                    try
                    {
                        _servant = _locator.locate(_current, _cookie);
                    }
                    catch(Ice.UserException ex)
                    {
                        Ice.EncodingVersion encoding = _is.skipEncaps(); // Required for batch requests.

                        if(_observer != null)
                        {
                            _observer.userException();
                        }

                        if(_response)
                        {
                            _os.writeByte(ReplyStatus.replyUserException);
                            _os.startWriteEncaps(encoding, Ice.FormatType.DefaultFormat);
                            _os.writeUserException(ex);
                            _os.endWriteEncaps();
                            if(_observer != null)
                            {
                                _observer.reply(_os.size() - Protocol.headerSize - 4);
                            }
                            _connection.sendResponse(_os, _compress);
                        }
                        else
                        {
                            _connection.sendNoResponse();
                        }

                        if(_observer != null)
                        {
                            _observer.detach();
                            _observer = null;
                        }
                        _connection = null;
                        return;
                    }
                    catch(java.lang.Exception ex)
                    {
                        _is.skipEncaps(); // Required for batch requests.
                        __handleException(ex);
                        return;
                    }
                }
            }
        }

        try
        {
            if(_servant != null)
            {
                if(_instance.useApplicationClassLoader())
                {
                    Thread.currentThread().setContextClassLoader(_servant.getClass().getClassLoader());
                }

                try
                {
                    //
                    // DispatchAsync is a "pseudo dispatch status", used internally only
                    // to indicate async dispatch.
                    //
                    if(_servant.__dispatch(this, _current) == Ice.DispatchStatus.DispatchAsync)
                    {
                        //
                        // If this was an asynchronous dispatch, we're done here.
                        //
                        return;
                    }
                }
                finally
                {
                    if(_instance.useApplicationClassLoader())
                    {
                        Thread.currentThread().setContextClassLoader(null);
                    }
                }
                
                if(_locator != null && !__servantLocatorFinished())
                {
                    return;
                }
            }
            else
            {
                //
                // Skip the input parameters, this is required for reading
                // the next batch request if dispatching batch requests.
                //
                _is.skipEncaps();
                
                if(servantManager != null && servantManager.hasServant(_current.id))
                {
                    throw new Ice.FacetNotExistException(_current.id, _current.facet, _current.operation);
                }
                else
                {
                    throw new Ice.ObjectNotExistException(_current.id, _current.facet, _current.operation);
                }
            }
        }
        catch(java.lang.Exception ex)
        {
            if(_servant != null && _locator != null && !__servantLocatorFinished())
            {
                return;
            }
            __handleException(ex);
            return;
        }
        
        //
        // Don't put the code below into the try block above. Exceptions
        // in the code below are considered fatal, and must propagate to
        // the caller of this operation.
        //

        assert(_connection != null);

        if(_response)
        {
            if(_observer != null)
            {
                _observer.reply(_os.size() - Protocol.headerSize - 4);
            }
            _connection.sendResponse(_os, _compress);
        }
        else
        {
            _connection.sendNoResponse();
        }

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
        _connection = null;
    }

    public final void
    push(Ice.DispatchInterceptorAsyncCallback cb)
    {
        if(_interceptorAsyncCallbackList == null)
        {
            _interceptorAsyncCallbackList = new java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback>();
        }
        
        _interceptorAsyncCallbackList.addFirst(cb);
    }
    
    public final void
    pop()
    {
        assert _interceptorAsyncCallbackList != null;
        _interceptorAsyncCallbackList.removeFirst();
    }

    public final void 
    startOver()
    {
        if(_inParamPos == -1)
        {
            //
            // That's the first startOver, so almost nothing to do
            //
            _inParamPos = _is.pos();
        }
        else
        {
            killAsync();
            
            //
            // Let's rewind _is and clean-up _os
            //
            _is.pos(_inParamPos);
            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); 
            }
        }
    }

    public final void
    killAsync()
    {
        //
        // Always runs in the dispatch thread
        //
        if(_cb != null)
        {
            //
            // May raise ResponseSentException
            //
            _cb.__deactivate(this);
            _cb = null;
        }
    }

    public final BasicStream
    startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is.startReadEncaps();
        return _is;
    }
    
    public final void
    endReadParams()
    {
        _is.endReadEncaps();
    }

    public final void
    readEmptyParams()
    {
        _current.encoding = _is.skipEmptyEncaps();
    }

    public final byte[]
    readParamEncaps()
    {
        _current.encoding = new Ice.EncodingVersion();
        return _is.readEncaps(_current.encoding);
    }

    final void
    setActive(IncomingAsync cb)
    {
        assert _cb == null;
        _cb = cb;
    }
 
    final boolean 
    isRetriable()
    {
        return _inParamPos != -1;
    }

    public Incoming next; // For use by ConnectionI.

    private BasicStream _is;

    private IncomingAsync _cb;
    private int _inParamPos = -1;
}
