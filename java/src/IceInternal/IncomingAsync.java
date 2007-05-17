// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IncomingAsync extends IncomingBase
{
    public
    IncomingAsync(Incoming in) // Adopts the argument. It must not be used afterwards.
    {
        super(in);
        _retriable = in.isRetriable();

        if(_retriable)
        {
            in.setActive(this);
            _active = true;
        }
    }

    final void
    __deactivate(Incoming in)
    {
        assert _retriable;

        synchronized(this)
        {
            if(!_active)
            {
                //
                // Since _deactivate can only be called on an active object,
                // this means the response has already been sent (see __validateXXX below)
                //
                throw new Ice.ResponseSentException();
            }
            _active = false;
        }

        in.adopt(this);
    }


    final protected void
    __response(boolean ok)
    {
        try
        {
            if(!__servantLocatorFinished())
            {
                return;
            }

            if(_response)
            {
                _os.endWriteEncaps();
                
                int save = _os.pos();
                _os.pos(Protocol.headerSize + 4); // Reply status position.
                
                if(ok)
                {
                    _os.writeByte(ReplyStatus.replyOK);
                }
                else
                {
                    _os.writeByte(ReplyStatus.replyUserException);
                }
                
                _os.pos(save);

                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.LocalException ex)
        {
            _connection.invokeException(ex, 1);
        }
    }

    final protected void
    __exception(java.lang.Exception exc)
    {
        
        try
        {
            if(!__servantLocatorFinished())
            {
                return;
            }

            __handleException(exc);
        }
        catch(Ice.LocalException ex)
        {
            _connection.invokeException(ex, 1);
        }
    }

    final private boolean
    __servantLocatorFinished()
    {
        try
        {
            if(_locator != null && _servant != null)
            {
                _locator.finished(_current, _servant, _cookie.value);
            }
            return true;
        }
        catch(java.lang.Exception ex)
        {
            __handleException(ex);
            return false;
        }
    }

    final protected boolean 
    __validateResponse(boolean ok)
    {
        if(!_retriable)
        {
            return true;
        }

        try
        {
            if(_interceptorAsyncCallbackList != null)
            {
                java.util.Iterator p = _interceptorAsyncCallbackList.iterator();
                while(p.hasNext())
                {
                    Ice.DispatchInterceptorAsyncCallback cb = (Ice.DispatchInterceptorAsyncCallback)p.next();
                    if(cb.response(ok) == false)
                    {
                        return false;
                    }
                }
            }
        }
        catch(java.lang.RuntimeException ex)
        {
            return false;
        }

        //
        // interceptorAsyncCallbackList is null or all its elements returned OK
        // 
        
        synchronized(this)
        {   
            if(_active)
            {
                _active = false;
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    
    final protected boolean 
    __validateException(java.lang.Exception exc)
    {
        if(!_retriable)
        {
            return true;
        }

        try
        {
            if(_interceptorAsyncCallbackList != null)
            {
                java.util.Iterator p = _interceptorAsyncCallbackList.iterator();
                while(p.hasNext())
                {
                    Ice.DispatchInterceptorAsyncCallback  cb = (Ice.DispatchInterceptorAsyncCallback)p.next();
                    if(cb.exception(exc) == false)
                    {
                        return false;
                    }
                }
            }
        }
        catch(java.lang.RuntimeException ex)
        {
            return false;
        }

        //
        // interceptorAsyncCallbackList is null or all its elements returned OK
        // 
        
        synchronized(this)
        {
            if(_active)
            {
                _active = false;
                return true;
            }
            else
            {
                return false;
            }
        }
    }


    final protected BasicStream
    __os()
    {
        return _os;
    }

    
    private final boolean _retriable;
    private boolean _active = false; // only meaningful when _retriable == true
}
