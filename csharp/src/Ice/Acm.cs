//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>This interface represents a monitor for the Acm (Active Connection Management)</summary>
    public interface IAcmMonitor
    {
        /// <summary>Gets the Acm configuration for this monitor.</summary>
        Acm Acm { get; }

        /// <summary>Adds a new connection to the set of monitored connections.</summary>
        /// <param name="connection">The connection to monitor for activity.</param>
        void Add(Connection connection);

        /// <summary>Adds a connection to the set of reaped connections.</summary>
        /// <param name="connection">The connection to reap.</param>
        void Reap(Connection connection);

        /// <summary>Removes a connection from the set of monitored connections.</summary>
        /// <param name="connection">The connection to remove.</param>
        void Remove(Connection connection);

        /// <summary>Creates a child Acm monitor with a specific Acm configuration.</summary>
        /// <param name="acm">The monitor Acm configuration.</param>
        /// <returns>Returns a new Acm monitor with the given configuration.</returns>
        IAcmMonitor Create(Acm acm);
    }

    internal class ConnectionFactoryAcmMonitor : IAcmMonitor
    {
        public Acm Acm { get; }

        private readonly List<(Connection Connection, bool Remove)> _changes =
            new List<(Connection Connection, bool Remove)>();
        private readonly Communicator _communicator;
        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private readonly object _mutex = new object();
        private List<Connection> _reapedConnections = new List<Connection>();
        private Timer? _timer;

        public void Add(Connection connection)
        {
            if (Acm.Timeout == TimeSpan.Zero || Acm.Timeout == Timeout.InfiniteTimeSpan)
            {
                return;
            }

            lock (_mutex)
            {
                if (_connections.Count == 0)
                {
                    _connections.Add(connection);
                    _timer = new Timer(RunTimerTask, this, Acm.Timeout / 2, Acm.Timeout / 2);
                }
                else
                {
                    _changes.Add((connection, false));
                }
            }
        }

        public IAcmMonitor Create(Acm acm) => new ConnectionAcmMonitor(this, acm, _communicator.Logger);

        public void Reap(Connection connection)
        {
            lock (_mutex)
            {
                _reapedConnections.Add(connection);
            }
        }

        public void Remove(Connection connection)
        {
            if (Acm.Timeout == TimeSpan.Zero || Acm.Timeout == Timeout.InfiniteTimeSpan)
            {
                return;
            }

            lock (_mutex)
            {
                _changes.Add((connection, true));
            }
        }

        internal ConnectionFactoryAcmMonitor(Communicator communicator, Acm acm)
        {
            _communicator = communicator;
            Acm = acm;
        }

        internal void Destroy()
        {
            lock (_mutex)
            {
                _timer?.Dispose();
                _timer = null;
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

        private void RunTimerTask(object? state)
        {
            lock (_mutex)
            {
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
            TimeSpan now = Time.Elapsed;
            foreach (Connection connection in _connections)
            {
                try
                {
                    connection.Monitor(now, Acm);
                }
                catch (Exception ex)
                {
                    _communicator.Logger.Error($"exception in connection monitor:\n{ex}");
                }
            }
        }
    }

    internal class ConnectionAcmMonitor : IAcmMonitor
    {
        public Acm Acm { get; }

        private Connection? _connection;
        private readonly ILogger _logger;
        private readonly object _mutex = new object();
        private readonly ConnectionFactoryAcmMonitor _parent;
        private Timer? _timer;

        internal ConnectionAcmMonitor(ConnectionFactoryAcmMonitor parent, Acm acm, ILogger logger)
        {
            _parent = parent;
            Acm = acm;
            _logger = logger;
        }

        public void Add(Connection connection)
        {
            lock (_mutex)
            {
                Debug.Assert(_connection == null);
                _connection = connection;
                if (Acm.Timeout != TimeSpan.Zero && Acm.Timeout != Timeout.InfiniteTimeSpan)
                {
                    _timer = new Timer(RunTimerTask, this, Acm.Timeout, Acm.Timeout);
                }
            }
        }

        public IAcmMonitor Create(Acm acm) => _parent.Create(acm);

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
                connection.Monitor(Time.Elapsed, Acm);
            }
            catch (Exception ex)
            {
                _logger.Error($"exception in connection monitor:\n{ex}");
            }
        }
    }
}
