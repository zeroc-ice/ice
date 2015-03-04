// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Selector
{
    static final class TimeoutException extends Exception
    {
    }

    public 
    Selector(Instance instance)
    {
        _instance = instance;

        try
        {
            _selector = java.nio.channels.Selector.open();
        }
        catch(java.io.IOException ex)
        {
            throw new Ice.SyscallException(ex);
        }

        //
        // The Selector holds a Set representing the selected keys. The
        // Set reference doesn't change, so we obtain it once here.
        //
        _keys = _selector.selectedKeys();
    }

    public void
    destroy()
    {
        try
        {
            _selector.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _selector = null;
    }

    public void
    initialize(EventHandler handler)
    {
        updateImpl(handler);
    }

    public void
    update(EventHandler handler, int remove, int add)
    {
        int previous = handler._registered;
        handler._registered = handler._registered & ~remove;
        handler._registered = handler._registered | add;
        if(previous == handler._registered)
        {
            return;
        }
        updateImpl(handler);

        if(handler.hasMoreData() && (handler._disabled & SocketOperation.Read) == 0)
        {
            if((add & SocketOperation.Read) != 0)
            {
                _pendingHandlers.add(handler);
            }
            if((remove & SocketOperation.Read) != 0)
            {
                _pendingHandlers.remove(handler);
            }
        }
    }

    public void
    enable(EventHandler handler, int status)
    {
        if((handler._disabled & status) == 0)
        {
            return;
        }
        handler._disabled = handler._disabled & ~status;

        if((handler._registered & status) != 0)
        {
            updateImpl(handler);

            if((status & SocketOperation.Read) != 0 && handler.hasMoreData())
            {
                // Add back the pending handler if reads are enabled.
                _pendingHandlers.add(handler);
            }
        }
    }

    public void
    disable(EventHandler handler, int status)
    {
        if((handler._disabled & status) != 0)
        {
            return;
        }
        handler._disabled = handler._disabled | status;
    
        if((handler._registered & status) != 0)
        {
            updateImpl(handler);

            if((status & SocketOperation.Read) != 0 && handler.hasMoreData())
            {
                // Remove the pending handler if reads are disabled.
                _pendingHandlers.remove(handler);
            }
        }
    }

    public void
    finish(EventHandler handler)
    {
        handler._registered = 0;

        if(handler._key != null)
        {
            handler._key.cancel();
            handler._key = null;
        }

        _changes.remove(handler);
        _pendingHandlers.remove(handler);
    }

    public void
    startSelect()
    {
        assert(_changes.isEmpty());

        //
        // Don't set _selecting = true if there are pending handlers, select() won't block
        // and will just call selectNow().
        //
        if(_pendingHandlers.isEmpty())
        {
            _selecting = true;
        }
    }

    public void
    finishSelect(java.util.List<EventHandler> handlers, long timeout)
    {
        _selecting = false;
        handlers.clear();

        if(!_changes.isEmpty())
        {
            for(EventHandler h : _changes)
            {
                updateImpl(h);
            }
            _changes.clear();
        }
        else if(_keys.isEmpty() && _pendingHandlers.isEmpty() && timeout <= 0)
        {
            //
            // This is necessary to prevent a busy loop in case of a spurious wake-up which
            // sometime occurs in the client thread pool when the communicator is destroyed.
            // If there are too many successive spurious wake-ups, we log an error.
            //
            try
            {
                Thread.sleep(1);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            
            if(++_spuriousWakeUp > 100)
            {
                _spuriousWakeUp = 0;
                _instance.initializationData().logger.warning("spurious selector wake up");
            }
            return;
        }
        
        _spuriousWakeUp = 0;

        for(java.nio.channels.SelectionKey key : _keys)
        {
            EventHandler handler = (EventHandler)key.attachment();
            try
            { 
                //
                // It's important to check for interestOps here because the event handler
                // registration might have changed above when _changes was processed. We
                // don't want to return event handlers which aren't interested anymore in
                // a given operation.
                //
                handler._ready = fromJavaOps(key.readyOps() & key.interestOps());
                if(handler.hasMoreData() && _pendingHandlers.remove(handler))
                {
                    handler._ready |= SocketOperation.Read;
                }
                handlers.add(handler);
            }
            catch(java.nio.channels.CancelledKeyException ex)
            {
                assert(handler._registered == 0);
            }
        }
        _keys.clear();

        for(EventHandler handler : _pendingHandlers)
        {
            if(handler.hasMoreData())
            {
                handler._ready = SocketOperation.Read;
                handlers.add(handler);
            }
        }
        _pendingHandlers.clear();
    }

    public void
    select(long timeout)
        throws TimeoutException
    {
        while(true)
        {
            try
            {
                //
                // Only block if _selecting = true, otherwise we call selectNow() to retrieve new 
                // ready handlers and process handlers from _pendingHandlers.
                //
                if(_selecting)
                {
                    if(timeout > 0)
                    {
                        //
                        // NOTE: On some platforms, select() sometime returns slightly before
                        // the timeout (at least according to our monotonic time). To make sure
                        // timeouts are correctly detected, we wait for a little longer than
                        // the configured timeout (10ms).
                        //
                        long before = IceInternal.Time.currentMonotonicTimeMillis();
                        if(_selector.select(timeout * 1000 + 10) == 0)
                        {
                            if(IceInternal.Time.currentMonotonicTimeMillis() - before >= timeout * 1000)
                            {
                                throw new TimeoutException();
                            }
                        }
                    }
                    else
                    {
                        _selector.select();
                    }
                }
                else
                {
                    _selector.selectNow();
                }
            }
            catch(java.nio.channels.CancelledKeyException ex)
            {
                // This sometime occurs on OS X, ignore.
                continue;
            }
            catch(java.io.IOException ex)
            {
                //
                // Pressing Ctrl-C causes select() to raise an
                // IOException, which seems like a JDK bug. We trap
                // for that special case here and ignore it.
                // Hopefully we're not masking something important!
                //
                if(Network.interrupted(ex))
                {
                    continue;
                }

                try
                {
                    String s = "fatal error: selector failed:\n" + ex.getCause().getMessage();
                    _instance.initializationData().logger.error(s);
                }
                finally
                {
                    Runtime.getRuntime().halt(1);
                }
            }

            break;
        }
    }

    public void
    hasMoreData(EventHandler handler)
    {
        assert(!_selecting && handler.hasMoreData());

        //
        // Only add the handler if read is still registered and enabled.
        //
        if((handler._registered & ~handler._disabled & SocketOperation.Read) != 0)
        {
            _pendingHandlers.add(handler);
        }
    }

    private void
    updateImpl(EventHandler handler)
    {
        if(_selecting)
        {
            //
            // Queue the change since we can't change the selection key interest ops while a select
            // operation is in progress (it could block depending on the underlying implementaiton
            // of the Java selector).
            //
            if(_changes.isEmpty())
            {
                _selector.wakeup();
            }
            _changes.add(handler);
           return;
        }

        int ops = toJavaOps(handler, handler._registered & ~handler._disabled);
        if(handler._key == null)
        {
            if(handler._registered != 0)
            {
                try
                {
                    handler._key = handler.fd().register(_selector, ops, handler);
                }
                catch(java.nio.channels.ClosedChannelException ex)
                {
                    assert(false);
                }
            }
        }
        else
        {
            handler._key.interestOps(ops);
        }
    }

    int
    toJavaOps(EventHandler handler, int o)
    {
        int op = 0;
        if((o & SocketOperation.Read) != 0)
        {
            if((handler.fd().validOps() & java.nio.channels.SelectionKey.OP_READ) != 0)
            {
                op |= java.nio.channels.SelectionKey.OP_READ;
            }
            else
            {
                op |= java.nio.channels.SelectionKey.OP_ACCEPT;
            }
        }
        if((o & SocketOperation.Write) != 0)
        {
            op |= java.nio.channels.SelectionKey.OP_WRITE;
        }
        if((o & SocketOperation.Connect) != 0)
        {
            op |= java.nio.channels.SelectionKey.OP_CONNECT;
        }
        return op;
    }

    int
    fromJavaOps(int o)
    {
        int op = 0;
        if((o & (java.nio.channels.SelectionKey.OP_READ | java.nio.channels.SelectionKey.OP_ACCEPT)) != 0)
        {
            op |= SocketOperation.Read;
        }
        if((o & java.nio.channels.SelectionKey.OP_WRITE) != 0)
        {
            op |= SocketOperation.Write;
        }
        if((o & java.nio.channels.SelectionKey.OP_CONNECT) != 0)
        {
            op |= SocketOperation.Connect;
        }
        return op;
    }


    final private Instance _instance;

    private java.nio.channels.Selector _selector;

    private java.util.Set<java.nio.channels.SelectionKey> _keys;
    private java.util.HashSet<EventHandler> _changes = new java.util.HashSet<EventHandler>();
    private java.util.HashSet<EventHandler> _pendingHandlers = new java.util.HashSet<EventHandler>();
    private boolean _selecting;
    private int _spuriousWakeUp;
}
