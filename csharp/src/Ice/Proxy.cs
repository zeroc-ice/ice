// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;

namespace Ice;

/// <summary>
/// Base interface of all object proxies.
/// </summary>
public interface ObjectPrx : IEquatable<ObjectPrx>
{
    /// <summary>
    /// Gets the communicator that created this proxy.
    /// </summary>
    /// <returns>The communicator that created this proxy.</returns>
    Communicator ice_getCommunicator();

    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    /// <param name="id">The type ID of the Slice interface to test against.</param>
    /// <param name="context">The request context.</param>
    /// <returns>True if the target object has the interface specified by id or derives
    /// from the interface specified by id.</returns>
    bool ice_isA(string id, Dictionary<string, string>? context = null);

    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    /// <param name="id">The type ID of the Slice interface to test against.</param>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task<bool> ice_isAAsync(
        string id,
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Tests whether the target object of this proxy can be reached.
    /// </summary>
    /// <param name="context">The request context.</param>
    void ice_ping(Dictionary<string, string>? context = null);

    /// <summary>
    /// Tests whether the target object of this proxy can be reached.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task ice_pingAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <returns>The Slice type IDs of the interfaces supported by the target object, in alphabetical order.
    /// </returns>
    string[] ice_ids(Dictionary<string, string>? context = null);

    /// <summary>
    /// Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task<string[]> ice_idsAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Gets the Slice type ID of the most-derived interface supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <returns>The Slice type ID of the most-derived interface.</returns>
    string ice_id(Dictionary<string, string>? context = null);

    /// <summary>
    /// Gets the Slice type ID of the most-derived interface supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task<string> ice_idAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Invokes an operation dynamically.
    /// </summary>
    /// <param name="operation">The name of the operation to invoke.</param>
    /// <param name="mode">The operation mode (normal or idempotent).</param>
    /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
    /// <param name="outEncaps">The encoded out-parameters and return value
    /// for the operation. The return value follows any out-parameters.</param>
    /// <param name="context">The request context.</param>
    /// <returns>If the operation completed successfully, the return value
    /// is true. If the operation raises a user exception,
    /// the return value is false; in this case, outEncaps
    /// contains the encoded user exception. If the operation raises a run-time exception,
    /// it throws it directly.</returns>
    bool ice_invoke(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        out byte[] outEncaps,
        Dictionary<string, string>? context = null);

    /// <summary>
    /// Invokes an operation dynamically.
    /// </summary>
    /// <param name="operation">The name of the operation to invoke.</param>
    /// <param name="mode">The operation mode (normal or idempotent).</param>
    /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task<Object_Ice_invokeResult>
    ice_invokeAsync(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Gets the identity embedded in this proxy.
    /// <returns>The identity of the target object.</returns>
    /// </summary>
    Identity ice_getIdentity();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
    /// <param name="newIdentity">The identity for the new proxy.</param>
    /// <returns>The proxy with the new identity.</returns>
    /// </summary>
    ObjectPrx ice_identity(Identity newIdentity);

    /// <summary>
    /// Gets the per-proxy context for this proxy.
    /// </summary>
    /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
    /// is null.</returns>
    Dictionary<string, string> ice_getContext();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
    /// </summary>
    /// <param name="newContext">The context for the new proxy.</param>
    /// <returns>The proxy with the new per-proxy context.</returns>
    ObjectPrx ice_context(Dictionary<string, string> newContext);

    /// <summary>
    /// Gets the facet for this proxy.
    /// </summary>
    /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
    /// empty string.</returns>
    string ice_getFacet();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the facet.
    /// </summary>
    /// <param name="newFacet">The facet for the new proxy.</param>
    /// <returns>The proxy with the new facet.</returns>
    ObjectPrx ice_facet(string newFacet);

    /// <summary>
    /// Gets the adapter ID for this proxy.
    /// </summary>
    /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
    /// empty string.</returns>
    string ice_getAdapterId();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the adapter ID.
    /// </summary>
    /// <param name="newAdapterId">The adapter ID for the new proxy.</param>
    /// <returns>The proxy with the new adapter ID.</returns>
    ObjectPrx ice_adapterId(string newAdapterId);

    /// <summary>
    /// Gets the endpoints used by this proxy.
    /// </summary>
    /// <returns>The endpoints used by this proxy.</returns>
    Endpoint[] ice_getEndpoints();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the endpoints.
    /// </summary>
    /// <param name="newEndpoints">The endpoints for the new proxy.</param>
    /// <returns>The proxy with the new endpoints.</returns>
    ObjectPrx ice_endpoints(Endpoint[] newEndpoints);

    /// <summary>
    /// Gets the locator cache timeout of this proxy.
    /// </summary>
    /// <returns>The locator cache timeout value.</returns>
    TimeSpan ice_getLocatorCacheTimeout();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    /// </summary>
    /// <param name="newTimeout">The new locator cache timeout (in seconds).</param>
    ObjectPrx ice_locatorCacheTimeout(int newTimeout);

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    /// </summary>
    /// <param name="newTimeout">The new locator cache timeout.</param>
    ObjectPrx ice_locatorCacheTimeout(TimeSpan newTimeout);

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    /// </summary>
    /// <param name="newTimeout">The new invocation timeout (in milliseconds).</param>
    ObjectPrx ice_invocationTimeout(int newTimeout);

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    /// </summary>
    /// <param name="newTimeout">The new invocation timeout.</param>
    ObjectPrx ice_invocationTimeout(TimeSpan newTimeout);

    /// <summary>
    /// Gets the invocation timeout of this proxy.
    /// </summary>
    /// <returns>The invocation timeout value.</returns>
    TimeSpan ice_getInvocationTimeout();

    /// <summary>
    /// Returns whether this proxy caches connections.
    /// </summary>
    /// <returns>True if this proxy caches connections; false, otherwise.</returns>
    bool ice_isConnectionCached();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for connection caching.
    /// </summary>
    /// <param name="newCache">True if the new proxy should cache connections; false, otherwise.</param>
    /// <returns>The new proxy with the specified caching policy.</returns>
    ObjectPrx ice_connectionCached(bool newCache);

    /// <summary>
    /// Returns how this proxy selects endpoints (randomly or ordered).
    /// </summary>
    /// <returns>The endpoint selection policy.</returns>
    EndpointSelectionType ice_getEndpointSelection();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
    /// </summary>
    /// <param name="newType">The new endpoint selection policy.</param>
    /// <returns>The new proxy with the specified endpoint selection policy.</returns>
    ObjectPrx ice_endpointSelection(EndpointSelectionType newType);

    /// <summary>
    /// Returns whether this proxy communicates only via secure endpoints.
    /// </summary>
    /// <returns>True if this proxy communicates only via secure endpoints; false, otherwise.</returns>
    bool ice_isSecure();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
    /// </summary>
    /// <param name="secure"> If secure is true, only endpoints that use a secure transport are
    /// used by the new proxy. If secure is false, the returned proxy uses both secure and insecure
    /// endpoints.</param>
    /// <returns>The new proxy with the specified selection policy.</returns>
    ObjectPrx ice_secure(bool secure);

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
    /// parameters.
    /// </summary>
    /// <param name="encodingVersion">The encoding version to use to marshal requests parameters.</param>
    /// <returns>The new proxy with the specified encoding version.</returns>
    ObjectPrx ice_encodingVersion(EncodingVersion encodingVersion);

    /// <summary>Gets the encoding version used to marshal requests parameters.</summary>
    /// <returns>The encoding version.</returns>
    EncodingVersion ice_getEncodingVersion();

    /// <summary>
    /// Returns whether this proxy prefers secure endpoints.
    /// </summary>
    /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
    /// attempts to use insecure endpoints; false, otherwise.</returns>
    bool ice_isPreferSecure();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
    /// </summary>
    /// <param name="preferSecure">If preferSecure is true, the new proxy will use secure endpoints for invocations
    /// and only use insecure endpoints if an invocation cannot be made via secure endpoints. If preferSecure is
    /// false, the proxy prefers insecure endpoints to secure ones.</param>
    /// <returns>The new proxy with the new endpoint selection policy.</returns>
    ObjectPrx ice_preferSecure(bool preferSecure);

    /// <summary>
    /// Gets the router for this proxy.
    /// </summary>
    /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
    /// is null.</returns>
    RouterPrx? ice_getRouter();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the router.
    /// </summary>
    /// <param name="router">The router for the new proxy.</param>
    /// <returns>The new proxy with the specified router.</returns>
    ObjectPrx ice_router(RouterPrx? router);

    /// <summary>
    /// Gets the locator for this proxy.
    /// </summary>
    /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
    LocatorPrx? ice_getLocator();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator.
    /// </summary>
    /// <param name="locator">The locator for the new proxy.</param>
    /// <returns>The new proxy with the specified locator.</returns>
    ObjectPrx ice_locator(LocatorPrx? locator);

    /// <summary>
    /// Returns whether this proxy uses collocation optimization.
    /// </summary>
    /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
    bool ice_isCollocationOptimized();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for collocation optimization.
    /// </summary>
    /// <param name="collocated">True if the new proxy enables collocation optimization; false, otherwise.</param>
    /// <returns>The new proxy the specified collocation optimization.</returns>
    ObjectPrx ice_collocationOptimized(bool collocated);

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses twoway invocations.
    /// </summary>
    /// <returns>A new proxy that uses twoway invocations.</returns>
    ObjectPrx ice_twoway();

    /// <summary>
    /// Returns whether this proxy uses twoway invocations.
    /// </summary>
    /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
    bool ice_isTwoway();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
    /// </summary>
    /// <returns>A new proxy that uses oneway invocations.</returns>
    ObjectPrx ice_oneway();

    /// <summary>
    /// Returns whether this proxy uses oneway invocations.
    /// </summary>
    /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
    bool ice_isOneway();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
    /// </summary>
    /// <returns>A new proxy that uses batch oneway invocations.</returns>
    ObjectPrx ice_batchOneway();

    /// <summary>
    /// Returns whether this proxy uses batch oneway invocations.
    /// </summary>
    /// <returns>True if this proxy uses batch oneway invocations; false, otherwise.</returns>
    bool ice_isBatchOneway();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
    /// </summary>
    /// <returns>A new proxy that uses datagram invocations.</returns>
    ObjectPrx ice_datagram();

    /// <summary>
    /// Returns whether this proxy uses datagram invocations.
    /// </summary>
    /// <returns>True if this proxy uses datagram invocations; false, otherwise.</returns>
    bool ice_isDatagram();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
    /// </summary>
    /// <returns>A new proxy that uses batch datagram invocations.</returns>
    ObjectPrx ice_batchDatagram();

    /// <summary>
    /// Returns whether this proxy uses batch datagram invocations.
    /// </summary>
    /// <returns>True if this proxy uses batch datagram invocations; false, otherwise.</returns>
    bool ice_isBatchDatagram();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for compression.
    /// </summary>
    /// <param name="compress">True enables compression for the new proxy; false disables compression.</param>
    /// <returns>A new proxy with the specified compression setting.</returns>
    ObjectPrx ice_compress(bool compress);

    /// <summary>
    /// Obtains the compression override setting of this proxy.
    /// </summary>
    /// <returns>The compression override setting. If no optional value is present, no override is
    /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
    bool? ice_getCompress();

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for its connection ID.
    /// </summary>
    /// <param name="connectionId">The connection ID for the new proxy. An empty string removes the
    /// connection ID.</param>
    /// <returns>A new proxy with the specified connection ID.</returns>
    ObjectPrx ice_connectionId(string connectionId);

    /// <summary>
    /// Gets the connection id of this proxy.
    /// </summary>
    /// <returns>The connection id.</returns>
    string ice_getConnectionId();

    /// <summary>
    /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
    /// the given connection.
    /// </summary>
    /// <param name="connection">The fixed proxy connection.</param>
    /// <returns>A fixed proxy bound to the given connection.</returns>
    ObjectPrx ice_fixed(Ice.Connection connection);

    /// <summary>
    /// Returns whether this proxy is a fixed proxy.
    /// </summary>
    /// <returns>True if this is a fixed proxy, false otherwise.
    /// </returns>
    bool ice_isFixed();

    /// <summary>
    /// Gets the connection to the server that hosts the target object. This method establishes the connection to the
    /// server if it is not already established.
    /// </summary>
    /// <returns>The connection to the server that hosts the target object, or null when this proxy uses collocation
    /// optimization to communicate with the target object.</returns>
    Connection? ice_getConnection();

    /// <summary>
    /// Gets the connection to the server that hosts the target object. This method establishes the connection to the
    /// server if it is not already established.
    /// </summary>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The connection to the server that hosts the target object, or null when this proxy uses collocation
    /// optimization to communicate with the target object.</returns>
    Task<Connection?> ice_getConnectionAsync(IProgress<bool>? progress = null, CancellationToken cancel = default);

    /// <summary>
    /// Gets the cached Connection for this proxy. If the proxy does not yet have an established
    /// connection, it does not attempt to create a connection.
    /// </summary>
    /// <returns>The cached Connection for this proxy (null if the proxy does not have
    /// an established connection).</returns>
    Connection? ice_getCachedConnection();

    /// <summary>
    /// Flushes any pending batched requests for this proxy. The call blocks until the flush is complete.
    /// </summary>
    void ice_flushBatchRequests();

    /// <summary>
    /// Asynchronously flushes any pending batched requests for this proxy.
    /// </summary>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    Task ice_flushBatchRequestsAsync(
        IProgress<bool>? progress = null,
        CancellationToken cancel = default);

    /// <summary>
    /// Write a proxy to the output stream.
    /// </summary>
    /// <param name="os">Output stream object to write the proxy.</param>
    [EditorBrowsable(EditorBrowsableState.Never)]
    void iceWrite(OutputStream os);

    /// <summary>
    /// Returns a scheduler object that uses the Ice thread pool.
    /// </summary>
    /// <returns>The task scheduler object.</returns>
    TaskScheduler ice_scheduler();
}

/// <summary>
/// Represent the result of the ice_invokeAsync operation.
/// </summary>
public record struct Object_Ice_invokeResult(bool returnValue, byte[] outEncaps);

/// <summary>
/// Base class of all object proxies.
/// </summary>
public abstract class ObjectPrxHelperBase : ObjectPrx
{
    public static bool operator ==(ObjectPrxHelperBase? lhs, ObjectPrxHelperBase? rhs) =>
        lhs is not null ? lhs.Equals(rhs) : rhs is null;

    public static bool operator !=(ObjectPrxHelperBase? lhs, ObjectPrxHelperBase? rhs) => !(lhs == rhs);

    /// <summary>
    /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
    /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
    /// </summary>
    /// <param name="other">The proxy to compare this proxy with.</param>
    /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
    public bool Equals(ObjectPrx? other) =>
        other is not null && _reference == ((ObjectPrxHelperBase)other)._reference;

    public override bool Equals(object? obj) => Equals(obj as ObjectPrx);

    /// <summary>
    /// Returns a hash code for this proxy.
    /// </summary>
    /// <returns>The hash code.</returns>
    public override int GetHashCode()
    {
        return _reference.GetHashCode();
    }

    /// <summary>
    /// Gets the communicator that created this proxy.
    /// </summary>
    /// <returns>The communicator that created this proxy.</returns>
    public Communicator ice_getCommunicator()
    {
        return _reference.getCommunicator();
    }

    /// <summary>
    /// Gets the stringified form of this proxy.
    /// </summary>
    /// <returns>The stringified proxy.</returns>
    public override string ToString()
    {
        return _reference.ToString();
    }

    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    /// <param name="id">The type ID of the Slice interface to test against.</param>
    /// <param name="context">The request context.</param>
    /// <returns>True if the target object has the interface specified by id or derives
    /// from the interface specified by id.</returns>
    public bool ice_isA(string id, Dictionary<string, string>? context = null)
    {
        try
        {
            return iceI_ice_isAAsync(id, context, synchronous: true, progress: null, CancellationToken.None).Result;
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    /// <param name="id">The type ID of the Slice interface to test against.</param>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    public Task<bool> ice_isAAsync(
        string id,
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        return iceI_ice_isAAsync(id, context, synchronous: false, progress, cancel);
    }

    private Task<bool>
    iceI_ice_isAAsync(
        string id,
        Dictionary<string, string>? context,
        bool synchronous,
        IProgress<bool>? progress,
        CancellationToken cancel)
    {
        iceCheckTwowayOnly(_ice_isA_name);
        var completed = new OperationTaskCompletionCallback<bool>(progress, cancel);
        iceI_ice_isA(id, context, completed, synchronous);
        return completed.Task;
    }

    private const string _ice_isA_name = "ice_isA";

    private void iceI_ice_isA(
        string id,
        Dictionary<string, string>? context,
        OutgoingAsyncCompletionCallback completed,
        bool synchronous)
    {
        iceCheckAsyncTwowayOnly(_ice_isA_name);
        getOutgoingAsync<bool>(completed).invoke(
            _ice_isA_name,
            OperationMode.Idempotent,
            FormatType.CompactFormat,
            context,
            synchronous,
            (OutputStream os) => { os.writeString(id); },
            null,
            (InputStream iss) => { return iss.readBool(); });
    }

    /// <summary>
    /// Tests whether the target object of this proxy can be reached.
    /// </summary>
    /// <param name="context">The request context.</param>
    public void ice_ping(Dictionary<string, string>? context = null)
    {
        try
        {
            iceI_ice_pingAsync(context, synchronous: true, progress: null, CancellationToken.None).Wait();
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    /// <summary>
    /// Tests whether the target object of this proxy can be reached.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    public Task ice_pingAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        return iceI_ice_pingAsync(context, synchronous: false, progress, cancel);
    }

    private Task
    iceI_ice_pingAsync(
        Dictionary<string, string>? context,
        bool synchronous,
        IProgress<bool>? progress,
        CancellationToken cancel)
    {
        var completed = new OperationTaskCompletionCallback<object>(progress, cancel);
        iceI_ice_ping(context, completed, synchronous);
        return completed.Task;
    }

    private const string _ice_ping_name = "ice_ping";

    private void iceI_ice_ping(
        Dictionary<string, string>? context,
        OutgoingAsyncCompletionCallback completed,
        bool synchronous)
    {
        getOutgoingAsync<object>(completed).invoke(
            _ice_ping_name,
            OperationMode.Idempotent,
            FormatType.CompactFormat,
            context,
            synchronous);
    }

    /// <summary>
    /// Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <returns>The Slice type IDs of the interfaces supported by the target object, in alphabetical order.
    /// </returns>
    public string[] ice_ids(Dictionary<string, string>? context = null)
    {
        try
        {
            return iceI_ice_idsAsync(context, synchronous: true, progress: null, CancellationToken.None).Result;
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    /// <summary>
    /// Gets the Slice type IDs of the interfaces supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    public Task<string[]>
    ice_idsAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        return iceI_ice_idsAsync(context, synchronous: false, progress, cancel);
    }

    private Task<string[]> iceI_ice_idsAsync(
        Dictionary<string, string>? context,
        bool synchronous,
        IProgress<bool>? progress,
        CancellationToken cancel)
    {
        iceCheckTwowayOnly(_ice_ids_name);
        var completed = new OperationTaskCompletionCallback<string[]>(progress, cancel);
        iceI_ice_ids(context, completed, synchronous);
        return completed.Task;
    }

    private const string _ice_ids_name = "ice_ids";

    private void iceI_ice_ids(
        Dictionary<string, string>? context,
        OutgoingAsyncCompletionCallback completed,
        bool synchronous)
    {
        iceCheckAsyncTwowayOnly(_ice_ids_name);
        getOutgoingAsync<string[]>(completed).invoke(
            _ice_ids_name,
            OperationMode.Idempotent,
            FormatType.CompactFormat,
            context,
            synchronous,
            read: (InputStream iss) => { return iss.readStringSeq(); });
    }

    /// <summary>
    /// Gets the Slice type ID of the most-derived interface supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <returns>The Slice type ID of the most-derived interface.</returns>
    public string ice_id(Dictionary<string, string>? context = null)
    {
        try
        {
            return iceI_ice_idAsync(context, synchronous: true, progress: null, CancellationToken.None).Result;
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    /// <summary>
    /// Gets the Slice type ID of the most-derived interface supported by the target object of this proxy.
    /// </summary>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    public Task<string> ice_idAsync(
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        return iceI_ice_idAsync(context, synchronous: false, progress, cancel);
    }

    private Task<string>
    iceI_ice_idAsync(
        Dictionary<string, string>? context,
        bool synchronous,
        IProgress<bool>? progress,
        CancellationToken cancel)
    {
        iceCheckTwowayOnly(_ice_id_name);
        var completed = new OperationTaskCompletionCallback<string>(progress, cancel);
        iceI_ice_id(context, completed, synchronous);
        return completed.Task;
    }

    private const string _ice_id_name = "ice_id";

    private void iceI_ice_id(
        Dictionary<string, string>? context,
        OutgoingAsyncCompletionCallback completed,
        bool synchronous)
    {
        getOutgoingAsync<string>(completed).invoke(
            _ice_id_name,
            OperationMode.Idempotent,
            FormatType.CompactFormat,
            context,
            synchronous,
            read: (InputStream iss) => { return iss.readString(); });
    }

    /// <summary>
    /// Invokes an operation dynamically.
    /// </summary>
    /// <param name="operation">The name of the operation to invoke.</param>
    /// <param name="mode">The operation mode (normal or idempotent).</param>
    /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
    /// <param name="outEncaps">The encoded out-parameters and return value
    /// for the operation. The return value follows any out-parameters.</param>
    /// <param name="context">The request context.</param>
    /// <returns>If the operation completed successfully, the return value
    /// is true. If the operation raises a user exception,
    /// the return value is false; in this case, outEncaps
    /// contains the encoded user exception. If the operation raises a run-time exception,
    /// it throws it directly.</returns>
    public bool ice_invoke(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        out byte[] outEncaps,
        Dictionary<string, string>? context = null)
    {
        try
        {
            var result = iceI_ice_invokeAsync(
                operation,
                mode,
                inEncaps,
                context,
                synchronous: true,
                progress: null,
                CancellationToken.None).Result;
            outEncaps = result.outEncaps;
            return result.returnValue;
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    /// <summary>
    /// Invokes an operation dynamically.
    /// </summary>
    /// <param name="operation">The name of the operation to invoke.</param>
    /// <param name="mode">The operation mode (normal or idempotent).</param>
    /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
    /// <param name="context">The request context.</param>
    /// <param name="progress">Sent progress provider.</param>
    /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
    /// <returns>The task object representing the asynchronous operation.</returns>
    public Task<Object_Ice_invokeResult>
    ice_invokeAsync(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        Dictionary<string, string>? context = null,
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        return iceI_ice_invokeAsync(operation, mode, inEncaps, context, synchronous: false, progress, cancel);
    }

    private Task<Object_Ice_invokeResult>
    iceI_ice_invokeAsync(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        Dictionary<string, string>? context,
        bool synchronous,
        IProgress<bool>? progress,
        CancellationToken cancel)
    {
        var completed = new InvokeTaskCompletionCallback(progress, cancel);
        iceI_ice_invoke(operation, mode, inEncaps, context, completed, synchronous);
        return completed.Task;
    }

    private void iceI_ice_invoke(
        string operation,
        OperationMode mode,
        byte[] inEncaps,
        Dictionary<string, string>? context,
        OutgoingAsyncCompletionCallback completed,
        bool synchronous)
    {
        getInvokeOutgoingAsync(completed).invoke(operation, mode, inEncaps, context, synchronous);
    }

    /// <summary>
    /// Gets the identity embedded in this proxy.
    /// </summary>
    /// <returns>The identity of the target object.</returns>
    public Identity ice_getIdentity() => _reference.getIdentity() with { };

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the proxy identity.
    /// </summary>
    /// <param name="newIdentity">The identity for the new proxy.</param>
    /// <returns>The new proxy with the specified identity.</returns>
    /// <exception cref="ArgumentException">If the name of the new identity is empty.</exception>
    public ObjectPrx ice_identity(Identity newIdentity)
    {
        if (newIdentity.name.Length == 0)
        {
            throw new ArgumentException("The name of an Ice object identity cannot be empty.", nameof(newIdentity));
        }
        if (newIdentity == _reference.getIdentity())
        {
            return this;
        }
        else
        {
            return new ObjectPrxHelper(_reference.changeIdentity(newIdentity));
        }
    }

    /// <summary>
    /// Gets the per-proxy context for this proxy.
    /// </summary>
    /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
    /// is null.</returns>
    public Dictionary<string, string> ice_getContext()
    {
        return new Dictionary<string, string>(_reference.getContext());
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the per-proxy context.
    /// </summary>
    /// <param name="newContext">The context for the new proxy.</param>
    /// <returns>The proxy with the new per-proxy context.</returns>
    public ObjectPrx ice_context(Dictionary<string, string> newContext)
    {
        return iceNewInstance(_reference.changeContext(newContext));
    }

    /// <summary>
    /// Gets the facet for this proxy.
    /// </summary>
    /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
    /// empty string.</returns>
    public string ice_getFacet()
    {
        return _reference.getFacet();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the facet.
    /// </summary>
    /// <param name="newFacet">The facet for the new proxy.</param>
    /// <returns>The proxy with the new facet.</returns>
    public ObjectPrx ice_facet(string newFacet)
    {
        newFacet ??= "";

        if (newFacet == _reference.getFacet())
        {
            return this;
        }
        else
        {
            return new ObjectPrxHelper(_reference.changeFacet(newFacet));
        }
    }

    /// <summary>
    /// Gets the adapter ID for this proxy.
    /// </summary>
    /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
    /// empty string.</returns>
    public string ice_getAdapterId()
    {
        return _reference.getAdapterId();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the adapter ID.
    /// </summary>
    /// <param name="newAdapterId">The adapter ID for the new proxy.</param>
    /// <returns>The new proxy with the specified adapter ID.</returns>
    public ObjectPrx ice_adapterId(string newAdapterId)
    {
        newAdapterId ??= "";

        if (newAdapterId == _reference.getAdapterId())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeAdapterId(newAdapterId));
        }
    }

    /// <summary>
    /// Gets the endpoints used by this proxy.
    /// </summary>
    /// <returns>The endpoints used by this proxy.</returns>
    public Endpoint[] ice_getEndpoints()
    {
        return (Endpoint[])_reference.getEndpoints().Clone();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the endpoints.
    /// </summary>
    /// <param name="newEndpoints">The endpoints for the new proxy.</param>
    /// <returns>The new proxy with the specified endpoints.</returns>
    public ObjectPrx ice_endpoints(Endpoint[] newEndpoints)
    {
        newEndpoints ??= [];

        if (_reference.getEndpoints().SequenceEqual(newEndpoints))
        {
            return this;
        }
        else
        {
            var endpoints = new EndpointI[newEndpoints.Length];
            for (int i = 0; i < newEndpoints.Length; ++i)
            {
                endpoints[i] = (EndpointI)newEndpoints[i];
            }
            return iceNewInstance(_reference.changeEndpoints(endpoints));
        }
    }

    /// <summary>
    /// Gets the locator cache timeout of this proxy.
    /// </summary>
    /// <returns>The locator cache timeout value.</returns>
    public TimeSpan ice_getLocatorCacheTimeout()
    {
        return _reference.getLocatorCacheTimeout();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    /// </summary>
    /// <param name="newTimeout">The new locator cache timeout (in seconds).</param>
    /// <returns>The new proxy with the specified locator cache timeout.</returns>
    public ObjectPrx ice_locatorCacheTimeout(int newTimeout)
    {
        return ice_locatorCacheTimeout(TimeSpan.FromSeconds(newTimeout));
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator cache timeout.
    /// </summary>
    /// <param name="newTimeout">The new locator cache timeout.</param>
    /// <returns>The new proxy with the specified locator cache timeout.</returns>
    public ObjectPrx ice_locatorCacheTimeout(TimeSpan newTimeout)
    {
        if (newTimeout == _reference.getLocatorCacheTimeout())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeLocatorCacheTimeout(newTimeout));
        }
    }

    /// <summary>
    /// Gets the invocation timeout of this proxy.
    /// </summary>
    /// <returns>The invocation timeout value.</returns>
    public TimeSpan ice_getInvocationTimeout()
    {
        return _reference.getInvocationTimeout();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    /// </summary>
    /// <param name="newTimeout">The new invocation timeout (in milliseconds).</param>
    /// <returns>The new proxy with the specified invocation timeout.</returns>
    public ObjectPrx ice_invocationTimeout(int newTimeout)
    {
        return ice_invocationTimeout(TimeSpan.FromMilliseconds(newTimeout));
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the invocation timeout.
    /// </summary>
    /// <param name="newTimeout">The new invocation timeout.</param>
    /// <returns>The new proxy with the specified invocation timeout.</returns>
    public ObjectPrx ice_invocationTimeout(TimeSpan newTimeout)
    {
        if (newTimeout == _reference.getInvocationTimeout())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeInvocationTimeout(newTimeout));
        }
    }

    /// <summary>
    /// Returns whether this proxy caches connections.
    /// </summary>
    /// <returns>True if this proxy caches connections; false, otherwise.</returns>
    public bool ice_isConnectionCached()
    {
        return _reference.getCacheConnection();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for connection caching.
    /// </summary>
    /// <param name="newCache">True if the new proxy should cache connections; false, otherwise.</param>
    /// <returns>The new proxy with the specified caching policy.</returns>
    public ObjectPrx ice_connectionCached(bool newCache)
    {
        if (newCache == _reference.getCacheConnection())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeCacheConnection(newCache));
        }
    }

    /// <summary>
    /// Returns how this proxy selects endpoints (randomly or ordered).
    /// </summary>
    /// <returns>The endpoint selection policy.</returns>
    public EndpointSelectionType ice_getEndpointSelection()
    {
        return _reference.getEndpointSelection();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the endpoint selection policy.
    /// </summary>
    /// <param name="newType">The new endpoint selection policy.</param>
    /// <returns>The new proxy with the specified endpoint selection policy.</returns>
    public ObjectPrx ice_endpointSelection(EndpointSelectionType newType)
    {
        if (newType == _reference.getEndpointSelection())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeEndpointSelection(newType));
        }
    }

    /// <summary>
    /// Returns whether this proxy communicates only via secure endpoints.
    /// </summary>
    /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
    public bool ice_isSecure()
    {
        return _reference.getSecure();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for how it selects endpoints.
    /// </summary>
    /// <param name="secure"> If secure is true, only endpoints that use a secure transport are
    /// used by the new proxy. If secure is false, the returned proxy uses both secure and insecure
    /// endpoints.</param>
    /// <returns>The new proxy with the specified selection policy.</returns>
    public ObjectPrx ice_secure(bool secure)
    {
        if (secure == _reference.getSecure())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeSecure(secure));
        }
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the encoding used to marshal
    /// parameters.
    /// </summary>
    /// <param name="encodingVersion">The encoding version to use to marshal requests parameters.</param>
    /// <returns>The new proxy with the specified encoding version.</returns>
    public ObjectPrx ice_encodingVersion(EncodingVersion encodingVersion)
    {
        if (encodingVersion == _reference.getEncoding())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeEncoding(encodingVersion));
        }
    }

    /// <summary>Gets the encoding version used to marshal requests parameters.</summary>
    /// <returns>The encoding version.</returns>
    public EncodingVersion ice_getEncodingVersion()
    {
        return _reference.getEncoding();
    }

    /// <summary>
    /// Returns whether this proxy prefers secure endpoints.
    /// </summary>
    /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
    /// attempts to use insecure endpoints; false, otherwise.</returns>
    public bool ice_isPreferSecure()
    {
        return _reference.getPreferSecure();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for its endpoint selection policy.
    /// </summary>
    /// <param name="preferSecure">If preferSecure is true, the new proxy will use secure endpoints for invocations
    /// and only use insecure endpoints if an invocation cannot be made via secure endpoints. If preferSecure is
    /// false, the proxy prefers insecure endpoints to secure ones.</param>
    /// <returns>The new proxy with the new endpoint selection policy.</returns>
    public ObjectPrx ice_preferSecure(bool preferSecure)
    {
        if (preferSecure == _reference.getPreferSecure())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changePreferSecure(preferSecure));
        }
    }

    /// <summary>
    /// Gets the router for this proxy.
    /// </summary>
    /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
    /// is null.</returns>
    public RouterPrx? ice_getRouter()
    {
        RouterInfo ri = _reference.getRouterInfo();
        return ri != null ? ri.getRouter() : null;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the router.
    /// </summary>
    /// <param name="router">The router for the new proxy.</param>
    /// <returns>The new proxy with the specified router.</returns>
    public ObjectPrx ice_router(RouterPrx? router)
    {
        if (router == _reference.getRouterInfo()?.getRouter())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeRouter(router));
        }
    }

    /// <summary>
    /// Gets the locator for this proxy.
    /// </summary>
    /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
    public LocatorPrx? ice_getLocator()
    {
        var li = _reference.getLocatorInfo();
        return li != null ? li.getLocator() : null;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for the locator.
    /// </summary>
    /// <param name="locator">The locator for the new proxy.</param>
    /// <returns>The new proxy with the specified locator.</returns>
    public ObjectPrx ice_locator(LocatorPrx? locator)
    {
        if (locator == _reference.getLocatorInfo()?.getLocator())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeLocator(locator));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses collocation optimization.
    /// </summary>
    /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
    public bool ice_isCollocationOptimized()
    {
        return _reference.getCollocationOptimized();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for collocation optimization.
    /// </summary>
    /// <param name="collocated">True if the new proxy enables collocation optimization; false, otherwise.</param>
    /// <returns>The new proxy the specified collocation optimization.</returns>
    public ObjectPrx ice_collocationOptimized(bool collocated)
    {
        if (collocated == _reference.getCollocationOptimized())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeCollocationOptimized(collocated));
        }
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses twoway invocations.
    /// </summary>
    /// <returns>A new proxy that uses twoway invocations.</returns>
    public ObjectPrx ice_twoway()
    {
        if (_reference.isTwoway)
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeMode(Reference.Mode.ModeTwoway));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses twoway invocations.
    /// </summary>
    /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
    public bool ice_isTwoway() => _reference.isTwoway;

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses oneway invocations.
    /// </summary>
    /// <returns>A new proxy that uses oneway invocations.</returns>
    public ObjectPrx ice_oneway()
    {
        if (_reference.getMode() == Reference.Mode.ModeOneway)
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeMode(Reference.Mode.ModeOneway));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses oneway invocations.
    /// </summary>
    /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
    public bool ice_isOneway()
    {
        return _reference.getMode() == Reference.Mode.ModeOneway;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses batch oneway invocations.
    /// </summary>
    /// <returns>A new proxy that uses batch oneway invocations.</returns>
    public ObjectPrx ice_batchOneway()
    {
        if (_reference.getMode() == Reference.Mode.ModeBatchOneway)
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeMode(Reference.Mode.ModeBatchOneway));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses batch oneway invocations.
    /// </summary>
    /// <returns>True if this proxy uses batch oneway invocations; false, otherwise.</returns>
    public bool ice_isBatchOneway()
    {
        return _reference.getMode() == Reference.Mode.ModeBatchOneway;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses datagram invocations.
    /// </summary>
    /// <returns>A new proxy that uses datagram invocations.</returns>
    public ObjectPrx ice_datagram()
    {
        if (_reference.getMode() == Reference.Mode.ModeDatagram)
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeMode(Reference.Mode.ModeDatagram));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses datagram invocations.
    /// </summary>
    /// <returns>True if this proxy uses datagram invocations; false, otherwise.</returns>
    public bool ice_isDatagram()
    {
        return _reference.getMode() == Reference.Mode.ModeDatagram;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, but uses batch datagram invocations.
    /// </summary>
    /// <returns>A new proxy that uses batch datagram invocations.</returns>
    public ObjectPrx ice_batchDatagram()
    {
        if (_reference.getMode() == Reference.Mode.ModeBatchDatagram)
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeMode(Reference.Mode.ModeBatchDatagram));
        }
    }

    /// <summary>
    /// Returns whether this proxy uses batch datagram invocations.
    /// </summary>
    /// <returns>True if this proxy uses batch datagram invocations; false, otherwise.</returns>
    public bool ice_isBatchDatagram()
    {
        return _reference.getMode() == Reference.Mode.ModeBatchDatagram;
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for compression.
    /// </summary>
    /// <param name="compress">True enables compression for the new proxy; false disables compression.</param>
    /// <returns>A new proxy with the specified compression setting.</returns>
    public ObjectPrx ice_compress(bool compress)
    {
        if (compress == _reference.getCompress())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeCompress(compress));
        }
    }

    /// <summary>
    /// Obtains the compression override setting of this proxy.
    /// </summary>
    /// <returns>The compression override setting. If no optional value is present, no override is
    /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
    public bool? ice_getCompress()
    {
        return _reference.getCompress();
    }

    /// <summary>
    /// Creates a new proxy that is identical to this proxy, except for its connection ID.
    /// </summary>
    /// <param name="connectionId">The connection ID for the new proxy. An empty string removes the
    /// connection ID.</param>
    /// <returns>A new proxy with the specified connection ID.</returns>
    public ObjectPrx ice_connectionId(string connectionId)
    {
        if (connectionId == _reference.getConnectionId())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeConnectionId(connectionId));
        }
    }

    /// <summary>
    /// Gets the connection id of this proxy.
    /// </summary>
    /// <returns>The connection id.</returns>
    public string ice_getConnectionId()
    {
        return _reference.getConnectionId();
    }

    /// <summary>
    /// Returns a proxy that is identical to this proxy, except it's a fixed proxy bound
    /// the given connection.
    /// </summary>
    /// <param name="connection">The fixed proxy connection.</param>
    /// <returns>A fixed proxy bound to the given connection.</returns>
    public ObjectPrx ice_fixed(Ice.Connection connection)
    {
        if (connection is null)
        {
            throw new ArgumentException("invalid null connection passed to ice_fixed");
        }
        if (!(connection is Ice.ConnectionI))
        {
            throw new ArgumentException("invalid connection passed to ice_fixed");
        }

        if (connection == _reference.getConnection())
        {
            return this;
        }
        else
        {
            return iceNewInstance(_reference.changeConnection((Ice.ConnectionI)connection));
        }
    }

    /// <summary>
    /// Returns whether this proxy is a fixed proxy.
    /// </summary>
    /// <returns>True if this is a fixed proxy, false otherwise.
    /// </returns>
    public bool ice_isFixed()
    {
        return _reference is Ice.Internal.FixedReference;
    }

    public class GetConnectionTaskCompletionCallback : TaskCompletionCallback<Connection?>
    {
        public GetConnectionTaskCompletionCallback(
            IProgress<bool>? progress = null,
            CancellationToken cancellationToken = default)
            : base(progress, cancellationToken)
        {
        }

        public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
        {
            SetResult(((ProxyGetConnection)og).getConnection());
        }
    }

    public Connection? ice_getConnection()
    {
        try
        {
            var completed = new GetConnectionTaskCompletionCallback();
            iceI_ice_getConnection(completed, true);
            return completed.Task.Result;
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    public Task<Connection?> ice_getConnectionAsync(IProgress<bool>? progress, CancellationToken cancel)
    {
        var completed = new GetConnectionTaskCompletionCallback(progress, cancel);
        iceI_ice_getConnection(completed, false);
        return completed.Task;
    }

    private const string _ice_getConnection_name = "ice_getConnection";

    private void iceI_ice_getConnection(OutgoingAsyncCompletionCallback completed, bool synchronous)
    {
        var outgoing = new ProxyGetConnection(this, completed);
        outgoing.invoke(_ice_getConnection_name, synchronous);
    }

    /// <summary>
    /// Gets the cached Connection for this proxy. If the proxy does not yet have an established
    /// connection, it does not attempt to create a connection.
    /// </summary>
    /// <returns>The cached Connection for this proxy (null if the proxy does not have
    /// an established connection).</returns>
    public Connection? ice_getCachedConnection() => _requestHandlerCache.cachedConnection;

    /// <summary>
    /// Flushes any pending batched requests for this communicator. The call blocks until the flush is complete.
    /// </summary>
    public void ice_flushBatchRequests()
    {
        try
        {
            var completed = new FlushBatchTaskCompletionCallback();
            iceI_ice_flushBatchRequests(completed, true);
            completed.Task.Wait();
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException!;
        }
    }

    internal const string _ice_flushBatchRequests_name = "ice_flushBatchRequests";

    public Task ice_flushBatchRequestsAsync(
        IProgress<bool>? progress = null,
        CancellationToken cancel = default)
    {
        var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
        iceI_ice_flushBatchRequests(completed, false);
        return completed.Task;
    }

    private void iceI_ice_flushBatchRequests(OutgoingAsyncCompletionCallback completed, bool synchronous)
    {
        var outgoing = new ProxyFlushBatchAsync(this, completed);
        outgoing.invoke(_ice_flushBatchRequests_name, synchronous);
    }

    public System.Threading.Tasks.TaskScheduler ice_scheduler()
    {
        return _reference.getThreadPool();
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public void iceWrite(OutputStream os)
    {
        Identity.ice_write(os, _reference.getIdentity());
        _reference.streamWrite(os);
    }

    internal Reference iceReference() => _reference;

    [EditorBrowsable(EditorBrowsableState.Never)]
    public void iceCheckTwowayOnly(string name)
    {
        //
        // No mutex lock necessary, there is nothing mutable in this
        // operation.
        //

        if (!ice_isTwoway())
        {
            throw new TwowayOnlyException(name);
        }
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public void iceCheckAsyncTwowayOnly(string name)
    {
        //
        // No mutex lock necessary, there is nothing mutable in this
        // operation.
        //

        if (!ice_isTwoway())
        {
            throw new ArgumentException("`" + name + "' can only be called with a twoway proxy");
        }
    }

    internal RequestHandlerCache iceGetRequestHandlerCache() => _requestHandlerCache;

    protected ObjectPrxHelperBase(ObjectPrx proxy)
    {
        // We don't supported decorated proxies here.
        var helper = (ObjectPrxHelperBase)proxy;

        _reference = helper._reference;
        _requestHandlerCache = helper._requestHandlerCache;
    }

    protected OutgoingAsyncT<T>
    getOutgoingAsync<T>(OutgoingAsyncCompletionCallback completed)
    {
        bool haveEntry = false;
        InputStream? iss = null;
        OutputStream? os = null;

        if (_reference.getInstance().cacheMessageBuffers() > 0)
        {
            lock (_mutex)
            {
                if (_streamCache != null && _streamCache.Count > 0)
                {
                    haveEntry = true;
                    iss = _streamCache.First!.Value.iss;
                    os = _streamCache.First.Value.os;

                    _streamCache.RemoveFirst();
                }
            }
        }
        if (!haveEntry)
        {
            return new OutgoingAsyncT<T>(this, completed);
        }
        else
        {
            return new OutgoingAsyncT<T>(this, completed, os, iss);
        }
    }

    private class InvokeOutgoingAsyncT : OutgoingAsync
    {
        public InvokeOutgoingAsyncT(
            ObjectPrxHelperBase prx,
            OutgoingAsyncCompletionCallback completionCallback,
            OutputStream? os = null,
            InputStream? iss = null)
            : base(prx, completionCallback, os, iss)
        {
        }

        public void invoke(
            string operation,
            OperationMode mode,
            byte[] inParams,
            Dictionary<string, string>? context,
            bool synchronous)
        {
            try
            {
                prepare(operation, mode, context);
                if (inParams is null || inParams.Length == 0)
                {
                    os_.writeEmptyEncapsulation(encoding_);
                }
                else
                {
                    os_.writeEncapsulation(inParams);
                }
                invoke(operation, synchronous);
            }
            catch (Exception ex)
            {
                abort(ex);
            }
        }

        public Object_Ice_invokeResult
        getResult(bool ok)
        {
            try
            {
                var ret = new Object_Ice_invokeResult();
                EncodingVersion encoding;
                if (proxy_.iceReference().isTwoway)
                {
                    ret.outEncaps = is_.readEncapsulation(out encoding);
                }
                else
                {
                    ret.outEncaps = [];
                }
                ret.returnValue = ok;
                return ret;
            }
            finally
            {
                cacheMessageBuffers();
            }
        }
    }

    private class InvokeTaskCompletionCallback : TaskCompletionCallback<Object_Ice_invokeResult>
    {
        public InvokeTaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken)
            : base(progress, cancellationToken)
        {
        }

        public override void handleInvokeSent(
            bool sentSynchronously,
            bool done,
            bool alreadySent,
            OutgoingAsyncBase og)
        {
            if (progress_ != null && !alreadySent)
            {
                progress_.Report(sentSynchronously);
            }
            if (done)
            {
                SetResult(new Object_Ice_invokeResult(true, []));
            }
        }

        public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
        {
            SetResult(((InvokeOutgoingAsyncT)og).getResult(ok));
        }
    }

    private InvokeOutgoingAsyncT
    getInvokeOutgoingAsync(OutgoingAsyncCompletionCallback completed)
    {
        bool haveEntry = false;
        InputStream? iss = null;
        OutputStream? os = null;

        if (_reference.getInstance().cacheMessageBuffers() > 0)
        {
            lock (_mutex)
            {
                if (_streamCache != null && _streamCache.Count > 0)
                {
                    haveEntry = true;
                    iss = _streamCache.First!.Value.iss;
                    os = _streamCache.First.Value.os;

                    _streamCache.RemoveFirst();
                }
            }
        }
        if (!haveEntry)
        {
            return new InvokeOutgoingAsyncT(this, completed);
        }
        else
        {
            return new InvokeOutgoingAsyncT(this, completed, os, iss);
        }
    }

    internal void cacheMessageBuffers(InputStream iss, OutputStream os)
    {
        lock (_mutex)
        {
            _streamCache ??= new LinkedList<(InputStream, OutputStream)>();
            _streamCache.AddLast((iss, os));
        }
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectPrxHelperBase" /> class.
    /// </summary>
    /// <param name="reference">The reference for the new proxy.</param>
    [EditorBrowsable(EditorBrowsableState.Never)]
    protected ObjectPrxHelperBase(Reference reference)
    {
        _reference = reference;
        _requestHandlerCache = new RequestHandlerCache(reference);
    }

    /// <summary>
    /// Constructs a new proxy instance.
    /// </summary>
    /// <param name="reference">The reference for the new proxy.</param>
    /// <returns>The new proxy instance.</returns>
    [EditorBrowsable(EditorBrowsableState.Never)]
    protected abstract ObjectPrxHelperBase iceNewInstance(Reference reference);

    private readonly Reference _reference;
    private readonly RequestHandlerCache _requestHandlerCache;
    private LinkedList<(InputStream iss, OutputStream os)>? _streamCache;
    private readonly object _mutex = new();
}

/// <summary>
/// Base class for all proxy helpers.
/// </summary>
public class ObjectPrxHelper : ObjectPrxHelperBase
{
    /// <summary>
    /// Creates a new proxy that implements <see cref="ObjectPrx" />.
    /// </summary>
    /// <param name="communicator">The communicator of the new proxy.</param>
    /// <param name="proxyString">The string representation of the proxy.</param>
    /// <returns>The new proxy.</returns>
    /// <exception name="ParseException">Thrown when <paramref name="proxyString" /> is not a valid proxy string.
    /// </exception>
    public static ObjectPrx createProxy(Communicator communicator, string proxyString)
    {
        Reference? reference = communicator.instance.referenceFactory().create(proxyString, "");

        return reference is not null ?
            new ObjectPrxHelper(reference) :
            throw new ParseException("Invalid empty proxy string.");
    }

    /// <summary>
    /// Casts a proxy to <see cref="ObjectPrx" />. This call contacts
    /// the server and throws an Ice run-time exception if the target
    /// object does not exist or the server cannot be reached.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <param name="context">The Context map for the invocation.</param>
    /// <returns>proxy.</returns>
    public static ObjectPrx? checkedCast(ObjectPrx? proxy, Dictionary<string, string>? context = null) =>
        proxy is not null && proxy.ice_isA("::Ice::Object", context) ? proxy : null;

    /// <summary>
    /// Creates a new proxy that is identical to the passed proxy, except
    /// for its facet. This call contacts
    /// the server and throws an Ice run-time exception if the target
    /// object does not exist, the specified facet does not exist, or the server cannot be reached.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <param name="facet">The facet for the new proxy.</param>
    /// <param name="context">The Context map for the invocation.</param>
    /// <returns>The new proxy with the specified facet.</returns>
    public static ObjectPrx? checkedCast(ObjectPrx? proxy, string facet, Dictionary<string, string>? context = null) =>
        checkedCast(proxy?.ice_facet(facet), context);

    /// <summary>
    /// Casts a proxy to <see cref="ObjectPrx" />. This call contacts
    /// the server and throws an Ice run-time exception if the target
    /// object does not exist or the server cannot be reached.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <param name="context">The Context map for the invocation.</param>
    /// <returns>proxy.</returns>
    public static async Task<ObjectPrx?> checkedCastAsync(ObjectPrx proxy, Dictionary<string, string>? context = null) =>
       await proxy.ice_isAAsync("::Ice::Object", context).ConfigureAwait(false) ? proxy : null;

    /// <summary>
    /// Creates a new proxy that is identical to the passed proxy, except
    /// for its facet. This call contacts
    /// the server and throws an Ice run-time exception if the target
    /// object does not exist, the specified facet does not exist, or the server cannot be reached.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <param name="facet">The facet for the new proxy.</param>
    /// <param name="context">The Context map for the invocation.</param>
    /// <returns>The new proxy with the specified facet.</returns>
    public static Task<ObjectPrx?> checkedCastAsync(ObjectPrx proxy, string facet, Dictionary<string, string>? context = null) =>
        checkedCastAsync(proxy.ice_facet(facet), context);

    /// <summary>
    /// Casts a proxy to <see cref="ObjectPrx" />. This call does
    /// not contact the server and always succeeds.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <returns>b.</returns>
    [return: NotNullIfNotNull("proxy")]
    public static ObjectPrx? uncheckedCast(ObjectPrx? proxy) => proxy;

    /// <summary>
    /// Creates a new proxy that is identical to the passed proxy, except
    /// for its facet. This call does not contact the server and always succeeds.
    /// </summary>
    /// <param name="proxy">The proxy to cast to ObjectPrx.</param>
    /// <param name="facet">The facet for the new proxy.</param>
    /// <returns>The new proxy with the specified facet.</returns>
    [return: NotNullIfNotNull("proxy")]
    public static ObjectPrx? uncheckedCast(ObjectPrx? proxy, string facet) => proxy?.ice_facet(facet);

    /// <summary>
    /// Gets the Slice type id of the interface or class associated
    /// with this proxy class.
    /// </summary>
    /// <returns>The type id, "::Ice::Object".</returns>
    public static string ice_staticId()
    {
        return ObjectImpl.ice_staticId();
    }

    protected override ObjectPrxHelperBase iceNewInstance(Reference reference) => new ObjectPrxHelper(reference);

    internal ObjectPrxHelper(Reference reference)
        : base(reference)
    {
    }
}
