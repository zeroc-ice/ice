//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// NOTE: We don't use C# timers, the API is quite a bit different from
// the C++ & Java timers and it's not clear what is the cost of
// scheduling and cancelling timers.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace IceInternal
{
    public interface ITimerTask
    {
        void RunTimerTask();
    }

    public sealed class Timer
    {
        public void Destroy()
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    return;
                }

                _communicator = null;
                Monitor.Pulse(this);

                _tokens.Clear();
                _tasks.Clear();
            }

            _thread.Join();
        }

        public void Schedule(ITimerTask task, long delay)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                var token = new Token(Time.CurrentMonotonicTimeMillis() + delay, ++_tokenId, 0, task);

                try
                {
                    _tasks.Add(task, token);
                    _tokens.Add(token, null);
                }
                catch (System.ArgumentException)
                {
                    Debug.Assert(false);
                }

                if (token.ScheduledTime < _wakeUpTime)
                {
                    Monitor.Pulse(this);
                }
            }
        }

        public void ScheduleRepeated(ITimerTask task, long period)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                var token = new Token(Time.CurrentMonotonicTimeMillis() + period, ++_tokenId, period, task);

                try
                {
                    _tasks.Add(task, token);
                    _tokens.Add(token, null);
                }
                catch (System.ArgumentException)
                {
                    Debug.Assert(false);
                }

                if (token.ScheduledTime < _wakeUpTime)
                {
                    Monitor.Pulse(this);
                }
            }
        }

        public bool Cancel(ITimerTask task)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    return false;
                }

                if (!_tasks.TryGetValue(task, out Token? token))
                {
                    return false;
                }
                _tasks.Remove(task);
                _tokens.Remove(token);
                return true;
            }
        }

        //
        // Only for use by Instance.
        //
        internal Timer(Ice.Communicator communicator, ThreadPriority priority = ThreadPriority.Normal)
        {
            _communicator = communicator;
            string? threadName = _communicator.GetProperty("Ice.ProgramName");
            if (threadName != null)
            {
                threadName += "-";
            }

            _thread = new Thread(new ThreadStart(Run));
            _thread.IsBackground = true;
            _thread.Name = $"{threadName}Ice.Timer";
            _thread.Priority = priority;
            _thread.Start();
        }

        internal void UpdateObserver(Ice.Instrumentation.ICommunicatorObserver obsv)
        {
            lock (this)
            {
                Debug.Assert(obsv != null);
                _observer = obsv.GetThreadObserver("Communicator",
                                                   _thread.Name!,
                                                   Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                   _observer);
                if (_observer != null)
                {
                    _observer.Attach();
                }
            }
        }

        public void Run()
        {
            Token? token = null;
            while (true)
            {
                lock (this)
                {
                    if (_communicator != null)
                    {
                        //
                        // If the task we just ran is a repeated task, schedule it
                        // again for execution if it wasn't canceled.
                        //
                        if (token != null && token.Delay > 0)
                        {
                            if (_tasks.ContainsKey(token.Task))
                            {
                                token.ScheduledTime = Time.CurrentMonotonicTimeMillis() + token.Delay;
                                _tokens.Add(token, null);
                            }
                        }
                    }
                    token = null;

                    if (_communicator == null)
                    {
                        break;
                    }

                    if (_tokens.Count == 0)
                    {
                        _wakeUpTime = long.MaxValue;
                        Monitor.Wait(this);
                    }

                    if (_communicator == null)
                    {
                        break;
                    }

                    while (_tokens.Count > 0 && _communicator != null)
                    {
                        long now = Time.CurrentMonotonicTimeMillis();

                        Token? first = null;
                        foreach (Token t in _tokens.Keys)
                        {
                            first = t;
                            break;
                        }
                        Debug.Assert(first != null);

                        if (first.ScheduledTime <= now)
                        {
                            _tokens.Remove(first);
                            token = first;
                            if (token.Delay == 0)
                            {
                                _tasks.Remove(token.Task);
                            }
                            break;
                        }

                        _wakeUpTime = first.ScheduledTime;
                        Monitor.Wait(this, (int)(first.ScheduledTime - now));
                    }

                    if (_communicator == null)
                    {
                        break;
                    }
                }

                if (token != null)
                {
                    try
                    {
                        Ice.Instrumentation.IThreadObserver? threadObserver = _observer;
                        if (threadObserver != null)
                        {
                            threadObserver.StateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                        Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                            try
                            {
                                token.Task.RunTimerTask();
                            }
                            finally
                            {
                                threadObserver.StateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                            Ice.Instrumentation.ThreadState.ThreadStateIdle);
                            }
                        }
                        else
                        {
                            token.Task.RunTimerTask();
                        }
                    }
                    catch (System.Exception ex)
                    {
                        lock (this)
                        {
                            if (_communicator != null)
                            {
                                string s = "unexpected exception from task run method in timer thread:\n" + ex;
                                _communicator.Logger.Error(s);
                            }
                        }
                    }
                }
            }
        }

        private class Token : System.IComparable
        {
            public
            Token(long scheduledTime, int id, long delay, ITimerTask task)
            {
                ScheduledTime = scheduledTime;
                Id = id;
                Delay = delay;
                Task = task;
            }

            public int CompareTo(object? o)
            {
                Debug.Assert(o != null);
                //
                // Token are sorted by scheduled time and token id.
                //
                var r = (Token)o;
                if (ScheduledTime < r.ScheduledTime)
                {
                    return -1;
                }
                else if (ScheduledTime > r.ScheduledTime)
                {
                    return 1;
                }

                if (Id < r.Id)
                {
                    return -1;
                }
                else if (Id > r.Id)
                {
                    return 1;
                }

                return 0;
            }

            public override bool Equals(object? o)
            {
                if (ReferenceEquals(this, o))
                {
                    return true;
                }
                return !(o is Token t) ? false : CompareTo(t) == 0;
            }

            public override int GetHashCode() => System.HashCode.Combine(Id, ScheduledTime);

            public long ScheduledTime;
            public int Id; // Since we can't compare references, we need to use another id.
            public long Delay;
            public ITimerTask Task;
        }

        private readonly IDictionary<Token, object?> _tokens = new SortedDictionary<Token, object?>();
        private readonly IDictionary<ITimerTask, Token> _tasks = new Dictionary<ITimerTask, Token>();
        private Ice.Communicator? _communicator;
        private long _wakeUpTime = long.MaxValue;
        private int _tokenId = 0;
        private readonly Thread _thread;

        //
        // We use a volatile to avoid synchronization when reading
        // _observer. Reference assignement is atomic in Java so it
        // also doesn't need to be synchronized.
        //
        private volatile Ice.Instrumentation.IThreadObserver? _observer;
    }
}
