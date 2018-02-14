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
using System.Collections.Generic;
using System.Threading;

//
// The .NET Compact Framework doesn't support the System.Timers.Timer class.
//
public class Timer
{
    public delegate void Task();

    private class Entry : IComparable<Entry>
    {
        public Task task;
        public long when;

        public int CompareTo(Entry e)
        {
            if(when < e.when)
            {
                return -1;
            }
            else if(when == e.when)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    };

    public Timer()
    {
        _t = new System.Threading.Thread(new System.Threading.ThreadStart(run));
        _t.Start();
    }

    public void schedule(Task task, int milliseconds)
    {
        lock(this)
        {
            Entry e = new Entry();
            e.task = task;
            e.when = currentMonotonicTimeMillis() + milliseconds;
            _tasks.Add(e);
            _tasks.Sort();
            System.Threading.Monitor.Pulse(this);
        }
    }

    public void shutdown()
    {
        lock(this)
        {
            Entry e = new Entry();
            e.task = null;
            e.when = 0;
            _tasks.Add(e);
            _tasks.Sort();
            System.Threading.Monitor.Pulse(this);
        }
    }

    public void waitForShutdown()
    {
        _t.Join();
    }

    private void run()
    {
        while(true)
        {
            Entry e;
            lock(this)
            {
                while(true)
                {
                    while(_tasks.Count == 0)
                    {
                        System.Threading.Monitor.Wait(this);
                    }

                    e = _tasks[0];
                    if(e.task == null)
                    {
                       return;
                    }
                    long now = currentMonotonicTimeMillis();
                    if(now >= e.when)
                    {
                        _tasks.RemoveAt(0);
                        break;
                    }
                    System.Threading.Monitor.Wait(this, (int)(e.when - now));
                }
            }
            e.task();
        }

    }

#if SILVERLIGHT
    private long currentMonotonicTimeMillis()
    {
        return (System.DateTime.Now.Ticks - _begin) / 10000;
    }
    private long _begin = System.DateTime.Now.Ticks;
#else
    private long currentMonotonicTimeMillis()
    {
        return _sw.ElapsedMilliseconds;
    }
    private Stopwatch _sw = Stopwatch.StartNew();
#endif

    private System.Threading.Thread _t;
    private List<Entry> _tasks = new List<Entry>();
}
