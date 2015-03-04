// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Net;
    using System.Threading;

    public class AsyncIOThread
    {
        internal AsyncIOThread(Instance instance)
        {
            _instance = instance;

            _thread = new HelperThread(this);
            updateObserver();
#if !SILVERLIGHT
            if(instance.initializationData().properties.getProperty("Ice.ThreadPriority").Length > 0)
            {
                ThreadPriority priority = IceInternal.Util.stringToThreadPriority(
                                           instance.initializationData().properties.getProperty("Ice.ThreadPriority"));
                _thread.Start(priority);
            }
            else
            {
                _thread.Start(ThreadPriority.Normal);
            }
#else
            _thread.Start();
#endif
        }

        public void
        updateObserver()
        {
            _m.Lock();
            try
            {
                Ice.Instrumentation.CommunicatorObserver obsv = _instance.getObserver();
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
            finally
            {
                _m.Unlock();
            }
        }

        public void queue(ThreadPoolWorkItem callback)
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_destroyed);
                _queue.AddLast(callback);
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void destroy()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
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
                _m.Lock();
                try
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
                        _m.Wait();
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
                finally
                {
                    _m.Unlock();
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

#if !SILVERLIGHT
            public void Start(ThreadPriority priority)
#else
            public void Start()
#endif
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = _name;
#if !SILVERLIGHT
                _thread.Priority = priority;
#endif
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
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
