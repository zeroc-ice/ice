// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
            while(true)
            {
                _m.Lock();
                try
                {
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
        }

        private Instance _instance;
        private bool _destroyed;
        private LinkedList<ThreadPoolWorkItem> _queue = new LinkedList<ThreadPoolWorkItem>();

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

            public void Start(ThreadPriority priority)
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = _name;
                _thread.Priority = priority;
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
