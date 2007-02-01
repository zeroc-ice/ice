// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        lock(this)
        {
            while(!_done)
            {
                if(_callbacks.Count == 0)
                {
                    Monitor.Wait(this);
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
                    Monitor.Wait(this, entry.delay);

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
    }

    public void Add(AMD_Hello_sayHello cb, int delay)
    {
        lock(this)
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
                    Monitor.Pulse(this);
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
    }

    public void destroy()
    {
        lock(this)
        {
            _done = true;
            Monitor.Pulse(this);
        }
    }

    private ArrayList _callbacks = new ArrayList();
    private bool _done = false;
    private Thread thread_;
}
