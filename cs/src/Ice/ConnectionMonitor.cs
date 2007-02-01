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

    using System.Diagnostics;
    using System.Threading;
    using IceUtil;

    public sealed class ConnectionMonitor
    {
        public void destroy()
        {
            lock(this)
            {
                Debug.Assert(instance_ != null);
                
                instance_ = null;
                _connections = null;
                
                System.Threading.Monitor.Pulse(this);
            }
            
            while(true)
            {
                _thread.Join();
                break;
            }
        }
        
        public void add(Ice.ConnectionI connection)
        {
            lock(this)
            {
                Debug.Assert(instance_ != null);
                _connections.Add(connection);
            }
        }
        
        public void remove(Ice.ConnectionI connection)
        {
            lock(this)
            {
                Debug.Assert(instance_ != null);
                _connections.Remove(connection);
            }
        }
        
        //
        // Only for use by Instance.
        //
        internal ConnectionMonitor(Instance instance, int interval)
        {
            Debug.Assert(interval > 0);

            instance_ = instance;
            _interval = interval * 1000;
            _connections = new Set();
            
            string threadName = instance_.initializationData().properties.getProperty("Ice.ProgramName");
            if(threadName.Length > 0)
            {
                threadName += "-";
            }

            _thread = new Thread(new ThreadStart(Run));
            _thread.IsBackground = true;
            _thread.Name = threadName + "Ice.ConnectionMonitor";
            _thread.Start();
        }
        
        public void Run()
        {
            Set connections = new Set();
            
            while(true)
            {
                lock(this)
                {
                    if(instance_ != null)
                    {
                        System.Threading.Monitor.Wait(this, _interval);
                    }
                    
                    if(instance_ == null)
                    {
                        return;
                    }
                    
                    connections.Clear();
                    foreach(Ice.ConnectionI connection in _connections)
                    {
                        connections.Add(connection);
                    }
                }
                
                //
                // Monitor connections outside the thread synchronization,
                // so that connections can be added or removed during
                // monitoring.
                //
                foreach(Ice.ConnectionI connection in connections)
                {
                    try
                    {
                        connection.monitor();
                    }
                    catch(Ice.LocalException ex)
                    {
                        lock(this)
                        {
                            if(instance_ == null)
                            {
                                return;
                            }
                            instance_.initializationData().logger.error("exception in connection monitor thread " + _thread.Name +
                                                     "::\n" + ex);
                        }
                    }
                    catch(System.Exception ex)
                    {
                        lock(this)
                        {
                            if(instance_ == null)
                            {
                                return;
                            }
                            instance_.initializationData().logger.error("unknown exception in connection monitor thread " +
                                                     _thread.Name + ":\n" + ex);
                        }
                    }
                }
            }
        }
        
        private Instance instance_;
        private readonly int _interval;
        private Set _connections;
        private Thread _thread;
    }

}
