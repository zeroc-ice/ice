// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice
{
    /// <summary>
    /// The batch compression option when flushing queued batch requests.
    /// </summary>
    public enum CompressBatch
    {
        /// <summary>
        /// Compress the batch requests.
        /// </summary>

        Yes,
        /// <summary>
        /// Don't compress the batch requests.
        /// </summary>

        No,
        /// <summary>
        /// Compress the batch requests if at least one request was made on a compressed proxy.
        /// </summary>

        BasedOnProxy
    }

    public class ConnectionInfo
    {
        public ConnectionInfo? underlying;
        public bool incoming;
        public string adapterName;
        public string connectionId;

        public ConnectionInfo()
        {
            adapterName = "";
            connectionId = "";
        }

        public ConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId)
        {
            this.underlying = underlying;
            this.incoming = incoming;
            this.adapterName = adapterName;
            this.connectionId = connectionId;
        }
    }

    public delegate void CloseCallback(Connection con);

    public delegate void HeartbeatCallback(Connection con);

    /// <summary>
    /// Determines the behavior when manually closing a connection.
    /// </summary>
    public enum ConnectionClose
    {
        /// <summary>
        /// Close the connection immediately without sending a close connection protocol message to the peer and waiting
        ///  for the peer to acknowledge it.
        /// </summary>

        Forcefully,
        /// <summary>
        /// Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete.
        /// On the
        ///  server side, the connection will not be closed until all incoming invocations have completed.
        /// </summary>

        Gracefully,
        /// <summary>
        /// Wait for all pending invocations to complete before closing the connection.
        /// </summary>

        GracefullyWithWait
    }

    public interface Connection
    {
        /// <summary>
        /// Manually close the connection using the specified closure mode.
        /// </summary>
        /// <param name="mode">Determines how the connection will be closed.
        ///  </param>
        void close(ConnectionClose mode);

        /// <summary>
        /// Create a special proxy that always uses this connection.
        /// This can be used for callbacks from a server to a
        ///  client if the server cannot directly establish a connection to the client, for example because of firewalls. In
        ///  this case, the server would create a proxy using an already established connection from the client.
        /// </summary>
        ///  <param name="id">The identity for which a proxy is to be created.
        ///  </param>
        /// <returns>A proxy that matches the given identity and uses this connection.
        ///  </returns>
        ObjectPrx createProxy(Identity id);

        /// <summary>
        /// Explicitly set an object adapter that dispatches requests that are received over this connection.
        /// A client can
        ///  invoke an operation on a server using a proxy, and then set an object adapter for the outgoing connection that
        ///  is used by the proxy in order to receive callbacks. This is useful if the server cannot establish a connection
        ///  back to the client, for example because of firewalls.
        /// </summary>
        ///  <param name="adapter">The object adapter that should be used by this connection to dispatch requests. The object
        ///  adapter must be activated. When the object adapter is deactivated, it is automatically removed from the
        ///  connection. Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException
        ///  </param>
        void setAdapter(ObjectAdapter adapter);

        /// <summary>
        /// Get the object adapter that dispatches requests for this connection.
        /// </summary>
        /// <returns>The object adapter that dispatches requests for the connection, or null if no adapter is set.
        ///  </returns>
        ObjectAdapter getAdapter();

        /// <summary>
        /// Get the endpoint from which the connection was created.
        /// </summary>
        /// <returns>The endpoint from which the connection was created.</returns>
        Endpoint getEndpoint();

        /// <summary>
        /// Flush any pending batch requests for this connection.
        /// This means all batch requests invoked on fixed proxies
        ///  associated with the connection.
        /// </summary>
        ///  <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent over
        ///  the wire.</param>
        void flushBatchRequests(CompressBatch compress);

        System.Threading.Tasks.Task flushBatchRequestsAsync(
            CompressBatch compress,
            System.IProgress<bool>? progress = null,
            System.Threading.CancellationToken cancel = default);

        /// <summary>
        /// Set a close callback on the connection.
        /// The callback is called by the connection when it's closed. The callback
        ///  is called from the Ice thread pool associated with the connection. If the callback needs more information about
        ///  the closure, it can call Connection.throwException.
        /// </summary>
        ///  <param name="callback">The close callback object.</param>
        void setCloseCallback(CloseCallback callback);

        /// <summary>
        /// Set a heartbeat callback on the connection.
        /// The callback is called by the connection when a heartbeat is
        ///  received. The callback is called from the Ice thread pool associated with the connection.
        /// </summary>
        ///  <param name="callback">The heartbeat callback object.</param>
        void setHeartbeatCallback(HeartbeatCallback callback);

        /// <summary>
        /// Send a heartbeat message.
        /// </summary>
        void heartbeat();

        System.Threading.Tasks.Task heartbeatAsync(
            System.IProgress<bool>? progress = null,
            System.Threading.CancellationToken cancel = default);

        /// <summary>
        /// Return the connection type.
        /// This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
        /// </summary>
        ///  <returns>The type of the connection.</returns>
        string type();

        /// <summary>
        /// Returns the connection information.
        /// </summary>
        /// <returns>The connection information.</returns>
        ConnectionInfo getInfo();

        /// <summary>
        /// Set the connection buffer receive/send size.
        /// </summary>
        /// <param name="rcvSize">The connection receive buffer size.
        ///  </param>
        /// <param name="sndSize">The connection send buffer size.</param>
        void setBufferSize(int rcvSize, int sndSize);

        /// <summary>
        /// Throw an exception indicating the reason for connection closure.
        /// For example,
        ///  CloseConnectionException is raised if the connection was closed gracefully, whereas
        ///  ConnectionManuallyClosedException is raised if the connection was manually closed by
        ///  the application. This operation does nothing if the connection is not yet closed.
        /// </summary>
        void throwException();
    }

    public class IPConnectionInfo : ConnectionInfo
    {
        public string localAddress;
        public int localPort;
        public string remoteAddress;
        public int remotePort;

        public IPConnectionInfo() : base()
        {
            localAddress = "";
            localPort = -1;
            remoteAddress = "";
            remotePort = -1;
        }

        public IPConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId, string localAddress, int localPort, string remoteAddress, int remotePort) : base(underlying, incoming, adapterName, connectionId)
        {
            this.localAddress = localAddress;
            this.localPort = localPort;
            this.remoteAddress = remoteAddress;
            this.remotePort = remotePort;
        }
    }

    public class TCPConnectionInfo : IPConnectionInfo
    {
        public int rcvSize;
        public int sndSize;

        public TCPConnectionInfo() : base()
        {
            rcvSize = 0;
            sndSize = 0;
        }

        public TCPConnectionInfo(
            ConnectionInfo underlying,
            bool incoming,
            string adapterName,
            string connectionId,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            int rcvSize,
            int sndSize)
            : base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            this.rcvSize = rcvSize;
            this.sndSize = sndSize;
        }
    }

    public class UDPConnectionInfo : IPConnectionInfo
    {
        public string mcastAddress;
        public int mcastPort;
        public int rcvSize;
        public int sndSize;

        public UDPConnectionInfo() : base()
        {
            mcastAddress = "";
            mcastPort = -1;
            rcvSize = 0;
            sndSize = 0;
        }

        public UDPConnectionInfo(
            ConnectionInfo underlying,
            bool incoming,
            string adapterName,
            string connectionId,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort,
            string mcastAddress,
            int mcastPort,
            int rcvSize,
            int sndSize)
            : base(underlying, incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
        {
            this.mcastAddress = mcastAddress;
            this.mcastPort = mcastPort;
            this.rcvSize = rcvSize;
            this.sndSize = sndSize;
        }
    }

    public class WSConnectionInfo : ConnectionInfo
    {
        public Dictionary<string, string> headers;

        public WSConnectionInfo() : base()
        {
            headers = [];
        }

        public WSConnectionInfo(
            ConnectionInfo underlying,
            bool incoming,
            string adapterName,
            string connectionId,
            Dictionary<string, string> headers)
            : base(underlying, incoming, adapterName, connectionId)
        {
            this.headers = headers;
        }
    }
}
