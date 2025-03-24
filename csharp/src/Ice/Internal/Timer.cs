// Copyright (c) ZeroC, Inc.

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

//
// NOTE: We don't use C# timers, the API is quite a bit different from
// the C++ & Java timers and it's not clear what is the cost of
// scheduling and cancelling timers.
//

namespace Ice.Internal;

public interface TimerTask
{
    void runTimerTask();
}

public sealed class Timer
{
    public void destroy()
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                return;
            }

            _instance = null;
            Monitor.Pulse(_mutex);

            _tokens.Clear();
            _tasks.Clear();
        }

        _thread.Join();
    }

    public void schedule(TimerTask task, long delay)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            var token = new Token(Time.currentMonotonicTimeMillis() + delay, ++_tokenId, 0, task);

            try
            {
                _tasks.Add(task, token);
                _tokens.Add(token, null);
            }
            catch (ArgumentException)
            {
                Debug.Assert(false);
            }

            if (token.scheduledTime < _wakeUpTime)
            {
                Monitor.Pulse(_mutex);
            }
        }
    }

    public void scheduleRepeated(TimerTask task, long period)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            var token = new Token(Time.currentMonotonicTimeMillis() + period, ++_tokenId, period, task);

            try
            {
                _tasks.Add(task, token);
                _tokens.Add(token, null);
            }
            catch (System.ArgumentException)
            {
                Debug.Assert(false);
            }

            if (token.scheduledTime < _wakeUpTime)
            {
                Monitor.Pulse(_mutex);
            }
        }
    }

    public bool cancel(TimerTask task)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                return false;
            }

            if (!_tasks.TryGetValue(task, out Token token))
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
    internal Timer(Instance instance, ThreadPriority priority = ThreadPriority.Normal) => init(instance, priority, true);

    internal void init(Instance instance, ThreadPriority priority, bool hasPriority)
    {
        _instance = instance;

        string threadName = _instance.initializationData().properties.getIceProperty("Ice.ProgramName");
        if (threadName.Length > 0)
        {
            threadName += "-";
        }

        _thread = new Thread(new ThreadStart(Run));
        _thread.IsBackground = true;
        _thread.Name = threadName + "Ice.Timer";
        if (hasPriority)
        {
            _thread.Priority = priority;
        }
        _thread.Start();
    }

    internal void updateObserver(Ice.Instrumentation.CommunicatorObserver obsv)
    {
        lock (_mutex)
        {
            Debug.Assert(obsv != null);
            _observer = obsv.getThreadObserver(
                "Communicator",
                _thread.Name,
                Ice.Instrumentation.ThreadState.ThreadStateIdle,
                _observer);
            _observer?.attach();
        }
    }

    public void Run()
    {
        Token token = null;
        while (true)
        {
            lock (_mutex)
            {
                if (_instance != null)
                {
                    //
                    // If the task we just ran is a repeated task, schedule it
                    // again for execution if it wasn't canceled.
                    //
                    if (token != null && token.delay > 0)
                    {
                        if (_tasks.ContainsKey(token.task))
                        {
                            token.scheduledTime = Time.currentMonotonicTimeMillis() + token.delay;
                            _tokens.Add(token, null);
                        }
                    }
                }
                token = null;

                if (_instance == null)
                {
                    break;
                }

                if (_tokens.Count == 0)
                {
                    _wakeUpTime = long.MaxValue;
                    Monitor.Wait(_mutex);
                }

                if (_instance == null)
                {
                    break;
                }

                while (_tokens.Count > 0 && _instance != null)
                {
                    long now = Time.currentMonotonicTimeMillis();

                    Token first = null;
                    foreach (Token t in _tokens.Keys)
                    {
                        first = t;
                        break;
                    }
                    Debug.Assert(first != null);

                    if (first.scheduledTime <= now)
                    {
                        _tokens.Remove(first);
                        token = first;
                        if (token.delay == 0)
                        {
                            _tasks.Remove(token.task);
                        }
                        break;
                    }

                    _wakeUpTime = first.scheduledTime;
                    Monitor.Wait(_mutex, (int)(first.scheduledTime - now));
                }

                if (_instance == null)
                {
                    break;
                }
            }

            if (token != null)
            {
                try
                {
                    Ice.Instrumentation.ThreadObserver threadObserver = _observer;
                    if (threadObserver != null)
                    {
                        threadObserver.stateChanged(
                            Ice.Instrumentation.ThreadState.ThreadStateIdle,
                            Ice.Instrumentation.ThreadState.ThreadStateInUseForOther);
                        try
                        {
                            token.task.runTimerTask();
                        }
                        finally
                        {
                            threadObserver.stateChanged(
                                Ice.Instrumentation.ThreadState.ThreadStateInUseForOther,
                                Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        }
                    }
                    else
                    {
                        token.task.runTimerTask();
                    }
                }
                catch (System.Exception ex)
                {
                    lock (_mutex)
                    {
                        if (_instance != null)
                        {
                            string s = "unexpected exception from task run method in timer thread:\n" + ex;
                            _instance.initializationData().logger.error(s);
                        }
                    }
                }
            }
        }
    }

    private class Token : System.IComparable
    {
        public
        Token(long scheduledTime, int id, long delay, TimerTask task)
        {
            this.scheduledTime = scheduledTime;
            this.id = id;
            this.delay = delay;
            this.task = task;
        }

        public int CompareTo(object o)
        {
            //
            // Token are sorted by scheduled time and token id.
            //
            var r = (Token)o;
            if (scheduledTime < r.scheduledTime)
            {
                return -1;
            }
            else if (scheduledTime > r.scheduledTime)
            {
                return 1;
            }

            if (id < r.id)
            {
                return -1;
            }
            else if (id > r.id)
            {
                return 1;
            }

            return 0;
        }

        public override bool Equals(object o)
        {
            if (ReferenceEquals(this, o))
            {
                return true;
            }
            return o is not Token t ? false : CompareTo(t) == 0;
        }

        public override int GetHashCode() => HashCode.Combine(id, scheduledTime);

        public long scheduledTime;
        public int id; // Since we can't compare references, we need to use another id.
        public long delay;
        public TimerTask task;
    }

    private readonly object _mutex = new object();
    private readonly IDictionary<Token, object> _tokens = new SortedDictionary<Token, object>();
    private readonly IDictionary<TimerTask, Token> _tasks = new Dictionary<TimerTask, Token>();
    private Instance _instance;
    private long _wakeUpTime = long.MaxValue;
    private int _tokenId;
    private Thread _thread;

    //
    // We use a volatile to avoid synchronization when reading
    // _observer. Reference assignment is atomic in Java so it
    // also doesn't need to be synchronized.
    //
    private volatile Ice.Instrumentation.ThreadObserver _observer;
}
