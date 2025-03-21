// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

//
// TODO replace with System.Timers.Timer
//
namespace Ice.hold;

public class Timer
{
    public delegate void Task();

    private class Entry : IComparable<Entry>
    {
        public Task task;
        public long when;

        public int CompareTo(Entry e)
        {
            if (when < e.when)
            {
                return -1;
            }
            else if (when == e.when)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }

    public Timer()
    {
        _t = new Thread(new ThreadStart(run));
        _t.Start();
    }

    public void schedule(Task task, int milliseconds)
    {
        lock (this)
        {
            var e = new Entry();
            e.task = task;
            e.when = currentMonotonicTimeMillis() + milliseconds;
            _tasks.Add(e);
            _tasks.Sort();
            Monitor.Pulse(this);
        }
    }

    public void shutdown()
    {
        lock (this)
        {
            var e = new Entry();
            e.task = null;
            e.when = 0;
            _tasks.Add(e);
            _tasks.Sort();
            Monitor.Pulse(this);
        }
    }

    public void waitForShutdown()
    {
        _t.Join();
    }

    private void run()
    {
        while (true)
        {
            Entry e;
            lock (this)
            {
                while (true)
                {
                    while (_tasks.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    e = _tasks[0];
                    if (e.task == null)
                    {
                        return;
                    }
                    long now = currentMonotonicTimeMillis();
                    if (now >= e.when)
                    {
                        _tasks.RemoveAt(0);
                        break;
                    }
                    Monitor.Wait(this, (int)(e.when - now));
                }
            }
            e.task();
        }

    }

    private long currentMonotonicTimeMillis()
    {
        return _sw.ElapsedMilliseconds;
    }
    private readonly Stopwatch _sw = Stopwatch.StartNew();

    private readonly Thread _t;
    private readonly List<Entry> _tasks = new List<Entry>();
}
