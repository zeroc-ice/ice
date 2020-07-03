//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public interface IAcmMonitor
    {
        Acm Acm { get; }

        void Add(Connection connection);
        void Reap(Connection connection);
        void Remove(Connection connection);
        IAcmMonitor Create(TimeSpan? timeout, AcmClose? close, AcmHeartbeat? hearbeat);
    }

    internal sealed class AcmConfig
    {
        internal AcmClose Close { get; }
        internal AcmHeartbeat Heartbeat { get; }
        internal TimeSpan Timeout { get; }

        internal AcmConfig(bool server)
        {
            Timeout = TimeSpan.FromSeconds(60);
            Heartbeat = AcmHeartbeat.HeartbeatOnDispatch;
            Close = server ? AcmClose.CloseOnInvocation : AcmClose.CloseOnInvocationAndIdle;
        }

        internal AcmConfig(TimeSpan timeout, AcmClose close, AcmHeartbeat heartbeat)
        {
            Timeout = timeout;
            Close = close;
            Heartbeat = heartbeat;
        }

        internal AcmConfig(Communicator communicator, string prefix, AcmConfig defaults)
        {
            Timeout = communicator.GetPropertyAsTimeSpan($"{prefix}.Timeout") ?? defaults.Timeout;

            if (communicator.GetPropertyAsInt($"{prefix}.Heartbeat") is int hearbeat)
            {
                if (hearbeat < (int)AcmHeartbeat.HeartbeatOff || hearbeat > (int)AcmHeartbeat.HeartbeatAlways)
                {
                    throw new InvalidConfigurationException($"invalid value for property `{prefix}.Heartbeat'");
                }
                Heartbeat = (AcmHeartbeat)hearbeat;
            }
            else
            {
                Heartbeat = defaults.Heartbeat;
            }

            if (communicator.GetPropertyAsInt($"{prefix}.Close") is int close)
            {
                if (close < (int)AcmClose.CloseOff || close > (int)AcmClose.CloseOnIdleForceful)
                {
                    throw new InvalidConfigurationException($"invalid value for property `{ prefix}.Close'");
                }
                Close = (AcmClose)close;
            }
            else
            {
                Close = defaults.Close;
            }
        }
    }

    internal class ConnectionFactoryAcmMonitor : IAcmMonitor
    {
        public Acm Acm => new Acm(_config.Timeout, _config.Close, _config.Heartbeat);

        private readonly List<(Connection Connection, bool Remove)> _changes =
            new List<(Connection Connection, bool Remove)>();
        private Communicator? _communicator;
        private readonly AcmConfig _config;
        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private readonly object _mutex = new object();
        private List<Connection> _reapedConnections = new List<Connection>();
        private Timer? _timer;

        public void Add(Connection connection)
        {
            if (_config.Timeout == TimeSpan.Zero || _config.Timeout == Timeout.InfiniteTimeSpan)
            {
                return;
            }

            lock (_mutex)
            {
                if (_connections.Count == 0)
                {
                    Debug.Assert(_communicator != null);
                    _connections.Add(connection);
                    _timer = new Timer(RunTimerTask, this, _config.Timeout / 2, _config.Timeout / 2);
                }
                else
                {
                    _changes.Add((connection, false));
                }
            }
        }

        public IAcmMonitor Create(TimeSpan? timeout, AcmClose? close, AcmHeartbeat? heartbeat) =>
            new ConnectionAcmMonitor(this,
                                     new AcmConfig(timeout ?? _config.Timeout,
                                                   close ?? _config.Close,
                                                   heartbeat ?? _config.Heartbeat));

        public void Reap(Connection connection)
        {
            lock (_mutex)
            {
                _reapedConnections.Add(connection);
            }
        }

        public void Remove(Connection connection)
        {
            if (_config.Timeout == TimeSpan.Zero || _config.Timeout == Timeout.InfiniteTimeSpan)
            {
                return;
            }

            lock (_mutex)
            {
                Debug.Assert(_communicator != null);
                _changes.Add((connection, true));
            }
        }

        internal ConnectionFactoryAcmMonitor(Communicator communicator, AcmConfig config)
        {
            _communicator = communicator;
            _config = config;
        }

        internal void Destroy()
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    // Ensure all the connections have been cleared, it's important to wait here
                    // to prevent the timer destruction in Instance::destroy.
                    while (_connections.Count > 0)
                    {
                        Monitor.Wait(_mutex);
                    }
                    return;
                }

                if (_connections.Count > 0)
                {
                    // Dispose the timer task and schedule the task on the thread pool to clear the connection set.
                    _timer?.Dispose();
                    _timer = null;
                    Task.Run(() => RunTimerTask(this));
                }

                _communicator = null;
                _changes.Clear();

                // Wait for the connection set to be cleared.
                while (_connections.Count > 0)
                {
                    Monitor.Wait(_mutex);
                }
            }
        }

        internal IEnumerable<Connection> SwapReapedConnections()
        {
            lock (_mutex)
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

        internal void HandleException(Exception ex)
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    return;
                }
                _communicator.Logger.Error("exception in connection monitor:\n" + ex);
            }
        }

        private void RunTimerTask(object? state)
        {
            lock (_mutex)
            {
                if (_communicator == null)
                {
                    _connections.Clear();
                    Monitor.PulseAll(_mutex);
                    return;
                }

                foreach ((Connection connection, bool remove) in _changes)
                {
                    if (remove)
                    {
                        _connections.Remove(connection);
                    }
                    else
                    {
                        _connections.Add(connection);
                    }
                }
                _changes.Clear();

                if (_connections.Count == 0)
                {
                    if (_timer != null)
                    {
                        _timer.Dispose();
                        _timer = null;
                    }
                    return;
                }
            }

            // Monitor connections outside the thread synchronization, so that connections can be added or removed
            // during monitoring.
            TimeSpan now = Time.CurrentMonotonicTime();
            foreach (Connection connection in _connections)
            {
                try
                {
                    connection.Monitor(now, _config);
                }
                catch (Exception ex)
                {
                    HandleException(ex);
                }
            }
        }
    }

    internal class ConnectionAcmMonitor : IAcmMonitor
    {
        public Acm Acm => new Acm(_config.Timeout, _config.Close, _config.Heartbeat);

        private readonly AcmConfig _config;
        private Connection? _connection;
        private readonly object _mutex = new object();
        private readonly ConnectionFactoryAcmMonitor _parent;
        private Timer? _timer;

        internal ConnectionAcmMonitor(ConnectionFactoryAcmMonitor parent, AcmConfig config)
        {
            _parent = parent;
            _config = config;
        }

        public void Add(Connection connection)
        {
            lock (_mutex)
            {
                Debug.Assert(_connection == null);
                _connection = connection;
                if (_config.Timeout != TimeSpan.Zero && _config.Timeout != Timeout.InfiniteTimeSpan)
                {
                    _timer = new Timer(RunTimerTask, this, _config.Timeout, _config.Timeout);
                }
            }
        }

        public IAcmMonitor Create(TimeSpan? timeout, AcmClose? close, AcmHeartbeat? heartbeat) =>
            _parent.Create(timeout, close, heartbeat);

        public void Reap(Connection connection) => _parent.Reap(connection);

        public void Remove(Connection connection)
        {
            lock (_mutex)
            {
                Debug.Assert(_connection == connection);
                _connection = null;
                if (_timer != null)
                {
                    _timer.Dispose();
                    _timer = null;
                }
            }
        }

        private void RunTimerTask(object? state)
        {
            Connection connection;
            lock (_mutex)
            {
                if (_connection == null)
                {
                    return;
                }
                connection = _connection;
            }

            try
            {
                connection.Monitor(Time.CurrentMonotonicTime(), _config);
            }
            catch (Exception ex)
            {
                _parent.HandleException(ex);
            }
        }
    }
}
