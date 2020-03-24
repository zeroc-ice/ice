//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;

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
        public bool IceIsA(string id, IReadOnlyDictionary<string, string>? context = null)
        {
            var request = IceI_IceIsACreateRequest(id, context);
            var response = IceInvoke(request, oneway: false);
            return IceI_IsAReadResponse(response);
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
                                      IReadOnlyDictionary<string, string>? context = null,
                                      IProgress<bool>? progress = null,
                                      CancellationToken cancel = default)
        {
            var request = IceI_IceIsACreateRequest(id, context);
            var task = IceInvokeAsync(request, oneway: false, progress, cancel);
            return ReadResponseAsync(task);

            static async Task<bool> ReadResponseAsync(Task<IncomingResponseFrame> task)
                => IceI_IsAReadResponse(await task.ConfigureAwait(false));
        }

        private OutgoingRequestFrame IceI_IceIsACreateRequest(string id, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithParameters(this, "ice_isA", idempotent: true, null, context,
                id, OutputStream.IceWriterFromString);
            return AwaitResponseAsync(IceInvokeAsync(request, oneway: false, progress, cancel, synchronous));

        private static bool IceI_IsAReadResponse(IncomingResponseFrame incomingResponseFrame)
        {
            var inputStream = incomingResponseFrame.ReadReturnValue();
            bool returnValue = inputStream.ReadBool();
            inputStream.EndEncapsulation(); // TODO: need a better name, like maybe Done()
            return returnValue;
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(IReadOnlyDictionary<string, string>? context = null)
        {
            var request = IceI_IcePingCreateRequest(context);
            var response = IceInvoke(request, IsOneway);

            if (!IsOneway)
            {
                IceI_IcePingReadResponse(response);
            }
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task IcePingAsync(IReadOnlyDictionary<string, string>? context = null,
                                 IProgress<bool>? progress = null,
                                 CancellationToken cancel = default)
        {
            var request = OutgoingRequestFrame.WithNoParameter(this, "ice_ping", idempotent: true, context);
            var task = IceInvokeAsync(request, oneway: IsOneway, progress, cancel, synchronous);
            return IsOneway ? task : AwaitResponseAsync(task);

            static async Task ReadResponseAsync(Task<IncomingResponseFrame> task)
                => IceI_IcePingReadResponse(await task.ConfigureAwait(false));
        }

        private OutgoingRequestFrame IceI_IcePingCreateRequest(IReadOnlyDictionary<string, string>? context)
            => OutgoingRequestFrame.Empty(this, "ice_ping", idempotent: true, context);

        private static void IceI_IcePingReadResponse(IncomingResponseFrame incomingResponseFrame)
            => incomingResponseFrame.ReadVoidReturnValue();

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(IReadOnlyDictionary<string, string>? context = null)
        {
            var request = IceI_IceIdsCreateRequest(context);
            var response = IceInvoke(request, oneway: false);
            return IceI_IceIdsReadResponse(response);
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]> IceIdsAsync(IReadOnlyDictionary<string, string>? context = null,
                                          IProgress<bool>? progress = null,
                                          CancellationToken cancel = default)
        {
            var request = OutgoingRequestFrame.WithNoParameter(this, "ice_ids", idempotent: true, context);
            return AwaitResponseAsync(IceInvokeAsync(request, oneway: false, progress, cancel, synchronous));

            static async Task<string[]> ReadResponseAsync(Task<IncomingResponseFrame> task)
                => IceI_IceIdsReadResponse(await task.ConfigureAwait(false));
        }

        private OutgoingRequestFrame IceI_IceIdsCreateRequest(IReadOnlyDictionary<string, string>? context)
            => OutgoingRequestFrame.Empty(this, "ice_ids", idempotent: true, context);

        private static string[] IceI_IceIdsReadResponse(IncomingResponseFrame incomingResponsFrame)
        {
            InputStream inputStream = incomingResponsFrame.ReadReturnValue();
            string[] returnValue = inputStream.ReadStringArray();
            inputStream.EndEncapsulation(); // TODO: need a better name, like maybe Done()
            return returnValue;
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(IReadOnlyDictionary<string, string>? context = null)
        {
            var request = IceI_IceIdCreateRequest(context);
            var response = IceInvoke(request, oneway: false);
            return IceI_IceIdReadResponse(response);
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> IceIdAsync(IReadOnlyDictionary<string, string>? context = null,
                                       IProgress<bool>? progress = null,
                                       CancellationToken cancel = new CancellationToken())
        {
            var request = OutgoingRequestFrame.WithNoParameter(this, "ice_id", idempotent: true, context);
            return AwaitResponseAsync(IceInvokeAsync(request, oneway: false, progress, cancel, synchronous));

            static async Task<string> ReadResponseAsync(Task<IncomingResponseFrame> task)
                => IceI_IceIdReadResponse(await task.ConfigureAwait(false));
        }

        private OutgoingRequestFrame IceI_IceIdCreateRequest(IReadOnlyDictionary<string, string>? context)
            => OutgoingRequestFrame.Empty(this, "ice_id", idempotent: true, context);

        private static string IceI_IceIdReadResponse(IncomingResponseFrame response)
        {
            InputStream inputStream = response.ReadReturnValue();
            string returnValue = inputStream.ReadString();
            inputStream.EndEncapsulation();
            return returnValue;
        }

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        public Identity Identity => IceReference.GetIdentity();

        /// <summary>Returns the context of this proxy.</summary>
        public IReadOnlyDictionary<string, string> Context => IceReference.GetContext();

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
        public Encoding Encoding => IceReference.GetEncoding();

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
        protected internal IncomingResponseFrame IceInvoke(OutgoingRequestFrame request, bool oneway)
        {
            try
            {
                var completed = new InvokeTaskCompletionCallback(null, default);
                new OutgoingAsync(this, completed, request, oneway: oneway).Invoke(request.Operation, request.Context,
                                                                                   synchronous: true);
                return completed.Task.Result;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }

        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected internal Task<IncomingResponseFrame> IceInvokeAsync(OutgoingRequestFrame request,
                                                                      bool oneway,
                                                                      IProgress<bool>? progress,
                                                                      CancellationToken cancel)
        {
            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(this, completed, request, oneway: oneway).Invoke(request.Operation, request.Context,
                                                                               synchronous: false);
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
        public int IceHandleException(System.Exception ex, IRequestHandler? handler, bool idempotent, bool sent,
                                      ref int cnt)
        {
            IceUpdateRequestHandler(handler, null); // Clear the request handler

            // We only retry after failing with a RequestFailedException or a local exception.
            //
            // A CloseConnectionException indicates graceful server shutdown, and is therefore always repeatable without
            // violating "at-most-once". That's because by sending a close connection message, the server guarantees
            // that all outstanding requests can safely be repeated.
            //
            // An ObjectNotExistException can always be retried as well without violating "at-most-once" (see the
            // implementation of the checkRetryAfterException method of the ProxyFactory class for the reasons why it
            // can be useful).
            //
            // If the request was not sent or the operation is idempotent it can also always be retried if the retry
            // count isn't reached.
            //
            // TODO: revisit retry logic
            if (ex is ObjectNotExistException || ex is CloseConnectionException || !sent || idempotent)
            {
                try
                {
                    return IceReference.GetCommunicator().CheckRetryAfterException(ex, IceReference, ref cnt);
                }
                catch (CommunicatorDestroyedException)
                {
                    // The communicator is already destroyed, so we cannot retry.
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

        // Temporary helper class for IceInvokeAsync
        private class InvokeTaskCompletionCallback : TaskCompletionCallback<IncomingResponseFrame>
        {
            internal InvokeTaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken) :
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
    }

    // The base class for all proxies. It's a publically visible Ice-internal class. Applications should
    // not use it directly.
    [Serializable]
    [EditorBrowsable(EditorBrowsableState.Never)]
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
                throw new ArgumentException("cannot deserialize proxy: Ice.Communicator not found in StreamingContext",
                    nameof(context));
            }
            IceReference = communicator.CreateReference(info.GetString("proxy"), null);
        }

        public virtual void GetObjectData(SerializationInfo info, StreamingContext context) =>
            info.AddValue("proxy", ToString());

        /// <summary>Returns the stringified form of this proxy.</summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString()
        {
            Debug.Assert(IceReference != null);
            return IceReference.ToString();
        }

        /// <summary>Returns a hash code for this proxy.</summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode() => IceReference.GetHashCode();

        /// <summary>Returns whether this proxy equals the given object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public override bool Equals(object? other) => Equals(other as IObjectPrx);

        /// <summary>Returns whether this proxy equals the given proxy. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The proxy to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public bool Equals(IObjectPrx? other) => other != null && IceReference.Equals(other.IceReference);
    }
}
