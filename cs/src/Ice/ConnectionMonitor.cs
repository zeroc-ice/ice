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

    using System.Diagnostics;
    using System.Collections.Generic;

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

        public void checkIntervalForACM(int acmTimeout)
        {
            if(acmTimeout <= 0)
            {
                return;
            }

            //
            // If Ice.MonitorConnections isn't set (_interval == 0), the given ACM is used
            // to determine the check interval: 1/10 of the ACM timeout with a minmal value
            // of 5 seconds and a maximum value of 5 minutes.
            //
            // Note: if Ice.MonitorConnections is set, the timer is schedulded only if ACM 
            // is configured for the communicator or some object adapters.
            //
            int interval;
            if(_interval == 0)
            {
                interval = System.Math.Min(300, System.Math.Max(5, (int)acmTimeout / 10));
            }
            else if(_scheduledInterval == _interval)
            {
                return; // Nothing to do, the timer is already scheduled.
            }
            else
            {
                interval = _interval;
            }

            //
            // If no timer is scheduled yet or if the given ACM requires a smaller interval,
            // we re-schedule the timer.
            //
            lock(this)
            {
                if(_scheduledInterval == 0 || _scheduledInterval > interval)
                {
                    _scheduledInterval = interval;
                    instance_.timer().cancel(this);
                    instance_.timer().scheduleRepeated(this, interval * 1000);
                }
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
            instance_ = instance;
            _interval = interval;
            _scheduledInterval = 0;
            _connections = new HashSet<Ice.ConnectionI>();
        }
        
        public void runTimerTask()
        {
            Ice.ConnectionI[] connections = null;
            lock(this)
            {                    
                if(instance_ == null)
                {
                    return;
                }

                connections = new Ice.ConnectionI[_connections.Count];
                _connections.CopyTo(connections);
            }
                
            //
            // Monitor connections outside the thread synchronization,
            // so that connections can be added or removed during
            // monitoring.
            //
            long now = IceInternal.Time.currentMonotonicTimeMillis();
            foreach(Ice.ConnectionI connection in connections)
            {
                try
                {
                    connection.monitor(now);
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
        private readonly int _interval;
        private int _scheduledInterval;
        private HashSet<Ice.ConnectionI> _connections;
    }

}
