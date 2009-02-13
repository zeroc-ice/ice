// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class Selector
{
    public 
    Selector(Instance instance, int timeout)
    {
        _instance = instance;
        _timeout = timeout;
        _interruptCount = 0;

        Network.SocketPair pair = Network.createPipe();
        _fdIntrRead = (java.nio.channels.ReadableByteChannel)pair.source;
        _fdIntrWrite = pair.sink;
        try
        {
            _selector = java.nio.channels.Selector.open();
            pair.source.configureBlocking(false);
            _fdIntrReadKey = pair.source.register(_selector, java.nio.channels.SelectionKey.OP_READ);
        }
        catch(java.io.IOException ex)
        {
            Ice.SyscallException sys = new Ice.SyscallException();
            sys.initCause(ex);
            throw sys;
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

        try
        {
            _fdIntrWrite.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _fdIntrWrite = null;

        try
        {
            _fdIntrRead.close();
        }
        catch(java.io.IOException ex)
        {
        }
        _fdIntrRead = null;
    }

    public void
    add(SelectorHandler handler, SocketStatus status)
    {
        // Note: we can't support noInterrupt for add() because a channel can't be registered again 
        // with the selector until the previous selection key has been removed from the cancel-key
        // set of the selector on the next select() operation.

        handler._pendingStatus = status;
        if(_changes.add(handler))
        {    
            setInterrupt();
        }
    }

    public void
    update(SelectorHandler handler, SocketStatus newStatus)
    {
        // Note: can only be called from the select() thread
        assert(handler._key != null);
        handler._key.interestOps(convertStatus(handler.fd(), newStatus));
    }

    public void
    remove(SelectorHandler handler)
    {
        // Note: we can't support noInterrupt for remove() because a channel can't be registered again 
        // with the selector until the previous selection key has been removed from the cancel-key
        // set of the selector on the next select() operation.

        handler._pendingStatus = IceInternal.SocketStatus.Finished;
        if(_changes.add(handler))
        {
            setInterrupt();
        }
    }

    public void
    select()
        throws java.io.IOException
    {
        //
        // If there are still interrupts, selected keys or pending handlers to process, 
        // return immediately.
        //
        if(_interrupted || !_keys.isEmpty() || !_pendingHandlers.isEmpty())
        {
            return;
        }

        //
        // There's nothing left to process, we can now select.
        //
        while(true)
        {
            try
            {
                if(_nextPendingHandlers.isEmpty())
                {
                    if(_timeout > 0)
                    {
                        _selector.select(_timeout * 1000);
                    }
                    else
                    {
                        _selector.select();
                    }
                }
                else
                {
                    _selector.selectNow();

                    java.util.HashSet<SelectorHandler> tmp = _nextPendingHandlers;
                    _nextPendingHandlers = _pendingHandlers;
                    _pendingHandlers = tmp;
                }
            }
            catch(java.nio.channels.CancelledKeyException ex)
            {
                // This sometime occurs on Mac OS X, ignore.
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

                throw ex;
            }

            break;
        }
    }

    public SelectorHandler
    getNextSelected()
    {
        assert(_interruptCount == 0);

        if(_iter == null && !_keys.isEmpty())
        {
            _iter = _keys.iterator();
        }

        while(_iter != null && _iter.hasNext())
        {
            java.nio.channels.SelectionKey key = _iter.next();
            _iter.remove();
            SelectorHandler handler = (SelectorHandler)key.attachment();
            if(handler == null)
            {
                assert(_pendingInterruptRead > 0);
                _pendingInterruptRead -= readInterrupt(_pendingInterruptRead);
                continue;
            }
            else if(handler._key == null || !handler._key.isValid())
            {
                continue;
            }
            if(handler.hasMoreData())
            {
                assert(_pendingIter == null);
                _pendingHandlers.remove(handler);
            }
            if(!_iter.hasNext())
            {
                _iter = null;
            }
            return handler;
        }

        if(_pendingIter == null && !_pendingHandlers.isEmpty())
        {
            _pendingIter = _pendingHandlers.iterator();
        }

        while(_pendingIter != null && _pendingIter.hasNext())
        {
            SelectorHandler handler = _pendingIter.next();
            _pendingIter.remove();
            if(handler._key == null || !handler._key.isValid() || !handler.hasMoreData())
            {
                continue;
            }
            if(!_pendingIter.hasNext())
            {
                _pendingIter = null;
            }
            return handler;
        }

        _iter = null;
        _pendingIter = null;
        return null;
    }

    public void
    hasMoreData(SelectorHandler handler)
    {
        _nextPendingHandlers.add(handler);
    }

    public boolean
    processInterrupt()
    {
        assert(_changes.size() <= _interruptCount);

        if(!_changes.isEmpty())
        {
            java.util.Iterator<SelectorHandler> p = _changes.iterator();
            while(p.hasNext())
            {
                SelectorHandler handler = p.next();
                if(handler._pendingStatus == SocketStatus.Finished)
                {
                    removeImpl(handler);
                }
                else
                {
                    addImpl(handler, handler._pendingStatus);
                }
                clearInterrupt();
            }
            _changes.clear();

            //
            // We call selectNow() to flush the cancelled-key set and ensure handlers can be
            // added again once this returns.
            //
            try
            {
                _selector.selectNow();
            }
            catch(java.io.IOException ex)
            {
                // Ignore.
            }
            
            //
            // Current iterator is invalidated by selectNow().
            //
            _iter = null;
            _pendingIter = null;
        }
        
        _interrupted = _interruptCount > 0;
        return _interruptCount == 0; // No more interrupts to process.
    }

    public boolean
    checkTimeout()
    {
        if(_interruptCount == 0 && _keys.isEmpty() && _pendingHandlers.isEmpty())
        {
            if(_timeout <= 0)
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
                    _instance.initializationData().logger.error("spurious selector wake up");
                }
                return false;
            }
            return true;
        }
        else
        {
            _spuriousWakeUp = 0;
            return false;
        }
    }

    public boolean 
    isInterrupted()
    {
        return _interruptCount > 0;
    }

    public void
    setInterrupt()
    {
        if(++_interruptCount == 1)
        {
            java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(1);
            buf.put(0, (byte)0);
            while(buf.hasRemaining())
            {
                try
                {
                    _fdIntrWrite.write(buf);
                }
                catch(java.io.IOException ex)
                {
                    Ice.SocketException se = new Ice.SocketException();
                    se.initCause(ex);
                    throw se;
                }
            }
        }
    }

    public boolean
    clearInterrupt()
    {
        if(--_interruptCount == 0)
        {
            //
            // If the interrupt byte has not been received by the pipe yet, we just increment 
            // _pendingInterruptRead. It will be read when the _fdIntrRead is ready for read.
            //
            if(_keys.contains(_fdIntrReadKey))
            {
                readInterrupt(1);
                _keys.remove(_fdIntrReadKey);
                _iter = null;
                _pendingIter = null;
            }
            else
            {
                ++_pendingInterruptRead;
            }
            _interrupted = false;
            return false;
        }
        else
        {
            return true;
        }
    }

    private int
    readInterrupt(int count)
    {
        java.nio.ByteBuffer buf = java.nio.ByteBuffer.allocate(count);
        try
        {
            buf.rewind();
            int ret = _fdIntrRead.read(buf);
            assert(ret > 0);
            return ret;
        }
        catch(java.io.IOException ex)
        {
            Ice.SocketException se = new Ice.SocketException();
            se.initCause(ex);
            throw se;
        }
    }
    
    private int
    convertStatus(java.nio.channels.SelectableChannel fd, SocketStatus status)
    {
        if(status == SocketStatus.NeedConnect)
        {
            return java.nio.channels.SelectionKey.OP_CONNECT;
        }
        else if(status == SocketStatus.NeedRead)
        {
            if((fd.validOps() & java.nio.channels.SelectionKey.OP_READ) > 0)
            {
                return java.nio.channels.SelectionKey.OP_READ;
            }
            else
            {
                return java.nio.channels.SelectionKey.OP_ACCEPT;
            }
        }
        else
        {
            assert(status == SocketStatus.NeedWrite);
            return java.nio.channels.SelectionKey.OP_WRITE;
        }
    }

    private void
    addImpl(SelectorHandler handler, SocketStatus status)
    {
        if(handler._key != null)
        {
            handler._key.interestOps(convertStatus(handler.fd(), status));
        }
        else
        {
            try
            {
                handler._key = handler.fd().register(_selector, convertStatus(handler.fd(), status), handler);
            }
            catch(java.nio.channels.ClosedChannelException ex)
            {
                assert(false);
            }
            assert(!_nextPendingHandlers.contains(handler));
        }

        if(handler.hasMoreData())
        {
            _nextPendingHandlers.add(handler);
        }
    }

    private void
    removeImpl(SelectorHandler handler)
    {
        _nextPendingHandlers.remove(handler);

        if(handler._key != null)
        {
            try
            {
                handler._key.cancel();
                handler._key = null;
            }
            catch(java.nio.channels.CancelledKeyException ex)
            {
                assert(false);
            }
        }
    }

    final private Instance _instance;
    final private int _timeout;

    private java.nio.channels.Selector _selector;
    private java.nio.channels.ReadableByteChannel _fdIntrRead;
    private java.nio.channels.WritableByteChannel _fdIntrWrite;
    private java.nio.channels.SelectionKey _fdIntrReadKey;

    private java.util.Set<java.nio.channels.SelectionKey> _keys;
    private java.util.Iterator<java.nio.channels.SelectionKey> _iter;
    private java.util.HashSet<SelectorHandler> _changes = new java.util.HashSet<SelectorHandler>();

    private boolean _interrupted;
    private int _spuriousWakeUp;
    private int _interruptCount;
    private int _pendingInterruptRead;

    private java.util.HashSet<SelectorHandler> _pendingHandlers = new java.util.HashSet<SelectorHandler>();
    private java.util.HashSet<SelectorHandler> _nextPendingHandlers = new java.util.HashSet<SelectorHandler>();
    private java.util.Iterator<SelectorHandler> _pendingIter;
};
