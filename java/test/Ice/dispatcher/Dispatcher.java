// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
        assert(_instance == null);
        _instance = this;
        _thread = new Thread(this);
        _thread.start();
    }

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

    static public void
    terminate()
    {
        synchronized(_instance)
        {
            _instance._terminated = true;
            _instance.notify();
        }
        while(true)
        {
            try
            {
                _instance._thread.join();
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }
    
    static public boolean
    isDispatcherThread()
    {
        return Thread.currentThread() == _instance._thread;
    }

    static Dispatcher _instance; 

    private java.util.Queue<Runnable> _calls = new java.util.LinkedList<Runnable>();
    Thread _thread;
    boolean _terminated = false;
};
