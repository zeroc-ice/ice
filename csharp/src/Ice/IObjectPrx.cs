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
        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly InputStreamReader<IObjectPrx?> IceReader = (istr) => istr.ReadProxy(Factory);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public static readonly OutputStreamWriter<IObjectPrx?> IceWriter = (ostr, value) => ostr.WriteProxy(value);

        [EditorBrowsable(EditorBrowsableState.Never)]
        public Reference IceReference { get; }

        // IceClone is re-implemented by all generated proxy classes
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected IObjectPrx IceClone(Reference reference) => new ObjectPrx(reference);
        internal IObjectPrx Clone(Reference reference) => IceClone(reference);

        /// <summary>
        /// Returns the communicator that created this proxy.
        /// </summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator Communicator => IceReference.Communicator;

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
        public bool IceIsA(string id, IReadOnlyDictionary<string, string>? context = null) =>
            IceI_IsARequest.Invoke(this, id, context);

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
                                      CancellationToken cancel = default) =>
            IceI_IsARequest.InvokeAsync(this, id, context, progress, cancel);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(IReadOnlyDictionary<string, string>? context = null) =>
            IceI_PingRequest.Invoke(this, context);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task IcePingAsync(IReadOnlyDictionary<string, string>? context = null,
                                 IProgress<bool>? progress = null,
                                 CancellationToken cancel = default) =>
            IceI_PingRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(IReadOnlyDictionary<string, string>? context = null) =>
            IceI_IdsRequest.Invoke(this, context);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string[]> IceIdsAsync(IReadOnlyDictionary<string, string>? context = null,
                                          IProgress<bool>? progress = null,
                                          CancellationToken cancel = default) =>
            IceI_IdsRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(IReadOnlyDictionary<string, string>? context = null) =>
            IceI_IdRequest.Invoke(this, context);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public Task<string> IceIdAsync(IReadOnlyDictionary<string, string>? context = null,
                                       IProgress<bool>? progress = null,
                                       CancellationToken cancel = default) =>
            IceI_IdRequest.InvokeAsync(this, context, progress, cancel);

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity => IceReference.Identity;

        /// <summary>The context of this proxy, which will be sent with each invocation made using this proxy.</summary>
        public IReadOnlyDictionary<string, string> Context => IceReference.Context;

        /// <summary>The facet to use on the target Ice object. The empty string corresponds to the default facet.
        /// </summary>
        public string Facet => IceReference.Facet;

        /// <summary>The adapter ID of this proxy, or an empty string if this proxy does not have an adapter ID.
        /// A proxy with an adapter ID is an indirect proxy. </summary>
        public string AdapterId => IceReference.AdapterId;

        /// <summary>The endpoints of this proxy. A proxy with a non-empty endpoint list is a direct proxy.</summary>
        public Endpoint[] Endpoints => (Endpoint[])IceReference.Endpoints.Clone();

        /// <summary>The locator cache timeout of this proxy, in seconds.</summary>
        public int LocatorCacheTimeout => IceReference.LocatorCacheTimeout;

        /// <summary>The invocation timeout of this proxy, in seconds.</summary>
        public int InvocationTimeout => IceReference.InvocationTimeout;

        /// <summary>Indicates whether or not this proxy caches its connection.</summary>
        /// <value>True when the proxy caches its connection; otherwise, false.</value>
        public bool IsConnectionCached => IceReference.IsConnectionCached;

        /// <summary>The endpoint selection policy of this proxy, which can be Random or Ordered.</summary>
        public EndpointSelectionType EndpointSelection => IceReference.EndpointSelection;

        /// <summary>The encoding used to marshal request parameters.</summary>
        public Encoding Encoding => IceReference.Encoding;

        /// <summary>Indicates whether or not this proxy prefers non-secure connections.</summary>
        /// <value>When true, the proxy attempts to establish a non-secure connection if such a connection is available;
        /// when false, the proxy attempts first to establish a secure connection and falls back to a non-secure
        /// connection. The fallback to a non-secure connection applies only when using the ice1 protocol.</value>
        public bool PreferNonSecure => IceReference.PreferNonSecure;

        /// <summary>The router associated with this proxy. This property is null when no router is associated with this
        /// proxy.</summary>
        public IRouterPrx? Router => IceReference.RouterInfo?.Router;

        /// <summary>The locator associated with this proxy. This property is null when no locator is associated with
        /// this proxy.</summary>
        public ILocatorPrx? Locator => IceReference.LocatorInfo?.Locator;

        /// <summary>Indicates whether this proxy uses collocation optimization.</summary>
        /// <value>When true, the proxy uses collocation optimization; otherwise, the value is false.</value>
        public bool IsCollocationOptimized => IceReference.IsCollocationOptimized;

        /// <summary>Indicates whether or not using this proxy to invoke an operation that does not return anything
        /// waits for an empty response from the target Ice object.</summary>
        /// <value>When true, invoking such an operation does not wait for the response from the target object. This
        /// corresponds to several <see cref="InvocationMode"/> enumerators, such as Oneway and Datagram. When false,
        /// invoking such an operation waits for the empty response from the target object, unless this behavior is
        /// overridden by metadata on the Slice operation's definition.</value>
        public bool IsOneway => IceReference.InvocationMode != InvocationMode.Twoway;

        /// <summary>The invocation mode of this proxy. </summary>
        public InvocationMode InvocationMode => IceReference.InvocationMode;

        /// <summary>The compression override setting of this proxy.</summary>
        /// <value>When null, no override is set and the compress setting of the endpoint(s) is used. When true,
        /// override the compress setting of all endpoints to enable compression. When false, override the compress
        /// setting of all endpoint to disable compression.</value>
        public bool? Compress => IceReference.Compress;

        /// <summary>The connection timeout override setting of this proxy, in milliseconds.</summary>
        /// <value>When null, no override is set and the timeout setting of the endpoint(s) is used. When non-null,
        /// overrides the timeout setting of all endpoints.</value>
        public int? ConnectionTimeout => IceReference.ConnectionTimeout;

        /// <summary>The connection ID of this proxy.</summary>
        public string ConnectionId => IceReference.ConnectionId;

        /// <summary>Indicates whether or not this proxy is fixed proxy.</summary>
        /// <value>When true, the proxy is a fixed proxy; when false, the proxy is a regular proxy, not bound to
        /// any connection.</value>
        public bool IsFixed => IceReference.IsFixed;

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceWrite(OutputStream ostr) => IceReference.Write(ostr);

        public TaskScheduler Scheduler => IceReference.ThreadPool;

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
            catch (Exception)
            {
                prx = null;
                return false;
            }
            return true;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public int IceHandleException(Exception ex, IRequestHandler? handler, bool idempotent, bool sent,
                                      ref int cnt)
        {
            if (!IceReference.IsFixed)
            {
                IceReference.UpdateRequestHandler(handler, null); // Clear the request handler
            }

            // We only retry after failing with a DispatchException or a local exception.
            //
            // A ConnectionClosedByPeerException indicates graceful server shutdown, and is therefore always repeatable
            // without violating "at-most-once". That's because by sending a close connection message, the server
            // guarantees that all outstanding requests can safely be repeated.
            //
            // An ObjectNotExistException can always be retried as well without violating "at-most-once" (see the
            // implementation of the checkRetryAfterException method of the ProxyFactory class for the reasons why it
            // can be useful).
            //
            // If the request was not sent or the operation is idempotent it can also always be retried if the retry
            // count isn't reached.
            //
            // TODO: revisit retry logic
            if (ex is ObjectNotExistException || ex is ConnectionClosedByPeerException || !sent || idempotent)
            {
                try
                {
                    return IceReference.Communicator.CheckRetryAfterException(ex, IceReference, ref cnt);
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

        // Temporary helper class for IceInvokeAsync
        internal class InvokeTaskCompletionCallback : TaskCompletionCallback<IncomingResponseFrame>
        {
            internal InvokeTaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken)
                : base(progress, cancellationToken)
            {
            }

            public override void HandleInvokeSent(bool sentSynchronously, bool done, bool alreadySent,
                OutgoingAsyncBase outgoing)
            {
                if (Progress != null && !alreadySent)
                {
                    Progress.Report(sentSynchronously);
                }

                if (done)
                {
                    IncomingResponseFrame? response = ((ProxyOutgoingAsyncBase)outgoing).ResponseFrame;
                    Debug.Assert(response != null);
                    SetResult(response);
                }
            }

            public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase outgoing) =>
                SetResult(((ProxyOutgoingAsyncBase)outgoing).ResponseFrame!);
        }

        private static OutgoingRequestWithParam<string, bool>? _iceI_IsARequest;
        private static OutgoingRequestWithParam<string, bool> IceI_IsARequest =>
            _iceI_IsARequest ??= new OutgoingRequestWithParam<string, bool>("ice_isA", idempotent: true,
                format: null,
                writer: OutputStream.IceWriterFromString,
                reader: InputStream.IceReaderIntoBool);

        private static OutgoingRequestWithEmptyParamList<string[]>? _iceI_IdsRequest;
        private static OutgoingRequestWithEmptyParamList<string[]> IceI_IdsRequest =>
            _iceI_IdsRequest ??= new OutgoingRequestWithEmptyParamList<string[]>("ice_ids", idempotent: true,
                reader: istr => istr.ReadStringArray());

        private static OutgoingRequestWithEmptyParamList<string>? _iceI_IdRequest;
        private static OutgoingRequestWithEmptyParamList<string> IceI_IdRequest =>
            _iceI_IdRequest ??= new OutgoingRequestWithEmptyParamList<string>("ice_id", idempotent: true,
                reader: InputStream.IceReaderIntoString);

        private static OutgoingRequestWithEmptyParamList? _iceI_PingRequest;
        private static OutgoingRequestWithEmptyParamList IceI_PingRequest =>
            _iceI_PingRequest ??= new OutgoingRequestWithEmptyParamList("ice_ping", idempotent: true);
    }

    // The base class for all proxies. It's a publicly visible Ice-internal class. Applications should not use it
    // directly.
    [Serializable]
    [EditorBrowsable(EditorBrowsableState.Never)]
    public class ObjectPrx : IObjectPrx, ISerializable
    {
        public Reference IceReference { get; }

        public virtual void GetObjectData(SerializationInfo info, StreamingContext context) =>
            info.AddValue("proxy", ToString());

        /// <summary>Returns the stringified form of this proxy.</summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString() => IceReference.ToString();

        /// <summary>Returns a hash code for this proxy.</summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode() => IceReference.GetHashCode();

        /// <summary>Returns whether this proxy equals the given object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public override bool Equals(object? other) => Equals(other as IObjectPrx);

        // Implements IEquatable<IObjectPrx>:
        /// <summary>Returns whether this proxy equals the given proxy. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.</summary>
        /// <param name="other">The proxy to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to other; otherwise, false.</returns>
        public bool Equals(IObjectPrx? other) => other != null && IceReference.Equals(other.IceReference);

        protected internal ObjectPrx(Reference reference) => IceReference = reference;

        protected ObjectPrx(SerializationInfo info, StreamingContext context)
        {
            if (!(context.Context is Communicator communicator))
            {
                throw new ArgumentException("cannot deserialize proxy: Ice.Communicator not found in StreamingContext",
                    nameof(context));
            }
            string? str = info.GetString("proxy");
            Debug.Assert(str != null);
            IceReference = communicator.CreateReference(str, null);
        }
    }
}
