// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents batch compression options when flushing queued batch requests.
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
/// An application can use this delegate to receive notifications when a connection closes.
/// </summary>
public delegate void CloseCallback(Connection con);

/// <summary>
/// Represents a connection that uses the Ice protocol.
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
    /// <remarks>If closing the connection takes longer than the configured close timeout, the connection is aborted
    /// with a <see cref="CloseTimeoutException"/>.</remarks>
    Task closeAsync();

    /// <summary>
    /// Creates a special proxy (a "fixed proxy") that always uses this connection.
    /// </summary>
    /// <param name="id">The identity of the target object.</param>
    /// <returns>A fixed proxy with the provided identity.</returns>
    ObjectPrx createProxy(Identity id);

    /// <summary>
    /// Associates an object adapter with this connection. When a connection receives a request, it dispatches this
    /// request using its associated object adapter. If the associated object adapter is null, the connection
    /// rejects any incoming request with an <see cref="ObjectNotExistException" />.
    /// The default object adapter of an incoming connection is the object adapter that created this connection;
    /// the default object adapter of an outgoing connection is the communicator's default object adapter.
    /// </summary>
    /// <param name="adapter">The object adapter to associate with this connection.</param>
    /// <seealso cref="Communicator.getDefaultObjectAdapter"/>
    /// <seealso cref="getAdapter"/>
    void setAdapter(ObjectAdapter? adapter);

    /// <summary>
    /// Gets the object adapter associated with this connection.
    /// </summary>
    /// <returns>The object adapter associated with this connection.</returns>
    /// <seealso cref="setAdapter"/>
    ObjectAdapter? getAdapter();

    /// <summary>
    /// Gets the endpoint from which the connection was created.
    /// </summary>
    /// <returns>The endpoint from which the connection was created.</returns>
    Endpoint getEndpoint();

    /// <summary>
    /// Flushes any pending batch requests for this connection.
    /// This means all batch requests invoked on fixed proxies associated with the connection.
    /// </summary>
    /// <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent
    /// over the wire.</param>
    void flushBatchRequests(CompressBatch compress);

    /// <summary>
    /// Flushes any pending batch requests for this connection.
    /// This means all batch requests invoked on fixed proxies associated with the connection.
    /// </summary>
    /// <param name="compress">Specifies whether or not the queued batch requests should be compressed before being sent
    /// over the wire.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>A task that completes when the flush completes.</returns>
    System.Threading.Tasks.Task flushBatchRequestsAsync(
        CompressBatch compress,
        System.IProgress<bool>? progress = null,
        System.Threading.CancellationToken cancel = default);

    /// <summary>
    /// Sets a close callback on the connection. The callback is called by the connection when it's closed.
    /// The callback is called from the Ice thread pool associated with the connection.
    /// </summary>
    /// <param name="callback">The close callback object.</param>
    void setCloseCallback(CloseCallback callback);

    /// <summary>
    /// Disables the inactivity check on this connection.
    /// </summary>
    void disableInactivityCheck();

    /// <summary>
    /// Returns the connection type. This corresponds to the endpoint type, such as "tcp", "udp", etc.
    /// </summary>
    /// <returns>The type of the connection.</returns>
    string type();

    /// <summary>
    /// Returns the connection information.
    /// </summary>
    /// <returns>The connection information.</returns>
    ConnectionInfo getInfo();

    /// <summary>
    /// Sets the size of the receive and send buffers.
    /// </summary>
    /// <param name="rcvSize">The size of the receive buffer.</param>
    /// <param name="sndSize">The size of the send buffer.</param>
    void setBufferSize(int rcvSize, int sndSize);

    /// <summary>
    /// Throws an exception that provides the reason for the closure of this connection. For example,
    /// this method throws <see cref="CloseConnectionException"/> when the connection was closed gracefully by the peer;
    /// It throws <see cref="ConnectionAbortedException"/> or <see cref="ConnectionClosedException"/>
    /// when the connection is aborted. This method does nothing if the connection is not yet closed.
    /// </summary>
    void throwException();
}

/// <summary>
/// Base class for all connection info classes.
/// </summary>
public class ConnectionInfo
{
    /// <summary>
    /// The information of the underlying transport or null if there's no underlying transport.
    /// </summary>
    public readonly ConnectionInfo? underlying;

    /// <summary>
    /// <see langword="true"/> if this an incoming connection, <see langword="false"/> otherwise.
    /// </summary>
    public readonly bool incoming;

    /// <summary>
    /// The name of the adapter associated with the connection.
    /// </summary>
    public readonly string adapterName;

    /// <summary>
    /// The connection ID.
    /// </summary>
    public readonly string connectionId;

    protected ConnectionInfo(ConnectionInfo underlying)
    {
        this.underlying = underlying;
        incoming = underlying.incoming;
        adapterName = underlying.adapterName;
        connectionId = underlying.connectionId;
    }

    protected ConnectionInfo(bool incoming, string adapterName, string connectionId)
    {
        this.incoming = incoming;
        this.adapterName = adapterName;
        this.connectionId = connectionId;
    }
}

/// <summary>
/// Provides access to the connection details of an IP connection.
/// </summary>
public class IPConnectionInfo : ConnectionInfo
{
    /// <summary>
    /// The local address.
    /// </summary>
    public readonly string localAddress;

    /// <summary>
    /// The local port.
    /// </summary>
    public readonly int localPort;

    /// <summary>
    /// The remote address.
    /// </summary>
    public readonly string remoteAddress;

    /// <summary>
    /// The remote port.
    /// </summary>
    public readonly int remotePort;

    protected IPConnectionInfo(
        bool incoming,
        string adapterName,
        string connectionId,
        string localAddress,
        int localPort,
        string remoteAddress,
        int remotePort)
        : base(incoming, adapterName, connectionId)
    {
        this.localAddress = localAddress;
        this.localPort = localPort;
        this.remoteAddress = remoteAddress;
        this.remotePort = remotePort;
    }
}

/// <summary>
/// Provides access to the connection details of a TCP connection.
/// </summary>
public sealed class TCPConnectionInfo : IPConnectionInfo
{
    /// <summary>
    /// The size of the receive buffer.
    /// </summary>
    public readonly int rcvSize;

    /// <summary>
    /// The size of the send buffer.
    /// </summary>
    public readonly int sndSize;

    internal TCPConnectionInfo(
        bool incoming,
        string adapterName,
        string connectionId,
        string localAddress,
        int localPort,
        string remoteAddress,
        int remotePort,
        int rcvSize,
        int sndSize)
        : base(incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
    {
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    internal TCPConnectionInfo(bool incoming, string adapterName, string connectionId)
        : this(
            incoming,
            adapterName,
            connectionId,
            localAddress: "",
            localPort: -1,
            remoteAddress: "",
            remotePort: -1,
            rcvSize: 0,
            sndSize: 0)
    {
    }
}

/// <summary>
/// Provides access to the connection details of a UDP connection.
/// </summary>
public sealed class UDPConnectionInfo : IPConnectionInfo
{
    /// <summary>
    /// The multicast address.
    /// </summary>
    public readonly string mcastAddress;

    /// <summary>
    /// The multicast port.
    /// </summary>
    public readonly int mcastPort;

    /// <summary>
    /// The size of the receive buffer.
    /// </summary>
    public readonly int rcvSize;

    /// <summary>
    /// The size of the send buffer.
    /// </summary>
    public readonly int sndSize;

    internal UDPConnectionInfo(
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
        : base(incoming, adapterName, connectionId, localAddress, localPort, remoteAddress, remotePort)
    {
        this.mcastAddress = mcastAddress;
        this.mcastPort = mcastPort;
        this.rcvSize = rcvSize;
        this.sndSize = sndSize;
    }

    internal UDPConnectionInfo(bool incoming, string adapterName, string connectionId)
        : this(
            incoming,
            adapterName,
            connectionId,
            localAddress: "",
            localPort: -1,
            remoteAddress: "",
            remotePort: -1,
            mcastAddress: "",
            mcastPort: -1,
            rcvSize: 0,
            sndSize: 0)
    {
    }
}

/// <summary>
/// Provides access to the connection details of a WebSocket connection.
/// </summary>
public sealed class WSConnectionInfo : ConnectionInfo
{
    /// <summary>
    /// The headers from the HTTP upgrade request.
    /// </summary>
    public readonly Dictionary<string, string> headers;

    internal WSConnectionInfo(ConnectionInfo underlying, Dictionary<string, string> headers)
        : base(underlying) => this.headers = headers;
}
