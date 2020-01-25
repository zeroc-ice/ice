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
        public IRequestHandler? RequestHandler { get; set; }
        public LinkedList<StreamCacheEntry>? StreamCache { get; set; }

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

        /// <summary>
        /// Tests whether the target object implements a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>True if the target object implements the Slice interface identified by id</return>.
        public bool IceIsA(string id, Dictionary<string, string>? context = null)
        {
            try
            {
                return IceI_ice_isAAsync(id, context, null, CancellationToken.None, true).Result;
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
                                      CancellationToken cancel = new CancellationToken()) =>
            IceI_ice_isAAsync(id, context, progress, cancel, false);

        private Task<bool>
        IceI_ice_isAAsync(string id, Dictionary<string, string>? context, IProgress<bool>? progress, CancellationToken cancel,
                          bool synchronous)
        {
            IceCheckTwowayOnly("ice_isA");
            var completed = new OperationTaskCompletionCallback<bool>(progress, cancel);
            IceI_ice_isA(id, context, completed, synchronous);
            return completed.Task;
        }

        private void IceI_ice_isA(string id,
                                  Dictionary<string, string>? context,
                                  IOutgoingAsyncCompletionCallback completed,
                                  bool synchronous)
        {
            IceCheckAsyncTwowayOnly("ice_isA");
            GetOutgoingAsync<bool>(completed).Invoke("ice_isA",
                                                     OperationMode.Nonmutating,
                                                     null,
                                                     context,
                                                     synchronous,
                                                     (OutputStream os) => os.WriteString(id),
                                                     null,
                                                     (InputStream iss) => iss.ReadBool());
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(Dictionary<string, string>? context = null)
        {
            try
            {
                IceI_IcePingAsync(context, null, CancellationToken.None, true).Wait();
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
                                 CancellationToken cancel = new CancellationToken()) => IceI_IcePingAsync(context, progress, cancel, false);

        private Task
        IceI_IcePingAsync(Dictionary<string, string>? context, IProgress<bool>? progress, CancellationToken cancel, bool synchronous)
        {
            var completed = new OperationTaskCompletionCallback<object>(progress, cancel);
            IceI_IcePing(context, completed, synchronous);
            return completed.Task;
        }

        private void IceI_IcePing(Dictionary<string, string>? context, IOutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            GetOutgoingAsync<object>(completed).Invoke("ice_ping",
                                                       OperationMode.Nonmutating,
                                                       null,
                                                       context,
                                                       synchronous);
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
                return IceI_ice_idsAsync(context, null, CancellationToken.None, true).Result;
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
        public Task<string[]>
        IceIdsAsync(Dictionary<string, string>? context = null,
                    IProgress<bool>? progress = null,
                    CancellationToken cancel = new CancellationToken()) => IceI_ice_idsAsync(context, progress, cancel, false);

        private Task<string[]> IceI_ice_idsAsync(Dictionary<string, string>? context,
                                                 IProgress<bool>? progress,
                                                 CancellationToken cancel,
                                                 bool synchronous)
        {
            IceCheckTwowayOnly("ice_ids");
            var completed = new OperationTaskCompletionCallback<string[]>(progress, cancel);
            IceI_ice_ids(context, completed, synchronous);
            return completed.Task;
        }

        private void IceI_ice_ids(Dictionary<string, string>? context, IOutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            IceCheckAsyncTwowayOnly("ice_ids");
            GetOutgoingAsync<string[]>(completed).Invoke("ice_ids",
                                                         OperationMode.Nonmutating,
                                                         null,
                                                         context,
                                                         synchronous,
                                                         read: (InputStream iss) => iss.ReadStringSeq());
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Dictionary<string, string>? context = null)
        {
            try
            {
                return IceI_ice_idAsync(context, null, CancellationToken.None, true).Result;
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
            IceI_ice_idAsync(context, progress, cancel, false);

        private Task<string>
        IceI_ice_idAsync(Dictionary<string, string>? context, IProgress<bool>? progress, CancellationToken cancel, bool synchronous)
        {
            IceCheckTwowayOnly("ice_id");
            var completed = new OperationTaskCompletionCallback<string>(progress, cancel);
            IceI_ice_id(context, completed, synchronous);
            return completed.Task;
        }

        private void IceI_ice_id(Dictionary<string, string>? context,
                                 IOutgoingAsyncCompletionCallback completed,
                                 bool synchronous)
        {
            GetOutgoingAsync<string>(completed).Invoke("ice_id",
                                                       OperationMode.Nonmutating,
                                                       null,
                                                       context,
                                                       synchronous,
                                                       read: (InputStream iss) => iss.ReadString());
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

        /// <summary>
        /// Returns whether this proxy uses twoway invocations.
        /// </summary>
        /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
        public bool IsTwoway => IceReference.GetMode() == InvocationMode.Twoway;

        /// <summary>
        /// Returns whether this proxy uses oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
        public bool IsOneway => IceReference.GetMode() == InvocationMode.Oneway;

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

        public static ProxyFactory<IObjectPrx> Factory = (reference) => new ObjectPrx(reference);

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
        public void IceCheckTwowayOnly(string name)
        {
            //
            // No mutex lock necessary, there is nothing mutable in this
            // operation.
            //

            if (!IsTwoway)
            {
                throw new TwowayOnlyException(name);
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceCheckAsyncTwowayOnly(string name)
        {
            //
            // No mutex lock necessary, there is nothing mutable in this
            // operation.
            //

            if (!IsTwoway)
            {
                throw new ArgumentException("`" + name + "' can only be called with a twoway proxy");
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public int IceHandleException(Exception ex, IRequestHandler? handler, OperationMode mode, bool sent,
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
                                        mode == OperationMode.Nonmutating || mode == OperationMode.Idempotent ||
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

        protected OutgoingAsyncT<T>
        GetOutgoingAsync<T>(IOutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream? iss = null;
            OutputStream? os = null;

            if (IceReference.GetCommunicator().CacheMessageBuffers > 0)
            {
                lock (this)
                {
                    if (StreamCache != null && StreamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = StreamCache.First.Value.Iss;
                        os = StreamCache.First.Value.Os;

                        StreamCache.RemoveFirst();
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

        internal InvokeOutgoingAsyncT
        GetInvokeOutgoingAsync(IOutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream? iss = null;
            OutputStream? os = null;

            if (IceReference.GetCommunicator().CacheMessageBuffers > 0)
            {
                lock (this)
                {
                    if (StreamCache != null && StreamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = StreamCache.First.Value.Iss;
                        os = StreamCache.First.Value.Os;

                        StreamCache.RemoveFirst();
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

        /// <summary>
        /// Only for internal use by OutgoingAsync
        /// </summary>
        /// <param name="iss"></param>
        /// <param name="os"></param>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public void
        CacheMessageBuffers(InputStream? iss, OutputStream os)
        {
            lock (this)
            {
                if (StreamCache == null)
                {
                    StreamCache = new LinkedList<StreamCacheEntry>();
                }
                StreamCacheEntry cacheEntry;
                cacheEntry.Iss = iss;
                cacheEntry.Os = os;
                StreamCache.AddLast(cacheEntry);
            }
        }

        public struct StreamCacheEntry
        {
            public InputStream? Iss;
            public OutputStream Os;
        }
    }

    /// <summary>
    /// Represent the result of the ice_invokeAsync operation
    /// </summary>
    public struct Object_Ice_invokeResult
    {
        public Object_Ice_invokeResult(bool returnValue, byte[]? outEncaps)
        {
            ReturnValue = returnValue;
            OutEncaps = outEncaps;
        }

        /// <summary>
        /// If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception.
        /// </summary>
        public bool ReturnValue;

        /// <summary>
        /// The encoded out-paramaters and return value for the operation.
        /// The return value follows any out-parameters. If returnValue is
        /// false it contains the encoded user exception.
        /// </summary>
        public byte[]? OutEncaps;
    };

    internal class InvokeOutgoingAsyncT : OutgoingAsync
    {
        public InvokeOutgoingAsyncT(IObjectPrx prx,
                                    IOutgoingAsyncCompletionCallback completionCallback,
                                    OutputStream? os = null,
                                    InputStream? iss = null) : base(prx, completionCallback, os, iss)
        {
        }

        public void Invoke(string operation, OperationMode mode, byte[] inParams,
                           Dictionary<string, string>? context, bool synchronous)
        {
            try
            {
                Debug.Assert(Os != null);
                Prepare(operation, mode, context);
                if (inParams == null || inParams.Length == 0)
                {
                    Os.WriteEmptyEncapsulation(Encoding);
                }
                else
                {
                    Os.WriteEncapsulation(inParams);
                }
                Invoke(operation, synchronous);
            }
            catch (Exception ex)
            {
                Abort(ex);
            }
        }

        public Object_Ice_invokeResult
        GetResult(bool ok)
        {
            try
            {
                var ret = new Object_Ice_invokeResult();
                if (Proxy.IceReference.GetMode() == InvocationMode.Twoway)
                {
                    ret.OutEncaps = Is!.ReadEncapsulation(out EncodingVersion _);
                }
                else
                {
                    ret.OutEncaps = null;
                }
                ret.ReturnValue = ok;
                return ret;
            }
            finally
            {
                CacheMessageBuffers();
            }
        }
    }

    internal class InvokeTaskCompletionCallback : TaskCompletionCallback<Object_Ice_invokeResult>
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
                SetResult(new Object_Ice_invokeResult(true, null));
            }
        }

        public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) =>
            SetResult(((InvokeOutgoingAsyncT)og).GetResult(ok));
    }

    /// <summary>
    /// Base class of all object proxies.
    /// </summary>
    [Serializable]
    public class ObjectPrx : IObjectPrx, ISerializable
    {
        public Reference IceReference { get; private set; }
        public IRequestHandler? RequestHandler { get; set; }
        public LinkedList<IObjectPrx.StreamCacheEntry>? StreamCache { get; set; }

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
            public GetConnectionTaskCompletionCallback(IObjectPrx proxy,
                                                       IProgress<bool>? progress = null,
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
                var completed = new GetConnectionTaskCompletionCallback(prx);
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
            var completed = new GetConnectionTaskCompletionCallback(prx, progress, cancel);
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

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="prx">The proxy to invoke the operation.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="outEncaps">The encoded out-paramaters and return value
        /// for the operation. The return value follows any out-parameters.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception. If the operation raises a run-time exception,
        /// it throws it directly.</returns>
        public static bool Invoke(this IObjectPrx prx,
                                  string operation,
                                  OperationMode mode,
                                  byte[] inEncaps,
                                  out byte[]? outEncaps,
                                  Dictionary<string, string>? context = null)
        {
            try
            {
                Object_Ice_invokeResult result = prx.IceI_ice_invokeAsync(operation, mode, inEncaps, context, null, CancellationToken.None, true).Result;
                outEncaps = result.OutEncaps;
                return result.ReturnValue;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
        /// <param name="prx">The proxy to invoke the operation.</param>
        /// <param name="operation">The name of the operation to invoke.</param>
        /// <param name="mode">The operation mode (normal or idempotent).</param>
        /// <param name="inEncaps">The encoded in-parameters for the operation.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The task object representing the asynchronous operation.</returns>
        public static Task<Object_Ice_invokeResult>
        InvokeAsync(this IObjectPrx prx,
                    string operation,
                    OperationMode mode,
                    byte[] inEncaps,
                    Dictionary<string, string>? context = null,
                    IProgress<bool>? progress = null,
                    CancellationToken cancel = new CancellationToken()) =>
            prx.IceI_ice_invokeAsync(operation, mode, inEncaps, context, progress, cancel, false);

        private static Task<Object_Ice_invokeResult>
        IceI_ice_invokeAsync(this IObjectPrx prx,
                             string operation,
                             OperationMode mode,
                             byte[] inEncaps,
                             Dictionary<string, string>? context,
                             IProgress<bool>? progress,
                             CancellationToken cancel,
                             bool synchronous)
        {
            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            prx.IceI_ice_invoke(operation, mode, inEncaps, context, completed, synchronous);
            return completed.Task;
        }

        private static void IceI_ice_invoke(this IObjectPrx prx,
                                     string operation,
                                     OperationMode mode,
                                     byte[] inEncaps,
                                     Dictionary<string, string>? context,
                                     IOutgoingAsyncCompletionCallback completed,
                                     bool synchronous) =>
            prx.GetInvokeOutgoingAsync(completed).Invoke(operation, mode, inEncaps, context, synchronous);
    }
}
