// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Incoming extends IncomingBase implements Ice.Request
{
    public
    Incoming(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress,
             int requestId)
    {
        super(instance, connection, adapter, response, compress, requestId);

        _is = new BasicStream(instance);
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
            
        if(_is == null)
        {
            _is = new BasicStream(instance);
        }

        super.reset(instance, connection, adapter, response, compress, requestId);
    }

    public void
    reclaim()
    {
        _cb = null;
        _inParamPos = -1;

        if(_is != null)
        {
            _is.reset();
        }

        super.reclaim();
    }

    public void
    invoke(ServantManager servantManager)
    {
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

        if(_response)
        {
            assert(_os.size() == Protocol.headerSize + 4); // Reply status position.
            _os.writeByte(ReplyStatus.replyOK);
            _os.startWriteEncaps();
        }

        byte replyStatus = ReplyStatus.replyOK;

        Ice.DispatchStatus dispatchStatus = Ice.DispatchStatus.DispatchOK;        

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
                        _os.writeUserException(ex);
                        replyStatus = ReplyStatus.replyUserException;
                    }
                    catch(java.lang.Exception ex)
                    {
                        __handleException(ex);
                        return;
                    }
                }
            }
        }

        if(_servant != null)
        {
            try
            {
                assert(replyStatus == ReplyStatus.replyOK);
                dispatchStatus = _servant.__dispatch(this, _current);
                if(dispatchStatus == Ice.DispatchStatus.DispatchUserException)
                {
                    replyStatus = ReplyStatus.replyUserException;
                }        
                
                if(dispatchStatus != Ice.DispatchStatus.DispatchAsync)
                {
                    if(_locator != null && !__servantLocatorFinished())
                    {
                        return;
                    }
                }
            }
            catch(java.lang.Exception ex)
            {
                if(_locator != null && !__servantLocatorFinished())
                {
                    return;
                }
                __handleException(ex);
                return;
            }
        }
        else if(replyStatus == ReplyStatus.replyOK)
        {
            if(servantManager != null && servantManager.hasServant(_current.id))
            {
                replyStatus = ReplyStatus.replyFacetNotExist;
            }
            else
            {
                replyStatus = ReplyStatus.replyObjectNotExist;
            }
        }
        
        //
        // Don't put the code below into the try block above. Exceptions
        // in the code below are considered fatal, and must propagate to
        // the caller of this operation.
        //

        //
        // DispatchAsync is "pseudo dispatch status", used internally
        // only to indicate async dispatch.
        //
        if(dispatchStatus == Ice.DispatchStatus.DispatchAsync)
        {
            //
            // If this was an asynchronous dispatch, we're done here.
            //
            return;
        }

        assert(_connection != null);

        if(_response)
        {
            _os.endWriteEncaps();
            
            if(replyStatus != ReplyStatus.replyOK && replyStatus != ReplyStatus.replyUserException)
            {
                assert(replyStatus == ReplyStatus.replyObjectNotExist ||
                       replyStatus == ReplyStatus.replyFacetNotExist);
                
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(replyStatus);
                
                _current.id.__write(_os);

                //
                // For compatibility with the old FacetPath.
                //
                if(_current.facet == null || _current.facet.length() == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    String[] facetPath2 = { _current.facet };
                    _os.writeStringSeq(facetPath2);
                }

                _os.writeString(_current.operation);
            }
            else
            {
                int save = _os.pos();
                _os.pos(Protocol.headerSize + 4); // Reply status position.
                _os.writeByte(replyStatus);
                _os.pos(save);
            }

            _connection.sendResponse(_os, _compress);
        }
        else
        {
            _connection.sendNoResponse();
        }

        _connection = null;
    }

    public BasicStream
    is()
    {
        return _is;
    }

    public BasicStream
    os()
    {
        return _os;
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
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); 
                _os.writeByte(ReplyStatus.replyOK);
                _os.startWriteEncaps();
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
