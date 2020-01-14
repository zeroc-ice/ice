//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    [Serializable]
    public class ConnectionInfo
    {
        public ConnectionInfo? Underlying;
        public bool Incoming;
        public string? AdapterName;
        public string ConnectionId;

        public ConnectionInfo()
        {
            AdapterName = "";
            ConnectionId = "";
        }

        public ConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId)
        {
            Underlying = underlying;
            Incoming = incoming;
            AdapterName = adapterName;
            ConnectionId = connectionId;
        }
    }

    public delegate void CloseCallback(Connection con);
    public delegate void HeartbeatCallback(Connection con);

    public enum ACMClose
    {
        CloseOff,
        CloseOnIdle,
        CloseOnInvocation,
        CloseOnInvocationAndIdle,
        CloseOnIdleForceful
    }

    public enum ACMHeartbeat
    {
        HeartbeatOff,
        HeartbeatOnDispatch,
        HeartbeatOnIdle,
        HeartbeatAlways
    }

    [Serializable]
    public struct ACM : IEquatable<ACM>
    {
        public int Timeout;
        public ACMClose Close;
        public ACMHeartbeat Heartbeat;

        public ACM(int timeout, ACMClose close, ACMHeartbeat heartbeat)
        {
            Timeout = timeout;
            Close = close;
            Heartbeat = heartbeat;
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ACM");
            IceInternal.HashUtil.hashAdd(ref h_, Timeout);
            IceInternal.HashUtil.hashAdd(ref h_, Close);
            IceInternal.HashUtil.hashAdd(ref h_, Heartbeat);
            return h_;
        }

        public bool Equals(ACM other)
        {
            return Timeout == other.Timeout && Close == other.Close && Heartbeat == other.Heartbeat;
        }

        public override bool Equals(object other)
        {
            return ReferenceEquals(this, other) || (other is ACM value && Equals(value));
        }

        public static bool operator ==(ACM lhs, ACM rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ACM lhs, ACM rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    public enum ConnectionClose
    {
        Forcefully,
        Gracefully,
        GracefullyWithWait
    }

    public interface Connection
    {
        /// <summary>
        /// Manually close the connection using the specified closure mode.
        /// </summary>
        /// <param name="mode">Determines how the connection will be closed.
        ///
        /// </param>
        void close(ConnectionClose mode);

        /// <summary>
        /// Create a special proxy that always uses this connection.
        /// This
        /// can be used for callbacks from a server to a client if the
        /// server cannot directly establish a connection to the client,
        /// for example because of firewalls. In this case, the server
        /// would create a proxy using an already established connection
        /// from the client.
        ///
        /// </summary>
        /// <param name="id">The identity for which a proxy is to be created.
        ///
        /// </param>
        /// <returns>A proxy that matches the given identity and uses this
        /// connection.
        ///
        /// </returns>
        IObjectPrx createProxy(Identity id);

        /// <summary>
        /// Explicitly set an object adapter that dispatches requests that
        /// are received over this connection.
        /// A client can invoke an
        /// operation on a server using a proxy, and then set an object
        /// adapter for the outgoing connection that is used by the proxy
        /// in order to receive callbacks. This is useful if the server
        /// cannot establish a connection back to the client, for example
        /// because of firewalls.
        ///
        /// </summary>
        /// <param name="adapter">The object adapter that should be used by this
        /// connection to dispatch requests. The object adapter must be
        /// activated. When the object adapter is deactivated, it is
        /// automatically removed from the connection. Attempts to use a
        /// deactivated object adapter raise ObjectAdapterDeactivatedException
        ///
        /// </param>
        void setAdapter(ObjectAdapter adapter);

        /// <summary>
        /// Get the object adapter that dispatches requests for this
        /// connection.
        /// </summary>
        /// <returns>The object adapter that dispatches requests for the
        /// connection, or null if no adapter is set.
        ///
        /// </returns>
        ObjectAdapter getAdapter();

        /// <summary>
        /// Get the endpoint from which the connection was created.
        /// </summary>
        /// <returns>The endpoint from which the connection was created.</returns>
        IEndpoint getEndpoint();

        /// <summary>
        /// Set a close callback on the connection.
        /// The callback is called by the
        /// connection when it's closed. The callback is called from the
        /// Ice thread pool associated with the connection. If the callback needs
        /// more information about the closure, it can call Connection.throwException.
        ///
        /// </summary>
        /// <param name="callback">The close callback object.</param>
        void setCloseCallback(CloseCallback callback);

        /// <summary>
        /// Set a heartbeat callback on the connection.
        /// The callback is called by the
        /// connection when a heartbeat is received. The callback is called
        /// from the Ice thread pool associated with the connection.
        ///
        /// </summary>
        /// <param name="callback">The heartbeat callback object.</param>
        void setHeartbeatCallback(HeartbeatCallback callback);

        /// <summary>
        /// Send a heartbeat message.
        /// </summary>
        void heartbeat();

        System.Threading.Tasks.Task heartbeatAsync(
            IProgress<bool>? progress = null,
            System.Threading.CancellationToken cancel = new System.Threading.CancellationToken());

        /// <summary>
        /// Set the active connection management parameters.
        /// </summary>
        /// <param name="timeout">The timeout value in seconds, must be &gt;= 0.
        ///
        /// </param>
        /// <param name="close">The close condition
        ///
        /// </param>
        /// <param name="heartbeat">The hertbeat condition</param>
        void setACM(int? timeout, ACMClose? close, ACMHeartbeat? heartbeat);

        /// <summary>
        /// Get the ACM parameters.
        /// </summary>
        /// <returns>The ACM parameters.</returns>
        ACM getACM();

        /// <summary>
        /// Return the connection type.
        /// This corresponds to the endpoint
        /// type, i.e., "tcp", "udp", etc.
        ///
        /// </summary>
        /// <returns>The type of the connection.</returns>
        string type();

        /// <summary>
        /// Get the timeout for the connection.
        /// </summary>
        /// <returns>The connection's timeout.</returns>
        int timeout();

        /// <summary>
        /// Return a description of the connection as human readable text,
        /// suitable for logging or error messages.
        /// </summary>
        /// <returns>The description of the connection as human readable
        /// text.</returns>
        string ice_toString_();

        /// <summary>
        /// Returns the connection information.
        /// </summary>
        /// <returns>The connection information.</returns>
        ConnectionInfo getInfo();

        /// <summary>
        /// Set the connection buffer receive/send size.
        /// </summary>
        /// <param name="rcvSize">The connection receive buffer size.
        /// </param>
        /// <param name="sndSize">The connection send buffer size.</param>
        void setBufferSize(int rcvSize, int sndSize);

        /// <summary>
        /// Throw an exception indicating the reason for connection closure.
        /// For example,
        /// CloseConnectionException is raised if the connection was closed gracefully,
        /// whereas ConnectionManuallyClosedException is raised if the connection was
        /// manually closed by the application. This operation does nothing if the connection is
        /// not yet closed.
        /// </summary>
        void throwException();
    }

    [Serializable]
    public class IPConnectionInfo : ConnectionInfo
    {
        public string LocalAddress;
        public int LocalPort;
        public string RemoteAddress;
        public int RemotePort;

        public IPConnectionInfo()
        {
            LocalAddress = "";
            LocalPort = -1;
            RemoteAddress = "";
            RemotePort = -1;
        }

        public IPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                string localAddress, int localPort, string remoteAddress, int remotePort) :
            base(underlying, incoming, adapterName, connectionId)
        {
            LocalAddress = localAddress;
            LocalPort = localPort;
            RemoteAddress = remoteAddress;
            RemotePort = remotePort;
        }
    }

    [Serializable]
    public class TCPConnectionInfo : IPConnectionInfo
    {
        public int RcvSize;
        public int SndSize;

        public TCPConnectionInfo()
        {
            RcvSize = 0;
            SndSize = 0;
        }

        public TCPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                 string localAddress, int localPort, string remoteAddress, int remotePort, int rcvSize,
                                 int sndSize) :
            base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            RcvSize = rcvSize;
            SndSize = sndSize;
        }
    }

    [Serializable]
    public class UDPConnectionInfo : IPConnectionInfo
    {
        public string McastAddress;
        public int McastPort;
        public int RcvSize;
        public int SndSize;

        public UDPConnectionInfo()
        {
            McastAddress = "";
            McastPort = -1;
            RcvSize = 0;
            SndSize = 0;
        }

        public UDPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
                                 string localAddress, int localPort, string remoteAddress, int remotePort,
                                 string mcastAddress, int mcastPort, int rcvSize, int sndSize) :
            base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            McastAddress = mcastAddress;
            McastPort = mcastPort;
            RcvSize = rcvSize;
            SndSize = sndSize;
        }
    }

    [Serializable]
    public class WSConnectionInfo : ConnectionInfo
    {
        public Dictionary<string, string>? Headers;

        public WSConnectionInfo()
        {
        }

        public WSConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId,
            Dictionary<string, string> headers) : base(underlying, incoming, adapterName, connectionId)
        {
            Headers = headers;
        }
    }
}
