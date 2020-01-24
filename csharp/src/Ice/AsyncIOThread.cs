//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace IceInternal
{

    public class AsyncIOThread
    {
        internal AsyncIOThread(Ice.Communicator communicator)
        {
            _communicator = communicator;

            _thread = new HelperThread(this);
            UpdateObserver();
            _thread.Start(Util.StringToThreadPriority(communicator.GetProperty("Ice.ThreadPriority")));
        }

        public void
        UpdateObserver()
        {
            lock (this)
            {
                Ice.Instrumentation.ICommunicatorObserver? obsv = _communicator.Observer;
                if (obsv != null)
                {
                    _observer = obsv.GetThreadObserver("Communicator",
                                                       _thread.GetName(),
                                                       Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                                       _observer);
                    if (_observer != null)
                    {
                        _observer.Attach();
                    }
                }
            }
        }

        public void Queue(ThreadPoolWorkItem callback)
        {
            lock (this)
            {
                Debug.Assert(!_destroyed);
                _queue.AddLast(callback);
                Monitor.Pulse(this);
            }
        }

        public void Destroy()
        {
            lock (this)
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                Monitor.Pulse(this);
            }
        }

        public void JoinWithThread()
        {
            if (_thread != null)
            {
                _thread.Join();
            }
        }

        public void Run()
        {
            var queue = new LinkedList<ThreadPoolWorkItem>();
            bool inUse = false;
            while (true)
            {
                lock (this)
                {
                    if (_observer != null && inUse)
                    {
                        _observer.StateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForIO,
                                               Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        inUse = false;
                    }

                    if (_destroyed && _queue.Count == 0)
                    {
                        break;
                    }

                    while (!_destroyed && _queue.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    LinkedList<ThreadPoolWorkItem> tmp = queue;
                    queue = _queue;
                    _queue = tmp;

                    if (_observer != null)
                    {
                        _observer.StateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle,
                                               Ice.Instrumentation.ThreadState.ThreadStateInUseForIO);
                        inUse = true;
                    }
                }

                foreach (ThreadPoolWorkItem cb in queue)
                {
                    try
                    {
                        cb();
                    }
                    catch (Ice.LocalException ex)
                    {
                        string s = "exception in asynchronous IO thread:\n" + ex;
                        _communicator.Logger.Error(s);
                    }
                    catch (System.Exception ex)
                    {
                        string s = "unknown exception in asynchronous IO thread:\n" + ex;
                        _communicator.Logger.Error(s);
                    }
                }
                queue.Clear();
            }

            if (_observer != null)
            {
                _observer.Detach();
            }
        }

        private readonly Ice.Communicator _communicator;
        private bool _destroyed;
        private LinkedList<ThreadPoolWorkItem> _queue = new LinkedList<ThreadPoolWorkItem>();
        private Ice.Instrumentation.IThreadObserver? _observer;

        private sealed class HelperThread
        {
            internal HelperThread(AsyncIOThread asyncIOThread)
            {
                _asyncIOThread = asyncIOThread;
                _name = _asyncIOThread._communicator.GetProperty("Ice.ProgramName") ?? "";
                if (_name.Length > 0)
                {
                    _name += "-";
                }
                _name += "Ice.AsyncIOThread";
            }

            public void Join() => _thread!.Join();

            public string GetName() => _name;

            public void Start(ThreadPriority priority)
            {
                _thread = new Thread(new ThreadStart(Run))
                {
                    IsBackground = true,
                    Name = _name
                };
                _thread.Start();
            }

            public void Run() => _asyncIOThread.Run();

            private readonly AsyncIOThread _asyncIOThread;
            private readonly string _name;
            private Thread? _thread;
        }

        private readonly HelperThread _thread;
    }
}
