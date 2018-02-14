// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.Threading;

public class Dispatcher
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public Dispatcher()
    {
        Debug.Assert(_instance == null);
        _instance = this;
        _thread = new Thread(run);
        _thread.Start();
    }

    public void run()
    {
        while(true)
        {
#if COMPACT
            Ice.VoidAction call = null;
#else
            System.Action call = null;
#endif
            lock(_m)
            {
                if(!_terminated && _calls.Count == 0)
                {
                    System.Threading.Monitor.Wait(_m);
                }

                if(_calls.Count > 0)
                {
                    call = _calls.Dequeue();
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
                    call();
                }
                catch(System.Exception)
                {
                    // Exceptions should never propagate here.
                    test(false);
                }
            }
        }
    }

#if COMPACT
    public void dispatch(Ice.VoidAction call, Ice.Connection con)
#else
    public void dispatch(System.Action call, Ice.Connection con)
#endif
    {
        lock(_m)
        {
            _calls.Enqueue(call);
            if(_calls.Count == 1)
            {
                System.Threading.Monitor.Pulse(_m);
            }
        }
    }

    static public void terminate()
    {
        lock(_m)
        {
            _instance._terminated = true;
            System.Threading.Monitor.Pulse(_m);
        }

        _instance._thread.Join();
    }

    static public bool isDispatcherThread()
    {
        return Thread.CurrentThread == _instance._thread;
    }

    static Dispatcher _instance;

#if COMPACT
    private Queue<Ice.VoidAction> _calls = new Queue<Ice.VoidAction>();
#else
    private Queue<System.Action> _calls = new Queue<System.Action>();
#endif
    Thread _thread;
    bool _terminated = false;
    private static readonly object _m = new object();
}
