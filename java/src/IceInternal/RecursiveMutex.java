// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class RecursiveMutex
{
    public
    RecursiveMutex()
    {
        _owner = null;
        _locked = false;
        _count = 0;
    }

    public synchronized void
    lock()
    {
        if(_locked && Thread.currentThread() == _owner)
        {
            _count++;
            return;
        }

        while(_locked)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }

        assert(_owner == null && _count == 0);
        _locked = true;
        _owner = Thread.currentThread();
        _count = 1;
    }

    public synchronized boolean
    trylock()
    {
        if(_owner == null)
        {
            _owner = Thread.currentThread();
            _count = 1;
            _locked = true;
            return true;
        }

        if(_owner == Thread.currentThread())
        {
            assert(_count > 0);
            _count++;
            return true;
        }

        return false;
    }

    public synchronized void
    unlock()
    {
        assert(_owner == Thread.currentThread() && _count > 0 && _locked);
        _count--;
        if(_count <= 0)
        {
            _locked = false;
            _owner = null;
            notify();
        }
    }

    private Thread _owner;
    private boolean _locked;
    private int _count;
}
