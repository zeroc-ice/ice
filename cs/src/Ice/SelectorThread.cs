// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    using System.Net.Sockets;
    using System.Threading;

    public sealed class SelectorThread
    {
        public interface SocketReadyCallback
        {
            //
            // The selector thread unregisters the callback when socketReady returns SocketStatus.Finished.
            //
            SocketStatus socketReady(bool finished);

            //
            // The selector thread doesn't unregister the callback when sockectTimeout is called; socketTimeout
            // must unregister the callback either explicitly with unregister() or by shutting down the socket 
            // (if necessary).
            //
            void socketTimeout();
        }

        internal SelectorThread(Instance instance)
        {
            _instance = instance;
            _destroyed = false;

            Network.SocketPair pair = Network.createPipe();
            _fdIntrRead = pair.source;
            _fdIntrWrite = pair.sink;
            Network.setBlock(_fdIntrRead, false);

            _thread = new HelperThread(this);
            _thread.Start();

            _timer = _instance.timer();
        }

        public void destroy()
        {
            lock(this)
            {
                Debug.Assert(!_destroyed);
                _destroyed = true;
                setInterrupt();
            }
        }

        public void register(Socket fd, SocketReadyCallback cb, SocketStatus status, int timeout)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection
                                           // factories.
                Debug.Assert(status != SocketStatus.Finished);

                SocketInfo info = new SocketInfo(fd, cb, status, timeout);
                _changes.AddLast(info);
                if(info.timeout >= 0)
                {
                    _timer.schedule(info, info.timeout);
                }
                setInterrupt();
            }
        }

        //
        // Unregister the given file descriptor. The registered callback will be notified with socketReady()
        // upon registration to allow some cleanup to be done.
        //
        public void unregister(Socket fd)
        {
            lock(this)
            {
                Debug.Assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing
                                                // connection factories.
                _changes.AddLast(new SocketInfo(fd, null, SocketStatus.Finished, 0));
                setInterrupt();
            }
        }

        public void joinWithThread()
        {
            if(_thread != null)
            {
                _thread.Join();
            }
        }

        private void clearInterrupt()
        {
        repeat:
            try
            {
                _fdIntrRead.Receive(_intrBuf);
            }
            catch(SocketException ex)
            {
                if(Network.interrupted(ex))
                {
                    goto repeat;
                }
                if(!Network.wouldBlock(ex))
                {
                    throw new Ice.SocketException(ex);
                }
            }
        }

        private void setInterrupt()
        {
        repeat:
            try
            {
                _fdIntrWrite.Send(_intrBuf);
            }
            catch(SocketException ex)
            {
                if(Network.interrupted(ex))
                {
                    goto repeat;
                }
                throw new Ice.SocketException(ex);
            }
        }

        private static byte[] _intrBuf = new byte[1];

        public void run()
        {
            Dictionary<Socket, SocketInfo> socketMap = new Dictionary<Socket, SocketInfo>();
            List<SocketInfo> readyList = new List<SocketInfo>();
            List<SocketInfo> finishedList = new List<SocketInfo>();

            Selector selector = new Selector();
            selector.add(_fdIntrRead, SocketStatus.NeedRead);

            while(true)
            {
                selector.select(-1);

                Debug.Assert(readyList.Count == 0 && finishedList.Count == 0);

                if(selector.selRead.Contains(_fdIntrRead))
                {
                    lock(this)
                    {
                        //
                        // There are two possiblities for an interrupt:
                        //
                        // 1. The selector thread has been destroyed.
                        // 2. A socket was registered or unregistered.
                        //

                        //
                        // Thread destroyed?
                        //
                        if(_destroyed)
                        {
                            break;
                        }

                        clearInterrupt();
                        SocketInfo info = _changes.First.Value;
                        _changes.RemoveFirst();
                        if(info.cb != null) // Registration
                        {
                            selector.add(info.fd, info.status);
                            Debug.Assert(!socketMap.ContainsKey(info.fd));
                            socketMap.Add(info.fd, info);
                        }
                        else // Unregistration
                        {
                            if(socketMap.TryGetValue(info.fd, out info))
                            {
                                if(info.status != SocketStatus.Finished)
                                {
                                    if(info.timeout >= 0)
                                    {
                                        _timer.cancel(info);
                                    }

                                    selector.remove(info.fd, info.status);
                                    info.status = SocketStatus.Finished;
                                    readyList.Add(info);
                                }
                            }
                        }
                    }
                }
                else
                {
                    //
                    // Examine the selected sockets.
                    //
                    foreach(Socket fd in selector.selRead)
                    {
                        Debug.Assert(fd != _fdIntrRead);
                        SocketInfo info = null;
                        if(socketMap.TryGetValue(fd, out info))
                        {
                            if(info.timeout >= 0)
                            {
                                _timer.cancel(info);
                            }
                            Debug.Assert(!readyList.Contains(info));
                            readyList.Add(info);
                        }
                    }
                    foreach(Socket fd in selector.selWrite)
                    {
                        SocketInfo info = null;
                        if(socketMap.TryGetValue(fd, out info))
                        {
                            if(info.timeout >= 0)
                            {
                                _timer.cancel(info);
                            }
                            Debug.Assert(!readyList.Contains(info));
                            readyList.Add(info);
                        }
                    }
                    foreach(Socket fd in selector.selError)
                    {
                        SocketInfo info = null;
                        if(socketMap.TryGetValue(fd, out info))
                        {
                            if(info.timeout >= 0)
                            {
                                _timer.cancel(info);
                            }
                            Debug.Assert(!readyList.Contains(info));
                            readyList.Add(info);
                        }
                    }
                }

                foreach(SocketInfo info in readyList)
                {
                    SocketStatus status;
                    try
                    {
                        status = info.cb.socketReady(info.status == SocketStatus.Finished);
                    }
                    catch(Ice.LocalException ex)
                    {
                        string s = "exception in selector thread " + _thread.Name() +
                            " while calling socketReady():\n" + ex;
                        _instance.initializationData().logger.error(s);
                        status = SocketStatus.Finished;
                    }

                    if(status == SocketStatus.Finished)
                    {
                        finishedList.Add(info);
                    }
                    else
                    {
                        Debug.Assert(info.status != SocketStatus.Finished);
                        if(status != info.status) // Status has changed.
                        {
                            selector.remove(info.fd, info.status);
                            selector.add(info.fd, status);
                        }
                        info.status = status;
                        if(info.timeout >= 0)
                        {
                            _timer.schedule(info, info.timeout);
                        }
                    }
                }
                readyList.Clear();

                if(finishedList.Count == 0)
                {
                    continue;
                }

                foreach(SocketInfo info in finishedList)
                {
                    if(info.status != SocketStatus.Finished)
                    {
                        selector.remove(info.fd, info.status);
                    }
                    socketMap.Remove(info.fd);
                }
                finishedList.Clear();
            }

            Debug.Assert(_destroyed);

            try
            {
                _fdIntrWrite.Close();
            }
            catch(System.IO.IOException)
            {
            }
            _fdIntrWrite = null;

            try
            {
                _fdIntrRead.Close();
            }
            catch(System.IO.IOException)
            {
            }
            _fdIntrRead = null;
        }

        private Instance _instance;
        private bool _destroyed;
        private Socket _fdIntrRead;
        private Socket _fdIntrWrite;
        private LinkedList<SocketInfo> _changes = new LinkedList<SocketInfo>();

        private sealed class SocketInfo : TimerTask
        {
            internal Socket fd;
            internal SocketReadyCallback cb;
            internal SocketStatus status;
            internal int timeout;

            public void runTimerTask()
            {
                this.cb.socketTimeout(); // Exceptions will be reported by the timer thread.
            }

            internal SocketInfo(Socket fd, SocketReadyCallback cb, SocketStatus status, int timeout)
            {
                this.fd = fd;
                this.cb = cb;
                this.status = status;
                this.timeout = timeout;
            }
        }

        private sealed class HelperThread
        {
            internal HelperThread(SelectorThread selectorThread)
            {
                selectorThread_ = selectorThread;
                name_ = selectorThread_._instance.initializationData().properties.getProperty("Ice.ProgramName");
                if(name_.Length > 0)
                {
                    name_ += "-";
                }
                name_ += "Ice.SelectorThread";
            }

            public string Name()
            {
                return name_;
            }

            public void Join()
            {
                _thread.Join();
            }

            public void Start()
            {
                _thread = new Thread(new ThreadStart(Run));
                _thread.IsBackground = true;
                _thread.Name = name_;
                _thread.Start();
            }

            public void Run()
            {
                if(selectorThread_._instance.initializationData().threadHook != null)
                {
                    selectorThread_._instance.initializationData().threadHook.start();
                }

                try
                {
                    selectorThread_.run();
                }
                catch(Ice.LocalException ex)
                {
                    string s = "exception in selector thread " + name_ + ":\n" + ex;
                    selectorThread_._instance.initializationData().logger.error(s);
                }
                catch(System.Exception ex)
                {
                    string s = "unknown exception in selector thread " + name_ + ":\n" + ex;
                    selectorThread_._instance.initializationData().logger.error(s);
                }

                if(selectorThread_._instance.initializationData().threadHook != null)
                {
                    selectorThread_._instance.initializationData().threadHook.stop();
                }
            }

            private SelectorThread selectorThread_;
            private string name_;
            private Thread _thread;
        }

        private HelperThread _thread;
        private Timer _timer;
    }
}
