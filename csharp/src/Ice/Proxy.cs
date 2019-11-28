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
    /// Value type to allow differenciate between a context that is explicity set to
    /// empty (empty or null dictionary) and a context that has non been set.
    /// </summary>
    public struct OptionalContext
    {
        private OptionalContext(Dictionary<string, string> ctx)
        {
            _ctx = ctx == null ? _emptyContext : ctx;
        }

        /// <summary>
        /// Implicit conversion between Dictionary&lt;string, string&gt; and
        /// OptionalContext.
        /// </summary>
        /// <param name="ctx">Dictionary to convert.</param>
        /// <returns>OptionalContext value representing the dictionary</returns>
        public static implicit operator OptionalContext(Dictionary<string, string> ctx)
        {
            return new OptionalContext(ctx);
        }

        /// <summary>
        /// Implicit conversion between OptionalContext and
        /// Dictionary&lt;string, string&gt;
        /// </summary>
        /// <param name="value">OptionalContext value to convert</param>
        /// <returns>The Dictionary object.</returns>
        public static implicit operator Dictionary<string, string>(OptionalContext value)
        {
            return value._ctx;
        }

        private Dictionary<string, string> _ctx;
        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
    }

    /// <summary>
    /// Base interface of all object proxies.
    /// </summary>
    public interface IObjectPrx : ISerializable, ICloneable
    {
        public Reference IceReference { get; }
        public RequestHandler? RequestHandler { get; set; }
        public LinkedList<StreamCacheEntry>? StreamCache { get; set; }

        public IObjectPrx Clone(Reference reference);

        /// <summary>
        /// Returns the communicator that created this proxy.
        /// </summary>
        /// <returns>The communicator that created this proxy.</returns>
        public Communicator Communicator
        {
            get
            {
                return IceReference.getCommunicator();
            }
        }

        /// <summary>
        /// Tests whether this object supports a specific Slice interface.
        /// </summary>
        /// <param name="id">The type ID of the Slice interface to test against.</param>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>True if the target object has the interface specified by id or derives
        /// from the interface specified by id.</returns>
        public bool IceIsA(string id, OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_isAAsync(id, context, null, CancellationToken.None, true).Result;
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
                                      OptionalContext context = new OptionalContext(),
                                      IProgress<bool>? progress = null,
                                      CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_isAAsync(id, context, progress, cancel, false);
        }

        private Task<bool>
        iceI_ice_isAAsync(string id, OptionalContext context, IProgress<bool>? progress, CancellationToken cancel,
                          bool synchronous)
        {
            iceCheckTwowayOnly("ice_isA");
            var completed = new OperationTaskCompletionCallback<bool>(progress, cancel);
            iceI_ice_isA(id, context, completed, synchronous);
            return completed.Task;
        }

        private void iceI_ice_isA(string id,
                                  Dictionary<string, string> context,
                                  OutgoingAsyncCompletionCallback completed,
                                  bool synchronous)
        {
            iceCheckAsyncTwowayOnly("ice_isA");
            getOutgoingAsync<bool>(completed).invoke("ice_isA",
                                                     OperationMode.Nonmutating,
                                                     FormatType.DefaultFormat,
                                                     context,
                                                     synchronous,
                                                     (OutputStream os) => { os.writeString(id); },
                                                     null,
                                                     (InputStream iss) => { return iss.readBool(); });
        }

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(OptionalContext context = new OptionalContext())
        {
            try
            {
                iceI_IcePingAsync(context, null, CancellationToken.None, true).Wait();
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
        public Task IcePingAsync(OptionalContext context = new OptionalContext(),
                                 IProgress<bool>? progress = null,
                                 CancellationToken cancel = new CancellationToken())
        {
            return iceI_IcePingAsync(context, progress, cancel, false);
        }

        private Task
        iceI_IcePingAsync(OptionalContext context, IProgress<bool>? progress, CancellationToken cancel, bool synchronous)
        {
            var completed = new OperationTaskCompletionCallback<object>(progress, cancel);
            iceI_IcePing(context, completed, synchronous);
            return completed.Task;
        }

        private void iceI_IcePing(Dictionary<string, string> context, OutgoingAsyncCompletionCallback completed,
                                       bool synchronous)
        {
            getOutgoingAsync<object>(completed).invoke("ice_ping",
                                                       OperationMode.Nonmutating,
                                                       FormatType.DefaultFormat,
                                                       context,
                                                       synchronous);
        }

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_idsAsync(context, null, CancellationToken.None, true).Result;
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
        IceIdsAsync(OptionalContext context = new OptionalContext(),
                    IProgress<bool>? progress = null,
                    CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_idsAsync(context, progress, cancel, false);
        }

        private Task<string[]> iceI_ice_idsAsync(OptionalContext context, IProgress<bool>? progress,
                                                 CancellationToken cancel,
                                                 bool synchronous)
        {
            iceCheckTwowayOnly("ice_ids");
            var completed = new OperationTaskCompletionCallback<string[]>(progress, cancel);
            iceI_ice_ids(context, completed, synchronous);
            return completed.Task;
        }

        private void iceI_ice_ids(Dictionary<string, string> context, OutgoingAsyncCompletionCallback completed,
                                  bool synchronous)
        {
            iceCheckAsyncTwowayOnly("ice_ids");
            getOutgoingAsync<string[]>(completed).invoke("ice_ids",
                                                         OperationMode.Nonmutating,
                                                         FormatType.DefaultFormat,
                                                         context,
                                                         synchronous,
                                                         read: (InputStream iss) => { return iss.readStringSeq(); });
        }

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(OptionalContext context = new OptionalContext())
        {
            try
            {
                return iceI_ice_idAsync(context, null, CancellationToken.None, true).Result;
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
        public Task<string> IceIdAsync(OptionalContext context = new OptionalContext(),
                                        IProgress<bool>? progress = null,
                                        CancellationToken cancel = new CancellationToken())
        {
            return iceI_ice_idAsync(context, progress, cancel, false);
        }

        private Task<string>
        iceI_ice_idAsync(OptionalContext context, IProgress<bool>? progress, CancellationToken cancel, bool synchronous)
        {
            iceCheckTwowayOnly("ice_id");
            var completed = new OperationTaskCompletionCallback<string>(progress, cancel);
            iceI_ice_id(context, completed, synchronous);
            return completed.Task;
        }

        private void iceI_ice_id(Dictionary<string, string> context,
                                 OutgoingAsyncCompletionCallback completed,
                                 bool synchronous)
        {
            getOutgoingAsync<string>(completed).invoke("ice_id",
                                                       OperationMode.Nonmutating,
                                                       FormatType.DefaultFormat,
                                                       context,
                                                       synchronous,
                                                       read: (InputStream iss) => { return iss.readString(); });
        }

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        public Identity Identity
        {
            get
            {
                return (Identity)IceReference.getIdentity().Clone();
            }
        }

        /// <summary>
        /// Returns the per-proxy context for this proxy.
        /// </summary>
        /// <returns>The per-proxy context. If the proxy does not have a per-proxy (implicit) context, the return value
        /// is null.</returns>
        public Dictionary<string, string> Context
        {
            get
            {
                return new Dictionary<string, string>(IceReference.getContext());
            }
        }

        /// <summary>
        /// Returns the facet for this proxy.
        /// </summary>
        /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
        /// empty string.</returns>
        public string Facet
        {
            get
            {
                return IceReference.getFacet();
            }
        }

        /// <summary>
        /// Returns the adapter ID for this proxy.
        /// </summary>
        /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
        /// empty string.</returns>
        public string AdapterId
        {
            get
            {
                return IceReference.getAdapterId();
            }
        }

        /// <summary>
        /// Returns the endpoints used by this proxy.
        /// </summary>
        /// <returns>The endpoints used by this proxy.</returns>
        public Endpoint[] Endpoints
        {
            get
            {
                return (Endpoint[])IceReference.getEndpoints().Clone();
            }
        }

        /// <summary>
        /// Returns the locator cache timeout of this proxy.
        /// </summary>
        /// <returns>The locator cache timeout value (in seconds).</returns>
        public int LocatorCacheTimeout
        {
            get
            {
                return IceReference.getLocatorCacheTimeout();
            }
        }

        /// <summary>
        /// Returns the invocation timeout of this proxy.
        /// </summary>
        /// <returns>The invocation timeout value (in seconds).</returns>
        public int InvocationTimeout
        {
            get
            {
                return IceReference.getInvocationTimeout();
            }
        }

        /// <summary>
        /// Returns whether this proxy caches connections.
        /// </summary>
        /// <returns>True if this proxy caches connections; false, otherwise.</returns>
        public bool IsConnectionCached
        {
            get
            {
                return IceReference.getCacheConnection();
            }
        }

        /// <summary>
        /// Returns how this proxy selects endpoints (randomly or ordered).
        /// </summary>
        /// <returns>The endpoint selection policy.</returns>
        public EndpointSelectionType EndpointSelection
        {
            get
            {
                return IceReference.getEndpointSelection();
            }
        }

        /// <summary>
        /// Returns whether this proxy communicates only via secure endpoints.
        /// </summary>
        /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
        public bool IsSecure
        {
            get
            {
                return IceReference.getSecure();
            }
        }

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        public EncodingVersion EncodingVersion
        {
            get
            {
                return IceReference.getEncoding();
            }
        }

        /// <summary>
        /// Returns whether this proxy prefers secure endpoints.
        /// </summary>
        /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
        /// attempts to use insecure endpoints; false, otherwise.</returns>
        public bool IsPreferSecure
        {
            get
            {
                return IceReference.getPreferSecure();
            }
        }

        /// <summary>
        /// Returns the router for this proxy.
        /// </summary>
        /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
        /// is null.</returns>
        public RouterPrx? Router
        {
            get
            {
                RouterInfo ri = IceReference.getRouterInfo();
                return ri != null ? ri.getRouter() : null;
            }
        }

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        public LocatorPrx? Locator
        {
            get
            {
                var li = IceReference.getLocatorInfo();
                return li != null ? li.getLocator() : null;
            }
        }

        /// <summary>
        /// Returns whether this proxy uses collocation optimization.
        /// </summary>
        /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
        public bool IsCollocationOptimized
        {
            get
            {
                return IceReference.getCollocationOptimized();

            }
        }

        /// <summary>
        /// Returns whether this proxy uses twoway invocations.
        /// </summary>
        /// <returns>True if this proxy uses twoway invocations; false, otherwise.</returns>
        public bool IsTwoway
        {
            get
            {
                return IceReference.getMode() == Ice.InvocationMode.Twoway;
            }
        }

        /// <summary>
        /// Returns whether this proxy uses oneway invocations.
        /// </summary>
        /// <returns>True if this proxy uses oneway invocations; false, otherwise.</returns>
        public bool IsOneway
        {
            get
            {
                return IceReference.getMode() == Ice.InvocationMode.Oneway;
            }
        }

        public InvocationMode InvocationMode
        {
            get
            {
                return IceReference.getMode();
            }
        }

        /// <summary>
        /// Obtains the compression override setting of this proxy.
        /// </summary>
        /// <returns>The compression override setting. If no optional value is present, no override is
        /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
        public Ice.Optional<bool> Compress
        {
            get
            {
                return IceReference.getCompress();
            }
        }

        /// <summary>
        /// Obtains the timeout override of this proxy.
        /// </summary>
        /// <returns>The timeout override. If no optional value is present, no override is set. Otherwise,
        /// returns the timeout override value.</returns>
        public Ice.Optional<int> ConnectionTimeout
        {
            get
            {
                return IceReference.getTimeout();
            }
        }

        /// <summary>
        /// Returns the connection id of this proxy.
        /// </summary>
        /// <returns>The connection id.</returns>
        public string ConnectionId
        {
            get
            {
                return IceReference.getConnectionId();
            }
        }

        /// <summary>
        /// Returns whether this proxy is a fixed proxy.
        /// </summary>
        /// <returns>True if this is a fixed proxy, false otherwise.
        /// </returns>
        public bool IsFixed
        {
            get
            {
                return IceReference is IceInternal.FixedReference;
            }
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void IceWrite(OutputStream os)
        {
            IceReference.getIdentity().ice_writeMembers(os);
            IceReference.streamWrite(os);
        }

        public TaskScheduler Scheduler
        {
            get
            {
                return IceReference.getThreadPool();
            }
        }

        public static ProxyFactory<IObjectPrx> Factory = (reference) => new ObjectPrx(reference);

        public static IObjectPrx Parse(string s, Communicator communicator)
        {
            return new ObjectPrx(communicator.StringToReference(s));
        }

        public static bool TryParse(string s, Communicator communicator, out IObjectPrx? prx)
        {
            try
            {
                prx = new ObjectPrx(communicator.StringToReference(s));
            }
            catch (System.Exception)
            {
                prx = null;
                return false;
            }
            return true;
        }

        public static IObjectPrx ParseProperty(string prefix, Communicator communicator)
        {
            string proxy = communicator.getProperties().getProperty(prefix);
            return new ObjectPrx(communicator.StringToReference(proxy, prefix));
        }

        public static bool TryParseProperty(string prefix, Communicator communicator, out IObjectPrx? prx)
        {
            try
            {
                string proxy = communicator.getProperties().getProperty(prefix);
                prx = new ObjectPrx(communicator.StringToReference(proxy, prefix));
            }
            catch (LocalException)
            {
                prx = null;
                return false;
            }
            return true;
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public void iceCheckTwowayOnly(string name)
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
        public void iceCheckAsyncTwowayOnly(string name)
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
        public int IceHandleException(Exception ex, RequestHandler handler, OperationMode mode, bool sent,
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
                    return IceReference.getInstance().proxyFactory().checkRetryAfterException((LocalException)ex,
                                                                                            IceReference,
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
        public RequestHandler IceGetRequestHandler()
        {
            if (IceReference.getCacheConnection())
            {
                lock (this)
                {
                    if (RequestHandler != null)
                    {
                        return RequestHandler;
                    }
                }
            }
            return IceReference.getRequestHandler(this);
        }

        [EditorBrowsable(EditorBrowsableState.Never)]
        public RequestHandler
        IceSetRequestHandler(RequestHandler handler)
        {
            if (IceReference.getCacheConnection())
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
        public void IceUpdateRequestHandler(RequestHandler previous, RequestHandler handler)
        {
            if (IceReference.getCacheConnection() && previous != null)
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
                        RequestHandler = RequestHandler.update(previous, handler);
                    }
                }
            }
        }

        protected OutgoingAsyncT<T>
        getOutgoingAsync<T>(OutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream iss = null;
            OutputStream os = null;

            if (IceReference.getInstance().cacheMessageBuffers() > 0)
            {
                lock (this)
                {
                    if (StreamCache != null && StreamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = StreamCache.First.Value.iss;
                        os = StreamCache.First.Value.os;

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
        getInvokeOutgoingAsync(OutgoingAsyncCompletionCallback completed)
        {
            bool haveEntry = false;
            InputStream iss = null;
            OutputStream os = null;

            if (IceReference.getInstance().cacheMessageBuffers() > 0)
            {
                lock (this)
                {
                    if (StreamCache != null && StreamCache.Count > 0)
                    {
                        haveEntry = true;
                        iss = StreamCache.First.Value.iss;
                        os = StreamCache.First.Value.os;

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
        CacheMessageBuffers(InputStream iss, OutputStream os)
        {
            lock (this)
            {
                if (StreamCache == null)
                {
                    StreamCache = new LinkedList<StreamCacheEntry>();
                }
                StreamCacheEntry cacheEntry;
                cacheEntry.iss = iss;
                cacheEntry.os = os;
                StreamCache.AddLast(cacheEntry);
            }
        }

        public struct StreamCacheEntry
        {
            public InputStream iss;
            public OutputStream os;
        }
    }

    /// <summary>
    /// Represent the result of the ice_invokeAsync operation
    /// </summary>
    public struct Object_Ice_invokeResult
    {
        public Object_Ice_invokeResult(bool returnValue, byte[] outEncaps)
        {
            this.returnValue = returnValue;
            this.outEncaps = outEncaps;
        }

        /// <summary>
        /// If the operation completed successfully, the return value
        /// is true. If the operation raises a user exception,
        /// the return value is false; in this case, outEncaps
        /// contains the encoded user exception.
        /// </summary>
        public bool returnValue;

        /// <summary>
        /// The encoded out-paramaters and return value for the operation.
        /// The return value follows any out-parameters. If returnValue is
        /// false it contains the encoded user exception.
        /// </summary>
        public byte[] outEncaps;
    };

    internal class InvokeOutgoingAsyncT : OutgoingAsync
    {
        public InvokeOutgoingAsyncT(IObjectPrx prx,
                                    OutgoingAsyncCompletionCallback completionCallback,
                                    OutputStream os = null,
                                    InputStream iss = null) : base(prx, completionCallback, os, iss)
        {
        }

        public void invoke(string operation, OperationMode mode, byte[] inParams,
                           Dictionary<string, string> context, bool synchronous)
        {
            try
            {
                prepare(operation, mode, context);
                if (inParams == null || inParams.Length == 0)
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
                if (proxy_.IceReference.getMode() == Ice.InvocationMode.Twoway)
                {
                    ret.outEncaps = is_.readEncapsulation(out encoding);
                }
                else
                {
                    ret.outEncaps = null;
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

    internal class InvokeTaskCompletionCallback : TaskCompletionCallback<Object_Ice_invokeResult>
    {
        public InvokeTaskCompletionCallback(IProgress<bool>? progress, CancellationToken cancellationToken) :
            base(progress, cancellationToken)
        {
        }

        public override void handleInvokeSent(bool sentSynchronously, bool done, bool alreadySent,
                                              OutgoingAsyncBase og)
        {
            if (progress_ != null && !alreadySent)
            {
                progress_.Report(sentSynchronously);
            }
            if (done)
            {
                SetResult(new Object_Ice_invokeResult(true, null));
            }
        }

        public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
        {
            SetResult(((InvokeOutgoingAsyncT)og).getResult(ok));
        }
    }

    /// <summary>
    /// Base class of all object proxies.
    /// </summary>
    [Serializable]
    public class ObjectPrx : IObjectPrx
    {
        public Reference IceReference { get; private set; }
        public RequestHandler? RequestHandler { get; set; }
        public LinkedList<IObjectPrx.StreamCacheEntry>? StreamCache { get; set; }

        public ObjectPrx(IObjectPrx other)
        {
            lock (other)
            {
                var h = (ObjectPrx)other;
                IceReference = h.IceReference;
                RequestHandler = h.RequestHandler;
            }
        }

        public object Clone()
        {
            return new ObjectPrx(this);
        }

        public virtual IObjectPrx Clone(Reference reference)
        {
            return new ObjectPrx(reference);
        }

        public ObjectPrx(Reference reference, RequestHandler? requestHandler = null)
        {
            if (reference == null)
            {
                throw new ArgumentNullException(nameof(reference));
            }
            IceReference = reference;
            RequestHandler = requestHandler;
        }

        protected ObjectPrx(SerializationInfo info, StreamingContext context)
        {
            Instance instance;

            if (context.Context is Communicator)
            {
                instance = IceInternal.Util.getInstance((Communicator)context.Context);
            }
            else if (context.Context is Instance)
            {
                instance = (Instance)context.Context;
            }
            else
            {
                throw new ArgumentException("Cannot deserialize proxy: Ice.Communicator not found in StreamingContext");
            }

            var proxy = (ObjectPrx)instance.proxyFactory().stringToProxy(info.GetString("proxy"));
            IceReference = proxy.IceReference;
            Debug.Assert(proxy.RequestHandler == null);
        }

        public virtual void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            info.AddValue("proxy", ToString());
        }

        /// <summary>
        /// Returns the stringified form of this proxy.
        /// </summary>
        /// <returns>The stringified proxy.</returns>
        public override string ToString()
        {
            return IceReference == null ? "reference: (null)" : IceReference.ToString();
        }

        /// <summary>
        /// Returns a hash code for this proxy.
        /// </summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode()
        {
            return IceReference.GetHashCode();
        }

        /// <summary>
        /// Returns whether this proxy equals the passed object. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="r">The object to compare this proxy with.</param>
        /// <returns>True if this proxy is equal to r; false, otherwise.</returns>
        public override bool Equals(object r)
        {
            var rhs = r as IObjectPrx;
            return ReferenceEquals(rhs, null) ? false : IceReference.Equals(rhs.IceReference);
        }

        /// <summary>
        /// Returns whether two proxies are equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are equal; false, otherwise.</returns>
        public static bool Equals(ObjectPrx lhs, ObjectPrx rhs)
        {
            return ReferenceEquals(lhs, null) ? ReferenceEquals(rhs, null) : lhs.Equals(rhs);
        }

        /// <summary>
        /// Returns whether two proxies are equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are equal; false, otherwise.</returns>
        public static bool operator ==(ObjectPrx lhs, ObjectPrx rhs)
        {
            return Equals(lhs, rhs);
        }

        /// <summary>
        /// Returns whether two proxies are not equal. Two proxies are equal if they are equal in all
        /// respects, that is, if their object identity, endpoints timeout settings, and so on are all equal.
        /// </summary>
        /// <param name="lhs">A proxy to compare with the proxy rhs.</param>
        /// <param name="rhs">A proxy to compare with the proxy lhs.</param>
        /// <returns>True if the proxies are not equal; false, otherwise.</returns>
        public static bool operator !=(ObjectPrx lhs, ObjectPrx rhs)
        {
            return !Equals(lhs, rhs);
        }
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
                                       Endpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       LocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       RouterPrx? router = null,
                                       bool? secure = null)
        {
            var reference = prx.IceReference.changeIdentity(id).ChangeOptions(
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
                                       Endpoint[]? endpoints = null,
                                       Connection? fixedConnection = null,
                                       InvocationMode? invocationMode = null,
                                       int? invocationTimeout = null,
                                       LocatorPrx? locator = null,
                                       int? locatorCacheTimeout = null,
                                       bool? oneway = null,
                                       bool? preferSecure = null,
                                       RouterPrx? router = null,
                                       bool? secure = null)
        {
            var reference = prx.IceReference.changeFacet(facet).ChangeOptions(
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
                                     Endpoint[]? endpoints = null,
                                     Connection? fixedConnection = null,
                                     InvocationMode? invocationMode = null,
                                     int? invocationTimeout = null,
                                     LocatorPrx? locator = null,
                                     int? locatorCacheTimeout = null,
                                     bool? oneway = null,
                                     bool? preferSecure = null,
                                     RouterPrx? router = null,
                                     bool? secure = null) where Prx : IObjectPrx
        {
            var reference = prx.IceReference.ChangeOptions(
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
                _proxy = proxy;
            }

            public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
            {
                SetResult(((ProxyGetConnection)og).getConnection());
            }

            private IObjectPrx _proxy;
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
                iceI_getConnection(prx, completed, true);
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
            iceI_getConnection(prx, completed, false);
            return completed.Task;
        }

        private static void iceI_getConnection(IObjectPrx prx, OutgoingAsyncCompletionCallback completed, bool synchronous)
        {
            var outgoing = new ProxyGetConnection(prx, completed);
            try
            {
                outgoing.invoke("ice_getConnection", synchronous);
            }
            catch (Exception ex)
            {
                outgoing.abort(ex);
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
            RequestHandler? handler;
            lock (prx)
            {
                handler = prx.RequestHandler;
            }

            if (handler != null)
            {
                try
                {
                    return handler.getConnection();
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
                                  out byte[] outEncaps,
                                  OptionalContext context = new OptionalContext())
        {
            try
            {
                var result = prx.iceI_ice_invokeAsync(operation, mode, inEncaps, context, null, CancellationToken.None, true).Result;
                outEncaps = result.outEncaps;
                return result.returnValue;
            }
            catch (AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        /// <summary>
        /// Invokes an operation dynamically.
        /// </summary>
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
                    OptionalContext context = new OptionalContext(),
                    IProgress<bool>? progress = null,
                    CancellationToken cancel = new CancellationToken())
        {
            return prx.iceI_ice_invokeAsync(operation, mode, inEncaps, context, progress, cancel, false);
        }

        private static Task<Object_Ice_invokeResult>
        iceI_ice_invokeAsync(this IObjectPrx prx,
                             string operation,
                             OperationMode mode,
                             byte[] inEncaps,
                             OptionalContext context,
                             IProgress<bool>? progress,
                             CancellationToken cancel,
                             bool synchronous)
        {
            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            prx.iceI_ice_invoke(operation, mode, inEncaps, context, completed, synchronous);
            return completed.Task;
        }

        private static void iceI_ice_invoke(this IObjectPrx prx,
                                     string operation,
                                     OperationMode mode,
                                     byte[] inEncaps,
                                     Dictionary<string, string> context,
                                     OutgoingAsyncCompletionCallback completed,
                                     bool synchronous)
        {
            prx.getInvokeOutgoingAsync(completed).invoke(operation, mode, inEncaps, context, synchronous);
        }
    }
}
