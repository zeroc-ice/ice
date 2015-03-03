// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class BatchOutgoing implements OutgoingMessageCallback
{
    public
    BatchOutgoing(Ice.ConnectionI connection, Instance instance)
    {
        _connection = connection;
        _sent = false;
        _os = new BasicStream(instance);
    }

    public
    BatchOutgoing(RequestHandler handler)
    {
        _handler = handler;
        _sent = false;
        _os = new BasicStream(handler.getReference().getInstance());
    }

    public void
    invoke()
    {
        assert(_handler != null || _connection != null);

        if(_handler != null && !_handler.flushBatchRequests(this) || 
           _connection != null && !_connection.flushBatchRequests(this))
        {
            synchronized(this)
            {
                while(_exception == null && !_sent)
                {
                    try
                    {
                        wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                
                if(_exception != null)
                {
                    throw _exception;
                }
            }
        }
    }

    public void
    sent(boolean async)
    {
        if(async)
        {
            synchronized(this)
            {
                _sent = true;
                notify();
            }
        }
        else
        {
            _sent = true;
        }
    }
    
    public synchronized void
    finished(Ice.LocalException ex, boolean sent)
    {
        _exception = ex;
        notify();
    }

    public BasicStream
    os()
    {
        return _os;
    }

    private RequestHandler _handler;
    private Ice.ConnectionI _connection;
    private BasicStream _os;
    private boolean _sent;
    private Ice.LocalException _exception;
}
