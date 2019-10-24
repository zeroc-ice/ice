//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;

//
// TODO replace with System.Timers.Timer
//
namespace Ice
{
    namespace hold
    {
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
            };

            public Timer()
            {
                _t = new Thread(new ThreadStart(run));
                _t.Start();
            }

            public void schedule(Task task, int milliseconds)
            {
                lock (this)
                {
                    Entry e = new Entry();
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
                    Entry e = new Entry();
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
            private Stopwatch _sw = Stopwatch.StartNew();

            private Thread _t;
            private List<Entry> _tasks = new List<Entry>();
        }
    }
}
