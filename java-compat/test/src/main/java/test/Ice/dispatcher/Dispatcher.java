// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

public class Dispatcher implements Runnable, Ice.Dispatcher
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public Dispatcher()
    {
        _thread = new Thread(this);
        _thread.start();
    }

    @Override
    public void 
    run()
    {
        while(true)
        {
            Runnable call = null;
            synchronized(this)
            {
                if(!_terminated && _calls.isEmpty())
                {
                    try
                    {
                        wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                
                if(!_calls.isEmpty())
                {
                    call = _calls.poll();
                }
                else if(_terminated)
                {
                    // Terminate only once all calls are dispatched.
                    return;
                }
            }
            
            if(call != null)
            {
                try
                {
                    call.run();
                }
                catch(Exception ex)
                {
                    // Exceptions should never propagate here.
                    test(false);
                }
            }
        }
    }
    
    @Override
    synchronized public void
    dispatch(Runnable call, Ice.Connection con)
    {
        boolean added = _calls.offer(call);
        assert(added);
        if(_calls.size() == 1)
        {
            notify();
        }
    }

    public void
    terminate()
    {
        synchronized(this)
        {
            _terminated = true;
            notify();
        }
        while(true)
        {
            try
            {
                _thread.join();
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }
    
    public boolean
    isDispatcherThread()
    {
        return Thread.currentThread() == _thread;
    }

    private java.util.Queue<Runnable> _calls = new java.util.LinkedList<Runnable>();
    private Thread _thread;
    private boolean _terminated = false;
}
