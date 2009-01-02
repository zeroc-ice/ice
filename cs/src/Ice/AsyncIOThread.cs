// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
            _thread.Start();
        }

        public void queue(AsyncCallback callback)
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
            LinkedList<AsyncCallback> queue = new LinkedList<AsyncCallback>();
            while(true)
            {
                lock(this)
                {
                    if(_destroyed && _queue.Count == 0)
                    {
                        break;
                    }

                    while(!_destroyed && _queue.Count == 0)
                    {
                        Monitor.Wait(this);
                    }

                    LinkedList<AsyncCallback> tmp = queue;
                    queue = _queue;
                    _queue = tmp;
                }

                foreach(AsyncCallback cb in queue)
                {
                    try
                    {
                        cb(null);
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
        private LinkedList<AsyncCallback> _queue = new LinkedList<AsyncCallback>();

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

            public void Start()
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
