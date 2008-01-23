// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;
    using IceUtilInternal;

    public sealed class ConnectionMonitor : TimerTask
    {
        public void destroy()
        {
            lock(this)
            {
                Debug.Assert(instance_ != null);
                
                instance_ = null;
                _connections = null;
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
            _connections = new Set();

            instance_.timer().scheduleRepeated(this, interval * 1000);
        }
        
        public void runTimerTask()
        {
            Set connections = new Set();
            lock(this)
            {                    
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
                catch(System.Exception ex)
                {
                    lock(this)
                    {
                        if(instance_ == null)
                        {
                            return;
                        }
                        instance_.initializationData().logger.error("unknown exception in connection monitor:\n" + ex);
                    }
                }
            }
        }
        
        private Instance instance_;
        private Set _connections;
    }

}
