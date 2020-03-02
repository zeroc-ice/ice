//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;
using IceInternal;

namespace Ice
{
    public enum InvocationMode : byte
    {
        Twoway,
        Oneway,
        BatchOneway,
        Datagram,
        BatchDatagram,
        Last = BatchDatagram
    }

    public delegate T ProxyFactory<T>(Reference reference) where T : IObjectPrx;

    /// <summary>
    /// Base interface of all object proxies.
    /// </summary>
    public interface IObjectPrx : IEquatable<IObjectPrx>
    {
        public Reference IceReference { get; }

        public static readonly InputStreamReader<IObjectPrx?> IceReader = (istr) => istr.ReadProxy(Factory);
        public static readonly OutputStreamWriter<IObjectPrx?> IceWriter = (ostr, value) => ostr.WriteProxy(value);

        public IRequestHandler? RequestHandler { get; set; }

        public IObjectPrx Clone(Reference reference);

        /// <summary>
        /// Returns the communicator that created this proxy.
        /// </summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator Communicator => IceReference.GetCommunicator();

        /// <summary>
        /// Convert a proxy to a set of proxy properties.
        /// </summary>
        /// <param name="property">
        /// The base property name.
        /// </param>
        /// <returns>The property set.</returns>
        public Dictionary<string, string> ToProperty(string property) => IceReference.ToProperty(property);

        /// <summary>Tests whether the target object implements a specific Slice interface.</summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>True if the target object implements the Slice interface identified by id</returns>.
        public bool IceIsA(string id, Dictionary<string, string>? context = null)
        {
            try
            {
                return IceIsAAsync(id, context, null, CancellationToken.None, synchronous: true).Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<bool> IceIsAAsync(string id,
                                      Dictionary<string, string>? context = null,
                                      IProgress<bool>? progress = null,
                                      CancellationToken cancel = default) =>
            IceIsAAsync(id, context, progress, cancel, synchronous: false);

        private Task<bool> IceIsAAsync(string id, Dictionary<string, string>? context, IProgress<bool>? progress,
                                       CancellationToken cancel, bool synchronous)
        {
            IceCheckTwowayOnly("ice_isA");

            var request = new OutgoingRequestFrame(this, "ice_isA", idempotent: true, context);
            request.StartParameters();
            request.WriteString(id);
            request.EndParameters();
            return AwaitResponseAsync(IceInvokeAsync(request, progress, cancel, synchronous));

            static async Task<bool> AwaitResponseAsync(Task<IncomingResponseFrame> task)
            {
                try
                {
                    InputStream istr = (await task.ConfigureAwait(false)).ReadReturnValue();
                    bool returnValue = istr.ReadBool();
                    istr.EndEncapsulation(); // TODO: need a better name, like maybe Done()
                    return returnValue;
                }
                catch (UserException userException)
                {
                    // TODO: remove this try/catch block once we eliminate checked exceptions.
                    throw new UnknownUserException(userException);
                }
            }
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(Dictionary<string, string>? context = null)
        {
            try
            {
                IcePingAsync(context, null, CancellationToken.None, synchronous: true).Wait();
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task IcePingAsync(Dictionary<string, string>? context = null,
                                 IProgress<bool>? progress = null,
                                 CancellationToken cancel = default)
            => IcePingAsync(context, progress, cancel, synchronous: false);

        private Task IcePingAsync(Dictionary<string, string>? context, IProgress<bool>? progress,
                                  CancellationToken cancel, bool synchronous)
        {
            var request = OutgoingRequestFrame.Empty(this, "ice_ping", idempotent: true, context);
            var task = IceInvokeAsync(request, progress, cancel, synchronous);
            return IsOneway ? task : AwaitResponseAsync(task);

            static async Task AwaitResponseAsync(Task<IncomingResponseFrame> task)
            {
                try
                {
                    (await task.ConfigureAwait(false)).ReadVoidReturnValue();
                }
                catch (UserException userException)
                {
                    // TODO: remove this try/catch block once we eliminate checked exceptions.
                    throw new UnknownUserException(userException);
                }
            }
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(Dictionary<string, string>? context = null)
        {
            try
            {
                return IceIdsAsync(context, null, CancellationToken.None, synchronous: true).Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]> IceIdsAsync(Dictionary<string, string>? context = null,
                                          IProgress<bool>? progress = null,
                                          CancellationToken cancel = default)
            => IceIdsAsync(context, progress, cancel, synchronous: false);

        private Task<string[]> IceIdsAsync(Dictionary<string, string>? context, IProgress<bool>? progress,
                                           CancellationToken cancel, bool synchronous)
        {
            IceCheckTwowayOnly("ice_ids");
            var request = OutgoingRequestFrame.Empty(this, "ice_ids", idempotent: true, context);
            return AwaitResponseAsync(IceInvokeAsync(request, progress, cancel, synchronous));

            static async Task<string[]> AwaitResponseAsync(Task<IncomingResponseFrame> task)
            {
                try
                {
                    InputStream istr = (await task.ConfigureAwait(false)).ReadReturnValue();
                    string[] returnValue = istr.ReadStringArray();
                    istr.EndEncapsulation(); // TODO: need a better name, like maybe Done()
                    return returnValue;
                }
                catch (UserException userException)
                {
                    // TODO: remove this try/catch block once we eliminate checked exceptions.
                    throw new UnknownUserException(userException);
                }
            }
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Dictionary<string, string>? context = null)
        {
            try
            {
                return IceIdAsync(context, null, CancellationToken.None, synchronous: true).Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> IceIdAsync(Dictionary<string, string>? context = null,
                                       IProgress<bool>? progress = null,
                                       CancellationToken cancel = new CancellationToken()) =>
            IceIdAsync(context, progress, cancel, synchronous: false);

        private Task<string> IceIdAsync(Dictionary<string, string>? context, IProgress<bool>? progress,
                                        CancellationToken cancel, bool synchronous)
        {
            IceCheckTwowayOnly("ice_id");
            var request = OutgoingRequestFrame.Empty(this, "ice_id", idempotent: true, context);
            return AwaitResponseAsync(IceInvokeAsync(request, progress, cancel, synchronous));

            static async Task<string> AwaitResponseAsync(Task<IncomingResponseFrame> task)
            {
                try
                {
                    InputStream istr = (await task.ConfigureAwait(false)).ReadReturnValue();
                    string returnValue = istr.ReadString();
                    istr.EndEncapsulation(); // TODO: need a better name, like maybe Done()
                    return returnValue;
                }
                catch (UserException userException)
                {
                    // TODO: remove this try/catch block once we eliminate checked exceptions.
                    throw new UnknownUserException(userException);
                }
            }
        }

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        public Identity Identity => IceReference.GetIdentity();

        /// <summary>
        /// Returns the per-proxy context for this proxy.
        /// </summary>
        /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
        /// is null.</returns>
        public Dictionary<string, string>? Context
        {
            get
            {
                Dictionary<string, string> context = IceReference.GetContext();
                if (context == null)
                {
                    return null;
                }
                else
                {
                    return new Dictionary<string, string>(context);
                }
            }
        }

        /// <summary>
        /// Returns the facet for this proxy.
        /// </summary>
        /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
        /// empty string.</returns>
        public string Facet => IceReference.GetFacet();

        /// <summary>
        /// Returns the adapter ID for this proxy.
        /// </summary>
        /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
        /// empty string.</returns>
        public string AdapterId => IceReference.GetAdapterId();

        /// <summary>
        /// Returns the endpoints used by this proxy.
        /// </summary>
        /// <returns>The endpoints used by this proxy.</returns>
        public IEndpoint[] Endpoints => (IEndpoint[])IceReference.GetEndpoints().Clone();

        /// <summary>
        /// Returns the locator cache timeout of this proxy.
        /// </summary>
        /// <returns>The locator cache timeout value (in seconds).</returns>
        public int LocatorCacheTimeout => IceReference.GetLocatorCacheTimeout();

        /// <summary>
        /// Returns the invocation timeout of this proxy.
        /// </summary>
        /// <returns>The invocation timeout value (in seconds).</returns>
        public int InvocationTimeout => IceReference.GetInvocationTimeout();

        /// <summary>
        /// Returns whether this proxy caches connections.
        /// </summary>
        /// <returns>True if this proxy caches connections; false, otherwise.</returns>
        public bool IsConnectionCached => IceReference.GetCacheConnection();

        /// <summary>
        /// Returns how this proxy selects endpoints (randomly or ordered).
        /// </summary>
        /// <returns>The endpoint selection policy.</returns>
        public EndpointSelectionType EndpointSelection => IceReference.GetEndpointSelection();

        /// <summary>
        /// Returns whether this proxy communicates only via secure endpoints.
        /// </summary>
        /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
        public bool IsSecure => IceReference.GetSecure();

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        public EncodingVersion EncodingVersion => IceReference.GetEncoding();

        /// <summary>
        /// Returns whether this proxy prefers secure endpoints.
        /// </summary>
        /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
        /// attempts to use insecure endpoints; false, otherwise.</returns>
        public bool IsPreferSecure => IceReference.GetPreferSecure();

        /// <summary>
        /// Returns the router for this proxy.
        /// </summary>
        /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
        /// is null.</returns>
        public IRouterPrx? Router => IceReference.GetRouterInfo()?.Router;

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        public ILocatorPrx? Locator => IceReference.GetLocatorInfo()?.Locator;

        /// <summary>
        /// Returns whether this proxy uses collocation optimization.
        /// </summary>
        /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
        public bool IsCollocationOptimized => IceReference.GetCollocationOptimized();

        /// <summary>Returns whether or not an operation invoked on this proxy returns a response.</summary>
        /// <returns>True if invoking an operation on this proxy does not return a response. This corresponds to
        /// several <see cref="InvocationMode"/> enumerators, such as Oneway and Datagram. Otherwise,
        /// returns false.</returns>
        public bool IsOneway => IceReference.GetMode() != InvocationMode.Twoway;

        public InvocationMode InvocationMode => IceReference.GetMode();

        /// <summary>
        /// Obtains the compression override setting of this proxy.
        /// </summary>
        /// <returns>The compression override setting. If no optional value is present, no override is
        /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
        public bool? Compress => IceReference.GetCompress();

        /// <summary>
        /// Obtains the timeout override of this proxy.
        /// </summary>
        /// <returns>The timeout override. If no optional value is present, no override is set. Otherwise,
        /// returns the timeout override value.</returns>
        public int? ConnectionTimeout => IceReference.GetTimeout();

        /// <summary>
        /// Returns the connection id of this proxy.
        /// </summary>
        /// <returns>The connection id.</returns>
        public string ConnectionId => IceReference.GetConnectionId();

        /// <summary>
        /// Returns whether this proxy is a fixed proxy.
        /// </summary>
        /// <returns>True if this is a fixed proxy, false otherwise.
        /// </returns>
        public bool IsFixed => IceReference is FixedReference;

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Task<IncomingResponseFrame> IceInvokeAsync(OutgoingRequestFrame request,
                                                          IProgress<bool>? progress,
                                                          CancellationToken cancel,
                                                          bool synchronous)
        {
            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(this, completed, request).Invoke(request.Operation, request.Context,
                synchronous);
            return completed.Task;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceWrite(OutputStream os)
        {
            IceReference.GetIdentity().IceWrite(os);
            IceReference.StreamWrite(os);
        }

        public TaskScheduler Scheduler => IceReference.GetThreadPool();

        public static bool Equals(IObjectPrx? lhs, IObjectPrx? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }

            return lhs.IceReference.Equals(rhs.IceReference);
        }

        public static readonly ProxyFactory<IObjectPrx> Factory = (reference) => new ObjectPrx(reference);

        public static IObjectPrx Parse(string s, Communicator communicator) =>
            new ObjectPrx(communicator.CreateReference(s));

        public static bool TryParse(string s, Communicator communicator, out IObjectPrx? prx)
        {
            try
            {
                prx = new ObjectPrx(communicator.CreateReference(s));
            }
            catch (System.Exception)
            {
                prx = null;
                return false;
            }
            return true;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected void IceCheckTwowayOnly(string name)
        {
            if (IsOneway)
            {
                throw new TwowayOnlyException(name);
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public int IceHandleException(Exception ex, IRequestHandler? handler, bool idempotent, bool sent,
                                      ref int cnt)
        {
            IceUpdateRequestHandler(handler, null); // Clear the request handler

            //
            // We only retry local exception, system exceptions aren't retried.
            //
            // A CloseConnectionException indicates graceful server shutdown, and is therefore
            // always repeatable without violating "at-most-once". That's because by sending a
            // close connection message, the server guarantees that all outstanding requests
            // can safely be repeated.
            //
            // An ObjectNotExistException can always be retried as well without violating
            // "at-most-once" (see the implementation of the checkRetryAfterException method
            //  of the ProxyFactory class for the reasons why it can be useful).
            //
            // If the request didn't get sent or if it's non-mutating or idempotent it can
            // also always be retried if the retry count isn't reached.
            //
            if (ex is LocalException && (!sent ||
                                        idempotent ||
                                        ex is CloseConnectionException ||
                                        ex is ObjectNotExistException))
            {
                try
                {
                    return IceReference.GetCommunicator().CheckRetryAfterException((LocalException)ex, IceReference,
                        ref cnt);
                }
                catch (CommunicatorDestroyedException)
                {
                    //
                    // The communicator is already destroyed, so we cannot retry.
                    //
                    throw ex;
                }
            }
            else
            {
                throw ex; // Retry could break at-most-once semantics, don't retry.
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public IRequestHandler IceGetRequestHandler()
        {
            if (IceReference.GetCacheConnection())
            {
                lock (this)
                {
                    if (RequestHandler != null)
                    {
                        return RequestHandler;
                    }
                }
            }
            return IceReference.GetRequestHandler(this);
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public IRequestHandler
        IceSetRequestHandler(IRequestHandler handler)
        {
            if (IceReference.GetCacheConnection())
            {
                lock (this)
                {
                    if (RequestHandler == null)
                    {
                        RequestHandler = handler;
                    }
                    return RequestHandler;
                }
            }
            return handler;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceUpdateRequestHandler(IRequestHandler? previous, IRequestHandler? handler)
        {
            if (IceReference.GetCacheConnection() && previous != null)
            {
                lock (this)
                {
                    if (RequestHandler != null && RequestHandler != handler)
                    {
                        //
                        // Update the request handler only if "previous" is the same
                        // as the current request handler. This is called after
                        // connection binding by the connect request handler. We only
                        // replace the request handler if the current handler is the
                        // connect request handler.
                        //
                        RequestHandler = RequestHandler.Update(previous, handler);
                    }
                }
            }
        }
    }

    internal class InvokeTaskCompletionCallback : TaskCompletionCallback<IncomingResponseFrame>
    {
        public InvokeTaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken) :
            base(progress, cancellationToken)
        {
        }

        public override void HandleInvokeSent(bool sentSynchronously, bool done, bool alreadySent,
                                              OutgoingAsyncBase og)
        {
            if (Progress != null && !alreadySent)
            {
                Progress.Report(sentSynchronously);
            }
            if (done)
            {
                SetResult(new IncomingResponseFrame(ReplyStatus.OK, og.GetIs()));
            }
        }

        public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) =>
            SetResult(new IncomingResponseFrame(ok ? ReplyStatus.OK : ReplyStatus.UserException, og.GetIs()));
    }

    /// <summary>
    /// Base class of all object proxies.
    /// </summary>
    [Serializable]
    public class ObjectPrx : IObjectPrx, ISerializable
    {
        public Reference IceReference { get; private set; }
        public IRequestHandler? RequestHandler { get; set; }

        public virtual IObjectPrx Clone(Reference reference) => new ObjectPrx(reference);

        public ObjectPrx(Reference reference, IRequestHandler? requestHandler = null)
        {
            IceReference = reference ?? throw new ArgumentNullException(nameof(reference));
            RequestHandler = requestHandler;
        }

        protected ObjectPrx(SerializationInfo info, StreamingContext context)
        {
            if (!(context.Context is Communicator communicator))
            {
                throw new ArgumentException("Cannot deserialize proxy: Ice.Communicator not found in StreamingContext");
            }
            IceReference = communicator.CreateReference(info.GetString("proxy"), null);
        }

        public virtual void GetObjectData(SerializationInfo info, StreamingContext context) =>
            info.AddValue("proxy", ToString());

        /// <summary>
        /// Returns the stringified form of this proxy.
        /// </summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString()
        {
            Debug.Assert(IceReference != null);
            return IceReference.ToString();
        }

        /// <summary>
        /// Returns a hash code for this proxy.
        /// </summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode() => IceReference.GetHashCode();

        /// <summary>
        /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="other">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
        public override bool Equals(object? other) => Equals(other as IObjectPrx);

        /// <summary>
        /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="other">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
        public bool Equals(IObjectPrx? other) => other != null && IceReference.Equals(other.IceReference);
    }

    public static class Proxy
    {
        public static IObjectPrx Clone(this IObjectPrx prx,
                                       Identity id,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       Dictionary<string, string>? context = null,
                                       EncodingVersion? encodingVersion = null,
                                       EndpointSelectionType? endpointSelectionType = null,
                                       IEndpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       ILocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       IRouterPrx? router = null,
                                       bool? secure = null)
        {
            Reference reference = prx.IceReference.Clone(
                id,
                null,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encodingVersion,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : prx.Clone(reference);
        }

        public static IObjectPrx Clone(this IObjectPrx prx,
                                       string facet,
                                       string? adapterId = null,
                                       bool clearLocator = false,
                                       bool clearRouter = false,
                                       bool? collocationOptimized = null,
                                       bool? compress = null,
                                       bool? connectionCached = null,
                                       string? connectionId = null,
                                       int? connectionTimeout = null,
                                       Dictionary<string, string>? context = null,
                                       EncodingVersion? encodingVersion = null,
                                       EndpointSelectionType? endpointSelectionType = null,
                                       IEndpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       ILocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       IRouterPrx? router = null,
                                       bool? secure = null)
        {
            Reference reference = prx.IceReference.Clone(
                null,
                facet,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encodingVersion,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : prx.Clone(reference);
        }

        public static Prx Clone<Prx>(this Prx prx,
                                     string? adapterId = null,
                                     bool clearLocator = false,
                                     bool clearRouter = false,
                                     bool? collocationOptimized = null,
                                     bool? compress = null,
                                     bool? connectionCached = null,
                                     string? connectionId = null,
                                     int? connectionTimeout = null,
                                     Dictionary<string, string>? context = null,
                                     EncodingVersion? encodingVersion = null,
                                     EndpointSelectionType? endpointSelectionType = null,
                                     IEndpoint[]? endpoints = null,
                                     Connection? fixedConnection = null,
                                     InvocationMode? invocationMode = null,
                                     int? invocationTimeout = null,
                                     ILocatorPrx? locator = null,
                                     int? locatorCacheTimeout = null,
                                     bool? oneway = null,
                                     bool? preferSecure = null,
                                     IRouterPrx? router = null,
                                     bool? secure = null) where Prx : IObjectPrx
        {
            Reference reference = prx.IceReference.Clone(
                null,
                null,
                adapterId,
                clearLocator,
                clearRouter,
                collocationOptimized,
                compress,
                connectionCached,
                connectionId,
                connectionTimeout,
                context,
                encodingVersion,
                endpointSelectionType,
                endpoints,
                fixedConnection,
                invocationMode,
                invocationTimeout,
                locator,
                locatorCacheTimeout,
                oneway,
                preferSecure,
                router,
                secure);
            return reference.Equals(prx.IceReference) ? prx : (Prx)prx.Clone(reference);
        }

        public class GetConnectionTaskCompletionCallback : TaskCompletionCallback<Connection>
        {
            public GetConnectionTaskCompletionCallback(IProgress<bool>? progress = null,
                                                       CancellationToken cancellationToken = new CancellationToken()) :
                base(progress, cancellationToken)
            {
            }

            public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) =>
                SetResult(((ProxyGetConnection)og).GetConnection()!);
        }

        /// <summary>
        /// Returns the Connection for this proxy. If the proxy does not yet have an established connection,
        /// it first attempts to create a connection.
        /// </summary>
        /// <returns>The Connection for this proxy.</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        public static Connection GetConnection(this IObjectPrx prx)
        {
            try
            {
                var completed = new GetConnectionTaskCompletionCallback();
                IceI_getConnection(prx, completed, true);
                return completed.Task.Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        public static Task<Connection> GetConnectionAsync(this IObjectPrx prx,
                                                          IProgress<bool>? progress = null,
                                                          CancellationToken cancel = new CancellationToken())
        {
            var completed = new GetConnectionTaskCompletionCallback(progress, cancel);
            IceI_getConnection(prx, completed, false);
            return completed.Task;
        }

        private static void IceI_getConnection(IObjectPrx prx, IOutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            var outgoing = new ProxyGetConnection(prx, completed);
            try
            {
                outgoing.Invoke("ice_getConnection", synchronous);
            }
            catch (Exception ex)
            {
                outgoing.Abort(ex);
            }
        }

        /// <summary>
        /// Returns the cached Connection for this proxy. If the proxy does not yet have an established
        /// connection, it does not attempt to create a connection.
        /// </summary>
        /// <returns>The cached Connection for this proxy (null if the proxy does not have
        /// an established connection).</returns>
        /// <exception name="CollocationOptimizationException">If the proxy uses collocation optimization and denotes a
        /// collocated object.</exception>
        public static Connection? GetCachedConnection(this IObjectPrx prx)
        {
            IRequestHandler? handler;
            lock (prx)
            {
                handler = prx.RequestHandler;
            }

            if (handler != null)
            {
                try
                {
                    return handler.GetConnection();
                }
                catch (LocalException)
                {
                }
            }
            return null;
        }

        /// <summary>Sends a request synchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <returns>The response frame.</returns>
        public static IncomingResponseFrame Invoke(this IObjectPrx proxy, OutgoingRequestFrame request)
        {
            try
            {
                return proxy.IceInvokeAsync(request, null, CancellationToken.None, true).Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>Sends a request asynchronously.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The outgoing request frame for this invocation. Usually this request frame should have
        /// been created using the same proxy, however some differences are acceptable, for example proxy can have
        /// different endpoints.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static Task<IncomingResponseFrame> InvokeAsync(this IObjectPrx proxy,
                                                              OutgoingRequestFrame request,
                                                              IProgress<bool>? progress = null,
                                                              CancellationToken cancel = default)
            => proxy.IceInvokeAsync(request, progress, cancel, false);

        /// <summary>Forwards an incoming request to another Ice object.</summary>
        /// <param name="proxy">The proxy for the target Ice object.</param>
        /// <param name="request">The incoming request frame.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A task holding the response frame.</returns>
        public static async ValueTask<OutputStream> ForwardAsync(this IObjectPrx proxy,
                                                                 IncomingRequestFrame request,
                                                                 IProgress<bool>? progress = null,
                                                                 CancellationToken cancel = default)
        {
            var forwardedRequest = new OutgoingRequestFrame(proxy, request.Current.Operation,
                request.Current.IsIdempotent, request.Current.Context, request.TakePayload());

            IncomingResponseFrame response =
                await proxy.InvokeAsync(forwardedRequest, progress, cancel).ConfigureAwait(false);
            return new OutgoingResponseFrame(request.Current, response.ReplyStatus, response.TakePayload());
        }
    }
}
