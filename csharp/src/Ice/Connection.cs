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

    /// <summary>
    ///  Base class providing access to the connection details.
    /// </summary>
    public class ConnectionInfo
    {
        /// <summary>
        /// The underlying connection information.
        /// </summary>
        public ConnectionInfo? underlying;

        /// <summary>
        /// Whether the connection is an incoming connection (<c>true</c>) or an outgoing connection (<c>false</c>).
        /// </summary>
        public bool incoming;

        /// <summary>
        /// The name of the adapter associated with the connection.
        /// </summary>
        public string adapterName;

        /// <summary>
        /// The connection id.
        /// </summary>
        public string connectionId;

        /// <summary>
        /// Initializes a new instance of the <see cref="ConnectionInfo" /> class.
        /// </summary>
        public ConnectionInfo()
        {
            adapterName = "";
            connectionId = "";
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ConnectionInfo" /> class.
        /// </summary>
        /// <param name="underlying">The underlying connection information.</param>
        /// <param name="incoming">Whether the connection is an incoming connection (<c>true</c>) or an outgoing
        /// connection (<c>false</c>).</param>
        /// <param name="adapterName">The name of the adapter associated with the connection.</param>
        /// <param name="connectionId">The connection id.</param>
        public ConnectionInfo(ConnectionInfo underlying, bool incoming, string adapterName, string connectionId)
        {
            this.underlying = underlying;
            this.incoming = incoming;
            this.adapterName = adapterName;
            this.connectionId = connectionId;
        }
    }

    public delegate void CloseCallback(Connection con);

    /// <summary>
    /// The user-level interface to a connection.
    /// </summary>
    public interface Connection
    {
        /// <summary>
        /// Aborts this connection.
        /// </summary>
        void abort();

        /// <summary>
        /// Closes the connection gracefully after waiting for all outstanding invocations to complete.
        /// </summary>
        /// <returns>A task that completes when the connection is closed.</returns>
        /// <remarks>If this operation takes longer than the configured close timeout, the connection is aborted with a
        /// <see cref="CloseTimeoutException"/>.</remarks>
        Task closeAsync();

        /// <summary>
        /// Create a special proxy that always uses this connection.
        /// This can be used for callbacks from a server to a
        /// client if the server cannot directly establish a connection to the client, for example because of firewalls. In
        /// this case, the server would create a proxy using an already established connection from the client.
        /// </summary>
        /// <param name="id">The identity for which a proxy is to be created.</param>
        /// <returns>A proxy that matches the given identity and uses this connection.</returns>
        ObjectPrx createProxy(Identity id);

        /// <summary>
        /// Associates an object adapter with this connection. When a connection receives a request, it dispatches this
        /// request using its associated object adapter. If the associated object adapter is null, the connection
        /// rejects any incoming request with an <see cref="ObjectNotExistException" />.
        /// The default object adapter of an incoming connection is the object adapter that created this connection;
        /// the default object adapter of an outgoing connection is null.
        /// </summary>
        /// <param name="adapter">The object adapter to associate with this connection.</param>
        void setAdapter(ObjectAdapter? adapter);

        /// <summary>
        /// Gets the object adapter associated with this connection.
        /// </summary>
        /// <returns>The object adapter associated with this connection.</returns>
        ObjectAdapter? getAdapter();

        /// <summary>
        /// Get the endpoint from which the connection was created.
        /// </summary>
        /// <returns>The endpoint from which the connection was created.</returns>
        Endpoint getEndpoint();

        /// <summary>
        /// Flush any pending batch requests for this connection.
        /// This means all batch requests invoked on fixed proxies associated with the connection.
        /// </summary>
        /// <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent over
        /// the wire.</param>
        void flushBatchRequests(CompressBatch compress);

        System.Threading.Tasks.Task flushBatchRequestsAsync(
            CompressBatch compress,
            System.IProgress<bool>? progress = null,
            System.Threading.CancellationToken cancel = default);

        /// <summary>
        /// Set a close callback on the connection.
        /// The callback is called by the connection when it's closed. The callback
        /// is called from the Ice thread pool associated with the connection. If the callback needs more information about
        /// the closure, it can call Connection.throwException.
        /// </summary>
        /// <param name="callback">The close callback object.</param>
        void setCloseCallback(CloseCallback callback);

        /// <summary>
        /// Return the connection type.
        /// This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
        /// </summary>
        /// <returns>The type of the connection.</returns>
        string type();

        /// <summary>
        /// Returns the connection information.
        /// </summary>
        /// <returns>The connection information.</returns>
        ConnectionInfo getInfo();

        /// <summary>
        /// Set the connection buffer receive/send size.
        /// </summary>
        /// <param name="rcvSize">The connection receive buffer size.</param>
        /// <param name="sndSize">The connection send buffer size.</param>
        void setBufferSize(int rcvSize, int sndSize);

        /// <summary>
        /// Throw an exception indicating the reason for connection closure.
        /// For example,
        /// CloseConnectionException is raised if the connection was closed gracefully, whereas
        /// ConnectionAbortedException/ConnectionClosedException is raised if the connection was manually closed by
        /// the application. This operation does nothing if the connection is not yet closed.
        /// </summary>
        void throwException();
    }

    public class IPConnectionInfo : ConnectionInfo
    {
        public string localAddress;
        public int localPort;
        public string remoteAddress;
        public int remotePort;

        public IPConnectionInfo()
        {
            localAddress = "";
            localPort = -1;
            remoteAddress = "";
            remotePort = -1;
        }

        public IPConnectionInfo(
            ConnectionInfo underlying,
            bool incoming,
            string adapterName,
            string connectionId,
            string localAddress,
            int localPort,
            string remoteAddress,
            int remotePort)
            : base(underlying, incoming, adapterName, connectionId)
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

        public TCPConnectionInfo()
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

        public UDPConnectionInfo()
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

        public WSConnectionInfo()
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
