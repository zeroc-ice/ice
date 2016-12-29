// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;

    public class AsyncIOThread
    {
        internal AsyncIOThread(Instance instance)
        {
            _instance = instance;

            _thread = new HelperThread(this);
            updateObserver();
            _thread.Start(Util.stringToThreadPriority(
                                        instance.initializationData().properties.getProperty("Ice.ThreadPriority")));
        }

        public void
        updateObserver()
        {
            lock(this)
            {
                Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
                if(obsv != null)
                {
                    _observer = obsv.getThreadObserver("Communicator", 
                                                       _thread.getName(), 
                                                       Ice.Instrumentation.ThreadState.ThreadStateIdle, 
                                                       _observer);
                    if(_observer != null)
                    {
                        _observer.attach();
                    }
                }
            }
        }

        public void queue(ThreadPoolWorkItem callback)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);
                _queue.AddLast(callback);
                Monitor.Pulse(this);
            }
        }

        public void destroy()
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                Monitor.Pulse(this);
            }
        }

        public void joinWithThread()
        {
            if(_thread != null)
            {
                _thread.Join();
            }
        }

        public void run()
        {
            LinkedList<ThreadPoolWorkItem> queue = new LinkedList<ThreadPoolWorkItem>();
            bool inUse = false;
            while(true)
            {
                lock(this)
                {
                    if(_observer != null && inUse)
                    {
                        _observer.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateInUseForIO, 
                                               Ice.Instrumentation.ThreadState.ThreadStateIdle);
                        inUse = false;
                    }

                    if(_destroyed && _queue.Count == 0)
                    {
                        break;
                    }

                    while(!_destroyed && _queue.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    LinkedList<ThreadPoolWorkItem> tmp = queue;
                    queue = _queue;
                    _queue = tmp;

                    if(_observer != null)
                    {
                        _observer.stateChanged(Ice.Instrumentation.ThreadState.ThreadStateIdle, 
                                               Ice.Instrumentation.ThreadState.ThreadStateInUseForIO);
                        inUse = true;
                    }
                }

                foreach(ThreadPoolWorkItem cb in queue)
                {
                    try
                    {
                        cb();
                    }
                    catch(Ice.LocalException ex)
                    {
                        string s = "exception in asynchronous IO thread:\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                    catch(System.Exception ex)
                    {
                        string s = "unknown exception in asynchronous IO thread:\n" + ex;
                        _instance.initializationData().logger.error(s);
                    }
                }
                queue.Clear();
            }

            if(_observer != null)
            {
                _observer.detach();
            }
        }

        private Instance _instance;
        private bool _destroyed;
        private LinkedList<ThreadPoolWorkItem> _queue = new LinkedList<ThreadPoolWorkItem>();
        private Ice.Instrumentation.ThreadObserver _observer;

        private sealed class HelperThread
        {
            internal HelperThread(AsyncIOThread asyncIOThread)
            {
                _asyncIOThread = asyncIOThread;
                _name = _asyncIOThread._instance.initializationData().properties.getProperty("Ice.ProgramName");
                if(_name.Length > 0)
                {
                    _name += "-";
                }
                _name += "Ice.AsyncIOThread";
            }

            public void Join()
            {
                _thread.Join();
            }

            public string getName()
            {
                return _name;
            }

            public void Start(ThreadPriority priority)
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = _name;
                _thread.Start();
            }

            public void Run()
            {
                _asyncIOThread.run();
            }

            private AsyncIOThread _asyncIOThread;
            private string _name;
            private Thread _thread;
        }

        private HelperThread _thread;
    }
}
