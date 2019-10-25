//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class ACMConfig : System.ICloneable
    {
        internal ACMConfig(bool server)
        {
            timeout = 60 * 1000;
            heartbeat = Ice.ACMHeartbeat.HeartbeatOnDispatch;
            close = server ? Ice.ACMClose.CloseOnInvocation : Ice.ACMClose.CloseOnInvocationAndIdle;
        }

        public ACMConfig(Ice.Properties p, Ice.Logger l, string prefix, ACMConfig dflt)
        {
            Debug.Assert(prefix != null);

            string timeoutProperty;
            if ((prefix.Equals("Ice.ACM.Client") || prefix.Equals("Ice.ACM.Server")) &&
               p.getProperty(prefix + ".Timeout").Length == 0)
            {
                timeoutProperty = prefix; // Deprecated property.
            }
            else
            {
                timeoutProperty = prefix + ".Timeout";
            }

            timeout = p.getPropertyAsIntWithDefault(timeoutProperty, dflt.timeout / 1000) * 1000; // To milliseconds
            if (timeout < 0)
            {
                l.warning("invalid value for property `" + timeoutProperty + "', default value will be used instead");
                timeout = dflt.timeout;
            }

            int hb = p.getPropertyAsIntWithDefault(prefix + ".Heartbeat", (int)dflt.heartbeat);
            if (hb >= (int)Ice.ACMHeartbeat.HeartbeatOff && hb <= (int)Ice.ACMHeartbeat.HeartbeatAlways)
            {
                heartbeat = (Ice.ACMHeartbeat)hb;
            }
            else
            {
                l.warning("invalid value for property `" + prefix + ".Heartbeat" +
                          "', default value will be used instead");
                heartbeat = dflt.heartbeat;
            }

            int cl = p.getPropertyAsIntWithDefault(prefix + ".Close", (int)dflt.close);
            if (cl >= (int)Ice.ACMClose.CloseOff && cl <= (int)Ice.ACMClose.CloseOnIdleForceful)
            {
                close = (Ice.ACMClose)cl;
            }
            else
            {
                l.warning("invalid value for property `" + prefix + ".Close" +
                          "', default value will be used instead");
                close = dflt.close;
            }
        }

        public object Clone()
        {
            return this.MemberwiseClone();
        }

        public int timeout;
        public Ice.ACMHeartbeat heartbeat;
        public Ice.ACMClose close;
    }

    public interface ACMMonitor : TimerTask
    {
        void add(Ice.ConnectionI con);
        void remove(Ice.ConnectionI con);
        void reap(Ice.ConnectionI con);

        ACMMonitor acm(Ice.Optional<int> timeout, Ice.Optional<Ice.ACMClose> c, Ice.Optional<Ice.ACMHeartbeat> h);
        Ice.ACM getACM();
    }

    public class FactoryACMMonitor : ACMMonitor
    {
        internal class Change
        {
            internal Change(Ice.ConnectionI connection, bool remove)
            {
                this.connection = connection;
                this.remove = remove;
            }

            public readonly Ice.ConnectionI connection;
            public readonly bool remove;
        }

        internal FactoryACMMonitor(Instance instance, ACMConfig config)
        {
            _instance = instance;
            _config = config;
        }

        internal void destroy()
        {
            lock (this)
            {
                if (_instance == null)
                {
                    //
                    // Ensure all the connections have been cleared, it's important to wait here
                    // to prevent the timer destruction in IceInternal::Instance::destroy.
                    //
                    while (_connections.Count > 0)
                    {
                        System.Threading.Monitor.Wait(this);
                    }
                    return;
                }

                if (_connections.Count > 0)
                {
                    //
                    // Cancel the scheduled timer task and schedule it again now to clear the
                    // connection set from the timer thread.
                    //
                    _instance.timer().cancel(this);
                    _instance.timer().schedule(this, 0);
                }

                _instance = null;
                _changes.Clear();

                //
                // Wait for the connection set to be cleared by the timer thread.
                //
                while (_connections.Count > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void add(Ice.ConnectionI connection)
        {
            if (_config.timeout == 0)
            {
                return;
            }

            lock (this)
            {
                if (_connections.Count == 0)
                {
                    _connections.Add(connection);
                    _instance.timer().scheduleRepeated(this, _config.timeout / 2);
                }
                else
                {
                    _changes.Add(new Change(connection, false));
                }
            }
        }

        public void remove(Ice.ConnectionI connection)
        {
            if (_config.timeout == 0)
            {
                return;
            }

            lock (this)
            {
                Debug.Assert(_instance != null);
                _changes.Add(new Change(connection, true));
            }
        }

        public void reap(Ice.ConnectionI connection)
        {
            lock (this)
            {
                _reapedConnections.Add(connection);
            }
        }

        public ACMMonitor acm(Ice.Optional<int> timeout, Ice.Optional<Ice.ACMClose> c, Ice.Optional<Ice.ACMHeartbeat> h)
        {
            Debug.Assert(_instance != null);

            ACMConfig config = (ACMConfig)_config.Clone();
            if (timeout.HasValue)
            {
                config.timeout = timeout.Value * 1000; // To milliseconds
            }
            if (c.HasValue)
            {
                config.close = c.Value;
            }
            if (h.HasValue)
            {
                config.heartbeat = h.Value;
            }
            return new ConnectionACMMonitor(this, _instance.timer(), config);
        }

        public Ice.ACM getACM()
        {
            Ice.ACM acm = new Ice.ACM();
            acm.timeout = _config.timeout / 1000;
            acm.close = _config.close;
            acm.heartbeat = _config.heartbeat;
            return acm;
        }

        internal List<Ice.ConnectionI> swapReapedConnections()
        {
            lock (this)
            {
                if (_reapedConnections.Count == 0)
                {
                    return null;
                }
                List<Ice.ConnectionI> connections = _reapedConnections;
                _reapedConnections = new List<Ice.ConnectionI>();
                return connections;
            }
        }

        public void runTimerTask()
        {
            lock (this)
            {
                if (_instance == null)
                {
                    _connections.Clear();
                    System.Threading.Monitor.PulseAll(this);
                    return;
                }

                foreach (Change change in _changes)
                {
                    if (change.remove)
                    {
                        _connections.Remove(change.connection);
                    }
                    else
                    {
                        _connections.Add(change.connection);
                    }
                }
                _changes.Clear();

                if (_connections.Count == 0)
                {
                    _instance.timer().cancel(this);
                    return;
                }
            }

            //
            // Monitor connections outside the thread synchronization, so
            // that connections can be added or removed during monitoring.
            //
            long now = Time.currentMonotonicTimeMillis();
            foreach (Ice.ConnectionI connection in _connections)
            {
                try
                {
                    connection.monitor(now, _config);
                }
                catch (System.Exception ex)
                {
                    handleException(ex);
                }
            }
        }

        internal void handleException(System.Exception ex)
        {
            lock (this)
            {
                if (_instance == null)
                {
                    return;
                }
                _instance.initializationData().logger.error("exception in connection monitor:\n" + ex);
            }
        }

        private Instance _instance;
        private readonly ACMConfig _config;

        private HashSet<Ice.ConnectionI> _connections = new HashSet<Ice.ConnectionI>();
        private List<Change> _changes = new List<Change>();
        private List<Ice.ConnectionI> _reapedConnections = new List<Ice.ConnectionI>();
    }

    internal class ConnectionACMMonitor : ACMMonitor
    {
        internal ConnectionACMMonitor(FactoryACMMonitor parent, Timer timer, ACMConfig config)
        {
            _parent = parent;
            _timer = timer;
            _config = config;
        }

        public void add(Ice.ConnectionI connection)
        {
            lock (this)
            {
                Debug.Assert(_connection == null);
                _connection = connection;
                if (_config.timeout > 0)
                {
                    _timer.scheduleRepeated(this, _config.timeout / 2);
                }
            }
        }

        public void remove(Ice.ConnectionI connection)
        {
            lock (this)
            {
                Debug.Assert(_connection == connection);
                _connection = null;
                if (_config.timeout > 0)
                {
                    _timer.cancel(this);
                }
            }
        }

        public void reap(Ice.ConnectionI connection)
        {
            _parent.reap(connection);
        }

        public ACMMonitor acm(Ice.Optional<int> timeout, Ice.Optional<Ice.ACMClose> c, Ice.Optional<Ice.ACMHeartbeat> h)
        {
            return _parent.acm(timeout, c, h);
        }

        public Ice.ACM getACM()
        {
            Ice.ACM acm = new Ice.ACM();
            acm.timeout = _config.timeout / 1000;
            acm.close = _config.close;
            acm.heartbeat = _config.heartbeat;
            return acm;
        }

        public void runTimerTask()
        {
            Ice.ConnectionI connection;
            lock (this)
            {
                if (_connection == null)
                {
                    return;
                }
                connection = _connection;
            }

            try
            {
                connection.monitor(Time.currentMonotonicTimeMillis(), _config);
            }
            catch (System.Exception ex)
            {
                _parent.handleException(ex);
            }
        }

        private readonly FactoryACMMonitor _parent;
        private readonly Timer _timer;
        private readonly ACMConfig _config;

        private Ice.ConnectionI _connection;
    }
}
