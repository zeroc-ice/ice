//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// NOTE: We don't use C# timers, the API is quite a bit different from the C++ & Java timers and it's not clear
// what is the cost of scheduling and canceling timers.

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    public interface ITimerTask
    {
        void RunTimerTask();
    }

    public sealed class Timer
    {
        private Communicator? _communicator;
        private readonly object _mutex = new object();
        // We use a volatile to avoid synchronization when reading _observer. Reference assignment is atomic in C#
        // so it doesn't need to be synchronized.
        private volatile IThreadObserver? _observer;
        private readonly IDictionary<ITimerTask, Token> _tasks = new Dictionary<ITimerTask, Token>();
        private readonly Thread _thread;
        private readonly IDictionary<Token, object?> _tokens = new SortedDictionary<Token, object?>();
        private int _tokenId = 0;
        private long _wakeUpTime = long.MaxValue;

        public void Destroy()
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    return;
                }

                _communicator = null;
                Monitor.Pulse(_mutex);

                _tokens.Clear();
                _tasks.Clear();
            }

            _thread.Join();
        }

        public void Schedule(ITimerTask task, long delay)
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    throw new CommunicatorDestroyedException();
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
                    Monitor.Pulse(_mutex);
                }
            }
        }

        public void ScheduleRepeated(ITimerTask task, long period)
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    throw new CommunicatorDestroyedException();
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
                    Monitor.Pulse(_mutex);
                }
            }
        }

        public bool Cancel(ITimerTask task)
        {
            lock (_mutex)
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

        // Only for use by Communicator.
        internal Timer(Communicator communicator)
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
            _thread.Start();
        }

        internal void UpdateObserver(ICommunicatorObserver obsv)
        {
            lock (_mutex)
            {
                Debug.Assert(obsv != null);
                _observer = obsv.GetThreadObserver("Communicator",
                                                   _thread.Name!,
                                                   Instrumentation.ThreadState.ThreadStateIdle,
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
                lock (_mutex)
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
                        Monitor.Wait(_mutex);
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
                        Monitor.Wait(_mutex, (int)(first.ScheduledTime - now));
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
                        IThreadObserver? threadObserver = _observer;
                        if (threadObserver != null)
                        {
                            threadObserver.StateChanged(Instrumentation.ThreadState.ThreadStateIdle,
                                                        Instrumentation.ThreadState.ThreadStateInUseForOther);
                            try
                            {
                                token.Task.RunTimerTask();
                            }
                            finally
                            {
                                threadObserver.StateChanged(Instrumentation.ThreadState.ThreadStateInUseForOther,
                                                            Instrumentation.ThreadState.ThreadStateIdle);
                            }
                        }
                        else
                        {
                            token.Task.RunTimerTask();
                        }
                    }
                    catch (System.Exception ex)
                    {
                        lock (_mutex)
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
                // Token are sorted by scheduled time and token id.
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
                return o is Token t && CompareTo(t) == 0;
            }

            public override int GetHashCode() => System.HashCode.Combine(Id, ScheduledTime);

            public long ScheduledTime;
            public int Id; // Since we can't compare references, we need to use another id.
            public long Delay;
            public ITimerTask Task;
        }
    }
}
