// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using System.Collections;
using Demo;

public class WorkQueue
{
    private class CallbackEntry
    {
        public AMD_Hello_sayHello cb;
        public int delay;
    }

    public void Join()
    {
        thread_.Join();
    }

    public void Start()
    {
        thread_ = new Thread(new ThreadStart(Run));
        thread_.Start();
    }

    public void Run()
    {
#if COMPACT
        _m.Lock();
        try
#else
        lock(this)
#endif
        {
            while(!_done)
            {
                if(_callbacks.Count == 0)
                {
#if COMPACT
                    _m.Wait();
#else
                    Monitor.Wait(this);
#endif
                }

                if(_callbacks.Count != 0)
                {
                    //
                    // Get next work item.
                    //
                    CallbackEntry entry = (CallbackEntry)_callbacks[0];

                    //
                    // Wait for the amount of time indicated in delay to
                    // emulate a process that takes a significant period of
                    // time to complete.
                    //
#if COMPACT
                    _m.TimedWait(entry.delay);
#else
                    Monitor.Wait(this, entry.delay);
#endif

                    if(!_done)
                    {
                        //
                        // Print greeting and send response.
                        //
                        _callbacks.RemoveAt(0);
                        Console.Out.WriteLine("Belated Hello World!");
                        entry.cb.ice_response();
                    }
                }
            }

            foreach(CallbackEntry e in _callbacks)
            {
                e.cb.ice_exception(new RequestCanceledException());
            }
        }
#if COMPACT
        finally
        {
            _m.Unlock();
        }
#endif
    }

    public void Add(AMD_Hello_sayHello cb, int delay)
    {
#if COMPACT
        _m.Lock();
        try
#else
        lock(this)
#endif
        {
            if(!_done)
            {
                //
                // Add the work item.
                //
                CallbackEntry entry = new CallbackEntry();
                entry.cb = cb;
                entry.delay = delay;

                if(_callbacks.Count == 0)
                {
#if COMPACT
                    _m.Notify();
#else
                    Monitor.Pulse(this);
#endif
                }
                _callbacks.Add(entry);
            }
            else
            {
                //
                // Destroyed, throw exception.
                //
                cb.ice_exception(new RequestCanceledException());
            }
        }
#if COMPACT
        finally
        {
            _m.Unlock();
        }
#endif
    }

    public void destroy()
    {
#if COMPACT
        _m.Lock();
        try
        {
            _done = true;
            _m.Notify();
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            _done = true;
            Monitor.Pulse(this);
        }
#endif
    }

    private ArrayList _callbacks = new ArrayList();
    private bool _done = false;
    private Thread thread_;
#if COMPACT
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
#endif
}
