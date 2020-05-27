//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace ZeroC.Ice
{
    internal sealed class ACMConfig
    {
        internal ACMConfig(bool server)
        {
            Timeout = 60 * 1000;
            Heartbeat = ACMHeartbeat.HeartbeatOnDispatch;
            Close = server ? ACMClose.CloseOnInvocation : ACMClose.CloseOnInvocationAndIdle;
        }

        public ACMConfig(Communicator communicator, ILogger logger, string prefix, ACMConfig defaults)
        {
            Debug.Assert(prefix != null);

            string timeoutProperty;
            if ((prefix == "Ice.ACM.Client" || prefix == "Ice.ACM.Server") &&
                communicator.GetProperty($"{prefix}.Timeout") == null)
            {
                timeoutProperty = prefix; // Deprecated property.
            }
            else
            {
                timeoutProperty = prefix + ".Timeout";
            }

            Timeout = communicator.GetPropertyAsInt(timeoutProperty) * 1000 ?? defaults.Timeout;
            if (Timeout < 0)
            {
                logger.Warning($"invalid value for property `{timeoutProperty}', default value will be used instead");
                Timeout = defaults.Timeout;
            }

            int hb = communicator.GetPropertyAsInt($"{prefix}.Heartbeat") ?? (int)defaults.Heartbeat;
            if (hb >= (int)ACMHeartbeat.HeartbeatOff && hb <= (int)ACMHeartbeat.HeartbeatAlways)
            {
                Heartbeat = (ACMHeartbeat)hb;
            }
            else
            {
                logger.Warning($"invalid value for property `{prefix}.Heartbeat', default value will be used instead");
                Heartbeat = defaults.Heartbeat;
            }

            int cl = communicator.GetPropertyAsInt($"{prefix}.Close") ?? (int)defaults.Close;
            if (cl >= (int)ACMClose.CloseOff && cl <= (int)ACMClose.CloseOnIdleForceful)
            {
                Close = (ACMClose)cl;
            }
            else
            {
                logger.Warning($"invalid value for property `{prefix}.Close', default value will be used instead");
                Close = defaults.Close;
            }
        }

        public ACMConfig Clone() => (ACMConfig)MemberwiseClone();

        public int Timeout;
        public ACMHeartbeat Heartbeat;
        public ACMClose Close;
    }

    public interface IACMMonitor : ITimerTask
    {
        void Add(Connection con);
        void Remove(Connection con);
        void Reap(Connection con);

        IACMMonitor Acm(int? timeout, ACMClose? c, ACMHeartbeat? h);
        ACM GetACM();
    }

    internal class FactoryACMMonitor : IACMMonitor
    {
        internal class Change
        {
            internal Change(Connection connection, bool remove)
            {
                Connection = connection;
                Remove = remove;
            }

            public readonly Connection Connection;
            public readonly bool Remove;
        }

        internal FactoryACMMonitor(Communicator communicator, ACMConfig config)
        {
            _communicator = communicator;
            _config = config;
        }

        internal void Destroy()
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    //
                    // Ensure all the connections have been cleared, it's important to wait here
                    // to prevent the timer destruction in Instance::destroy.
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
                    _communicator.Timer().Cancel(this);
                    _communicator.Timer().Schedule(this, 0);
                }

                _communicator = null;
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

        public void Add(Connection connection)
        {
            if (_config.Timeout == 0)
            {
                return;
            }

            lock (this)
            {
                if (_connections.Count == 0)
                {
                    Debug.Assert(_communicator != null);
                    _connections.Add(connection);
                    _communicator.Timer().ScheduleRepeated(this, _config.Timeout / 2);
                }
                else
                {
                    _changes.Add(new Change(connection, false));
                }
            }
        }

        public void Remove(Connection connection)
        {
            if (_config.Timeout == 0)
            {
                return;
            }

            lock (this)
            {
                Debug.Assert(_communicator != null);
                _changes.Add(new Change(connection, true));
            }
        }

        public void Reap(Connection connection)
        {
            lock (this)
            {
                _reapedConnections.Add(connection);
            }
        }

        public IACMMonitor Acm(int? timeout, ACMClose? c, ACMHeartbeat? h)
        {
            Debug.Assert(_communicator != null);

            ACMConfig config = _config.Clone();
            if (timeout.HasValue)
            {
                config.Timeout = timeout.Value * 1000; // To milliseconds
            }
            if (c.HasValue)
            {
                config.Close = c.Value;
            }
            if (h.HasValue)
            {
                config.Heartbeat = h.Value;
            }
            return new ConnectionACMMonitor(this, _communicator.Timer(), config);
        }

        public ACM GetACM()
        {
            return new ACM
            {
                Timeout = _config.Timeout / 1000,
                Close = _config.Close,
                Heartbeat = _config.Heartbeat
            };
        }

        internal IEnumerable<Connection> SwapReapedConnections()
        {
            lock (this)
            {
                if (_reapedConnections.Count == 0)
                {
                    return Enumerable.Empty<Connection>();
                }
                List<Connection> connections = _reapedConnections;
                _reapedConnections = new List<Connection>();
                return connections;
            }
        }

        public void RunTimerTask()
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    _connections.Clear();
                    System.Threading.Monitor.PulseAll(this);
                    return;
                }

                foreach (Change change in _changes)
                {
                    if (change.Remove)
                    {
                        _connections.Remove(change.Connection);
                    }
                    else
                    {
                        _connections.Add(change.Connection);
                    }
                }
                _changes.Clear();

                if (_connections.Count == 0)
                {
                    _communicator.Timer().Cancel(this);
                    return;
                }
            }

            //
            // Monitor connections outside the thread synchronization, so
            // that connections can be added or removed during monitoring.
            //
            long now = Time.CurrentMonotonicTimeMillis();
            foreach (Connection connection in _connections)
            {
                try
                {
                    connection.Monitor(now, _config);
                }
                catch (System.Exception ex)
                {
                    HandleException(ex);
                }
            }
        }

        internal void HandleException(System.Exception ex)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    return;
                }
                _communicator.Logger.Error("exception in connection monitor:\n" + ex);
            }
        }

        private Communicator? _communicator;
        private readonly ACMConfig _config;

        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private readonly List<Change> _changes = new List<Change>();
        private List<Connection> _reapedConnections = new List<Connection>();
    }

    internal class ConnectionACMMonitor : IACMMonitor
    {
        internal ConnectionACMMonitor(FactoryACMMonitor parent, Timer timer, ACMConfig config)
        {
            _parent = parent;
            _timer = timer;
            _config = config;
        }

        public void Add(Connection connection)
        {
            lock (this)
            {
                Debug.Assert(_connection == null);
                _connection = connection;
                if (_config.Timeout > 0)
                {
                    _timer.ScheduleRepeated(this, _config.Timeout / 2);
                }
            }
        }

        public void Remove(Connection connection)
        {
            lock (this)
            {
                Debug.Assert(_connection == connection);
                _connection = null;
                if (_config.Timeout > 0)
                {
                    _timer.Cancel(this);
                }
            }
        }

        public void Reap(Connection connection) => _parent.Reap(connection);

        public IACMMonitor Acm(int? timeout, ACMClose? c, ACMHeartbeat? h) => _parent.Acm(timeout, c, h);

        public ACM GetACM()
        {
            return new ACM
            {
                Timeout = _config.Timeout / 1000,
                Close = _config.Close,
                Heartbeat = _config.Heartbeat
            };
        }

        public void RunTimerTask()
        {
            Connection connection;
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
                connection.Monitor(Time.CurrentMonotonicTimeMillis(), _config);
            }
            catch (System.Exception ex)
            {
                _parent.HandleException(ex);
            }
        }

        private readonly FactoryACMMonitor _parent;
        private readonly Timer _timer;
        private readonly ACMConfig _config;

        private Connection? _connection;
    }
}
