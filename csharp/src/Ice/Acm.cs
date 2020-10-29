// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>Specifies the close semantics for ACM (Active Connection Management).</summary>
    public enum AcmClose
    {
        /// <summary>Disables automatic connection closure.</summary>
        Off,
        /// <summary>Gracefully closes a connection that has been idle for the configured timeout period.</summary>
        OnIdle,
        /// <summary>Forcefully closes a connection that has been idle for the configured timeout period, but only if
        /// the connection has pending invocations.</summary>
        OnInvocation,
        /// <summary>Combines the behaviors of CloseOnIdle and CloseOnInvocation.</summary>
        OnInvocationAndIdle,
        /// <summary>Forcefully closes a connection that has been idle for the configured timeout period, regardless of
        /// whether the connection has pending invocations or dispatch.</summary>
        OnIdleForceful
    }

    /// <summary>Specifies the heartbeat semantics for ACM (Active Connection Management).</summary>
    public enum AcmHeartbeat
    {
        /// <summary>Disables heartbeats.</summary>
        Off,
        /// <summary>Send a heartbeat at regular intervals if the connection is idle and only if there are pending
        /// dispatch.</summary>
        OnDispatch,
        /// <summary>Send a heartbeat at regular intervals when the connection is idle.</summary>
        OnIdle,
        /// <summary>Send a heartbeat at regular intervals until the connection is closed.</summary>
        Always
    }

    /// <summary>This struct represents the Acm (Active Connection Management) configuration.</summary>
    public readonly struct Acm : IEquatable<Acm>
    {
        /// <summary>Gets the Acm configuration for disabling activation connection management.</summary>
        public static readonly Acm Disabled =
            new Acm(System.Threading.Timeout.InfiniteTimeSpan, AcmClose.Off, AcmHeartbeat.Off);

        /// <summary>Gets the <see cref="AcmClose"/> setting for the Acm configuration.</summary>
        public AcmClose Close { get; }
        /// <summary>Gets <see cref="AcmHeartbeat"/> setting for the Acm configuration.</summary>
        public AcmHeartbeat Heartbeat { get; }
        /// <summary>Returns true if ACM is disabled, false otherwise.</summary>
        public bool IsDisabled => (Close == AcmClose.Off && Heartbeat == AcmHeartbeat.Off) ||
            Timeout == System.Threading.Timeout.InfiniteTimeSpan;

        /// <summary>Gets the timeout setting for the Acm configuration.</summary>
        public TimeSpan Timeout { get; }

        internal static Acm ServerDefault =>
            new Acm(TimeSpan.FromSeconds(60), AcmClose.OnInvocation, AcmHeartbeat.OnDispatch);
        internal static Acm ClientDefault =>
            new Acm(TimeSpan.FromSeconds(60), AcmClose.OnInvocationAndIdle, AcmHeartbeat.OnDispatch);

        /// <summary>Creates an Acm configuration structure.</summary>
        /// <param name="timeout">The timeout setting. The timeout must be a positive, non-zero value.</param>
        /// <param name="close">The close setting.</param>
        /// <param name="heartbeat">The heartbeat setting.</param>
        public Acm(TimeSpan timeout, AcmClose close, AcmHeartbeat heartbeat)
        {
            if (timeout != System.Threading.Timeout.InfiniteTimeSpan && timeout <= TimeSpan.Zero)
            {
                throw new ArgumentOutOfRangeException($"invalid {nameof(timeout)} argument");
            }

            Timeout = timeout;
            Close = close;
            Heartbeat = heartbeat;
        }

        /// <inheritdoc/>
        public override int GetHashCode() => HashCode.Combine(Timeout, Close, Heartbeat);

        /// <inheritdoc/>
        public bool Equals(Acm other) =>
            Timeout == other.Timeout && Close == other.Close && Heartbeat == other.Heartbeat;

        /// <inheritdoc/>
        public override bool Equals(object? other) => other is Acm value && Equals(value);

        /// <summary>The equality operator == returns true if its operands are equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are equal, otherwise <c>false</c>.</returns>
        public static bool operator ==(Acm lhs, Acm rhs) => Equals(lhs, rhs);

        /// <summary>The inequality operator != returns true if its operands are not equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are not equal, otherwise <c>false</c>.</returns>
        public static bool operator !=(Acm lhs, Acm rhs) => !Equals(lhs, rhs);

        internal Acm(Communicator communicator, string prefix, Acm defaults)
        {
            Timeout = communicator.GetPropertyAsTimeSpan($"{prefix}.Timeout") ?? defaults.Timeout;
            if (Timeout == TimeSpan.Zero)
            {
                throw new ArgumentOutOfRangeException($"0 is not a valid value for `{prefix}.Timeout'");
            }

            Heartbeat = communicator.GetPropertyAsEnum<AcmHeartbeat>($"{prefix}.Heartbeat") ?? defaults.Heartbeat;
            Close = communicator.GetPropertyAsEnum<AcmClose>($"{prefix}.Close") ?? defaults.Close;
        }
    }

    /// <summary>This interface represents a monitor for the Acm (Active Connection Management).</summary>
    public interface IAcmMonitor
    {
        /// <summary>Gets the Acm configuration for this monitor.</summary>
        Acm Acm { get; }

        /// <summary>Adds a new connection to the set of monitored connections.</summary>
        /// <param name="connection">The connection to monitor for activity.</param>
        void Add(Connection connection);

        /// <summary>Removes a connection from the set of monitored connections.</summary>
        /// <param name="connection">The connection to remove.</param>
        void Remove(Connection connection);
    }

    internal class ConnectionFactoryAcmMonitor : IAcmMonitor
    {
        public Acm Acm { get; }

        private readonly List<(Connection Connection, bool Remove)> _changes =
            new List<(Connection Connection, bool Remove)>();
        private readonly Communicator _communicator;
        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private readonly object _mutex = new object();
        private Timer? _timer;

        public void Add(Connection connection)
        {
            if (!Acm.IsDisabled)
            {
                lock (_mutex)
                {
                    // Unless it's the first connection, we don't add the connection directly to the _connections set.
                    // The _connections set can only be accessed by the timer when it's set. The goal here is to avoid
                    // copying _connections or holding a lock while iterating over all the connections to call Monitor.
                    // There could be thousands of connection in this set. So instead of modifying the connection set
                    // in Add/Remove, we add the Add/Remove request to the _changes list which is then processed by
                    // the timer to add/remove connections from the connection set.
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
        }

        public void Remove(Connection connection)
        {
            if (!Acm.IsDisabled)
            {
                // See Add comment for the reason why we don't directly modify the _connections set.
                lock (_mutex)
                {
                    Debug.Assert(_connections.Count > 0);
                    _changes.Add((connection, true));
                }
            }
        }

        internal ConnectionFactoryAcmMonitor(Communicator communicator, Acm acm)
        {
            _communicator = communicator;
            Acm = acm;
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

        private readonly ILogger _logger;
        private Timer? _timer;

        internal ConnectionAcmMonitor(Acm acm, ILogger logger)
        {
            Acm = acm;
            _logger = logger;
        }

        public void Add(Connection connection)
        {
            if (!Acm.IsDisabled)
            {
                _timer = new Timer(_ =>
                {
                    try
                    {
                        connection.Monitor(Time.Elapsed, Acm);
                    }
                    catch (Exception ex)
                    {
                        _logger.Error($"exception in connection monitor:\n{ex}");
                    }
                },
                null,
                Acm.Timeout,
                Acm.Timeout);
            }
        }

        public void Remove(Connection connection) => _timer?.Dispose();
    }
}
