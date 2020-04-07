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
            _iceIsA_Request.Invoke(this, id, context);

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
            _iceIsA_Request.InvokeAsync(this, id, context, progress, cancel);

        /// <summary>
        /// Tests whether the target object of this proxy can be reached.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        public void IcePing(IReadOnlyDictionary<string, string>? context = null) =>
            IceInvoke("ice_ping", idempotent: true, context);

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
            IceInvokeAsync("ice_ping", idempotent: true, context, progress, cancel);

        /// <summary>
        /// Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
        /// </summary>
        /// <param name="context">The context dictionary for the invocation.</param>
        /// <returns>The Slice type IDs of the interfaces supported by the target object, in base-to-derived
        /// order. The first element of the returned array is always ::Ice::IObject.</returns>
        public string[] IceIds(IReadOnlyDictionary<string, string>? context = null) =>
            _iceIds_Request.Invoke(this, context);

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
            _iceIds_Request.InvokeAsync(this, context, progress, cancel);

        /// <summary>
        /// Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
        /// </summary>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(IReadOnlyDictionary<string, string>? context = null) =>
            _iceId_Request.Invoke(this, context);

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
            _iceId_Request.InvokeAsync(this, context, progress, cancel);

        /// <summary>
        /// Returns the identity embedded in this proxy.
        /// <returns>The identity of the target object.</returns>
        /// </summary>
        public Identity Identity => IceReference.Identity;

        /// <summary>Returns the context of this proxy.</summary>
        public IReadOnlyDictionary<string, string> Context => IceReference.Context;

        /// <summary>
        /// Returns the facet for this proxy.
        /// </summary>
        /// <returns>The facet for this proxy. If the proxy uses the default facet, the return value is the
        /// empty string.</returns>
        public string Facet => IceReference.Facet;

        /// <summary>
        /// Returns the adapter ID for this proxy.
        /// </summary>
        /// <returns>The adapter ID. If the proxy does not have an adapter ID, the return value is the
        /// empty string.</returns>
        public string AdapterId => IceReference.AdapterId;

        /// <summary>
        /// Returns the endpoints used by this proxy.
        /// </summary>
        /// <returns>The endpoints used by this proxy.</returns>
        public IEndpoint[] Endpoints => (IEndpoint[])IceReference.Endpoints.Clone();

        /// <summary>
        /// Returns the locator cache timeout of this proxy.
        /// </summary>
        /// <returns>The locator cache timeout value (in seconds).</returns>
        public int LocatorCacheTimeout => IceReference.LocatorCacheTimeout;

        /// <summary>
        /// Returns the invocation timeout of this proxy.
        /// </summary>
        /// <returns>The invocation timeout value (in seconds).</returns>
        public int InvocationTimeout => IceReference.InvocationTimeout;

        /// <summary>
        /// Returns whether this proxy caches connections.
        /// </summary>
        /// <returns>True if this proxy caches connections; false, otherwise.</returns>
        public bool IsConnectionCached => IceReference.IsConnectionCached;

        /// <summary>
        /// Returns how this proxy selects endpoints (randomly or ordered).
        /// </summary>
        /// <returns>The endpoint selection policy.</returns>
        public EndpointSelectionType EndpointSelection => IceReference.EndpointSelection;

        /// <summary>
        /// Returns whether this proxy communicates only via secure endpoints.
        /// </summary>
        /// <returns>True if this proxy communicates only vi secure endpoints; false, otherwise.</returns>
        public bool IsSecure => IceReference.IsSecure;

        /// <summary>Returns the encoding version used to marshal requests parameters.</summary>
        /// <returns>The encoding version.</returns>
        public Encoding Encoding => IceReference.Encoding;

        /// <summary>
        /// Returns whether this proxy prefers secure endpoints.
        /// </summary>
        /// <returns>True if the proxy always attempts to invoke via secure endpoints before it
        /// attempts to use insecure endpoints; false, otherwise.</returns>
        public bool IsPreferSecure => IceReference.IsPreferSecure;

        /// <summary>
        /// Returns the router for this proxy.
        /// </summary>
        /// <returns>The router for the proxy. If no router is configured for the proxy, the return value
        /// is null.</returns>
        public IRouterPrx? Router => IceReference.RouterInfo?.Router;

        /// <summary>
        /// Returns the locator for this proxy.
        /// </summary>
        /// <returns>The locator for this proxy. If no locator is configured, the return value is null.</returns>
        public ILocatorPrx? Locator => IceReference.LocatorInfo?.Locator;

        /// <summary>
        /// Returns whether this proxy uses collocation optimization.
        /// </summary>
        /// <returns>True if the proxy uses collocation optimization; false, otherwise.</returns>
        public bool IsCollocationOptimized => IceReference.IsCollocationOptimized;

        /// <summary>Returns whether or not an operation invoked on this proxy returns a response.</summary>
        /// <returns>True if invoking an operation on this proxy does not return a response. This corresponds to
        /// several <see cref="InvocationMode"/> enumerators, such as Oneway and Datagram. Otherwise,
        /// returns false.</returns>
        public bool IsOneway => IceReference.InvocationMode != InvocationMode.Twoway;

        public InvocationMode InvocationMode => IceReference.InvocationMode;

        /// <summary>
        /// Obtains the compression override setting of this proxy.
        /// </summary>
        /// <returns>The compression override setting. If no optional value is present, no override is
        /// set. Otherwise, true if compression is enabled, false otherwise.</returns>
        public bool? Compress => IceReference.Compress;

        /// <summary>
        /// Obtains the timeout override of this proxy.
        /// </summary>
        /// <returns>The timeout override. If no optional value is present, no override is set. Otherwise,
        /// returns the timeout override value.</returns>
        public int? ConnectionTimeout => IceReference.ConnectionTimeout;

        /// <summary>
        /// Returns the connection id of this proxy.
        /// </summary>
        /// <returns>The connection id.</returns>
        public string ConnectionId => IceReference.ConnectionId;

        /// <summary>
        /// Returns whether this proxy is a fixed proxy.
        /// </summary>
        /// <returns>True if this is a fixed proxy, false otherwise.
        /// </returns>
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

        // publicly visible Ice-internal type used to make request without input and output parameters.
        protected void IceInvoke(string operationName,
                                 bool idempotent,
                                 IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithEmptyParamList(this, operationName, idempotent, context);

            var completed = new InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(this, completed, request, IsOneway).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                if (!IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        protected Task IceInvokeAsync(string operationName,
                                      bool idempotent,
                                      IReadOnlyDictionary<string, string>? context,
                                      IProgress<bool>? progress,
                                      CancellationToken cancel)
        {
            var request = OutgoingRequestFrame.WithEmptyParamList(this, operationName, idempotent, context);

            var completed = new InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(this, completed, request, IsOneway).Invoke(
                request.Operation, request.Context, synchronous: false);

            return ReadVoidReturnValueAsync(this, completed);

            static async Task ReadVoidReturnValueAsync(IObjectPrx prx, InvokeTaskCompletionCallback completed)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
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

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected static async Task IceReadVoidResponseAsync(Task<IncomingResponseFrame> task)
            => (await task.ConfigureAwait(false)).ReadVoidReturnValue();

        [EditorBrowsable(EditorBrowsableState.Never)]
        protected static async Task<T> IceReadResponseAsync<T>(Task<IncomingResponseFrame> task,
                                                               InputStreamReader<T> reader)
            => (await task.ConfigureAwait(false)).ReadReturnValue(reader);

        private static OutgoingRequestWithNonStructParam<string, bool>? _iceIsA_RequestValue;
        private static OutgoingRequestWithNonStructParam<string, bool> _iceIsA_Request
        {
            get
            {
                if (_iceIsA_RequestValue == null)
                {
                    _iceIsA_RequestValue = new OutgoingRequestWithNonStructParam<string, bool>("ice_isA",
                                                                idempotent: true,
                                                                format: null,
                                                                writer: OutputStream.IceWriterFromString,
                                                                reader: InputStream.IceReaderIntoBool);
                }
                return _iceIsA_RequestValue;
            }
        }

        private static OutgoingRequestWithEmptyParamList<string[]>? _iceIds_RequestValue;

        private static OutgoingRequestWithEmptyParamList<string[]> _iceIds_Request
        {
            get
            {
                if (_iceIds_RequestValue == null)
                {
                    _iceIds_RequestValue = new OutgoingRequestWithEmptyParamList<string[]>(
                        "ice_ids",
                        idempotent: true,
                        reader: istr => istr.ReadStringArray());
                }
                return _iceIds_RequestValue;
            }
        }

        private static OutgoingRequestWithEmptyParamList<string>? _iceId_RequestValue;

        private static OutgoingRequestWithEmptyParamList<string> _iceId_Request
        {
            get
            {
                if (_iceId_RequestValue == null)
                {
                    _iceId_RequestValue = new OutgoingRequestWithEmptyParamList<string>(
                        "ice_id", idempotent: true, InputStream.IceReaderIntoString);
                }
                return _iceId_RequestValue;
            }
        }
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

    // publicly visible Ice-internal type used to make request with input and output parameters,
    // the input parameters are pass with the `in' modifier, this helper is used when input parameter
    // is an structure type, or when there is several input parameters as we pass them wrapped in a
    // tuple.
    public class OutgoingRequest<TInParams, TOutParams> where TInParams : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamStructWriter<TInParams> _writer;
        private readonly InputStreamReader<TOutParams> _reader;

        public OutgoingRequest(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamStructWriter<TInParams> writer,
            InputStreamReader<TOutParams> reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
            _reader = reader;
        }

        public TOutParams Invoke(IObjectPrx prx, in TInParams inParams, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                return response.ReadReturnValue(_reader);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        public Task<TOutParams> InvokeAsync(IObjectPrx prx,
                                            in TInParams inParams,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(request.Operation, request.Context,
                synchronous: false);
            return ReadReturnValueAsync(completed, _reader);

            static async Task<TOutParams> ReadReturnValueAsync(IObjectPrx.InvokeTaskCompletionCallback completed,
                InputStreamReader<TOutParams> reader)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                return response.ReadReturnValue(reader);
            }
        }
    }

    // publicly visible Ice-internal type used to make request with input and output parameters,
    // the input parameters are pass without the `in' modifier, this helper is used when there is a single
    // input parameter and the parameter is of a non structure type.
    public class OutgoingRequestWithNonStructParam<TInParams, TOutParams>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamWriter<TInParams> _writer;
        private readonly InputStreamReader<TOutParams> _reader;

        public OutgoingRequestWithNonStructParam(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamWriter<TInParams> writer,
            InputStreamReader<TOutParams> reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
            _reader = reader;
        }

        public TOutParams Invoke(IObjectPrx prx, TInParams inParams, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                return response.ReadReturnValue(_reader);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        public Task<TOutParams> InvokeAsync(IObjectPrx prx,
                                            TInParams inParams,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel)
        {
            Debug.Assert(_writer != null);
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: false);

            return ReadReturnValueAsync(completed, _reader);

            static async Task<TOutParams> ReadReturnValueAsync(IObjectPrx.InvokeTaskCompletionCallback completed,
                InputStreamReader<TOutParams> reader)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                return response.ReadReturnValue(reader);
            }
        }
    }

    // publicly visible Ice-internal type used to make request with input parameters and no output parameters,
    // the input parameters are pass with the `in' modifier, this helper is used when input parameter
    // is an structure type, or when there is several input parameters as we pass them wrapped in a
    // tuple.
    public class OutgoingRequestWithVoidReturnValue<TInParams> where TInParams : struct
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamStructWriter<TInParams> _writer;

        public OutgoingRequestWithVoidReturnValue(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamStructWriter<TInParams> writer)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public void Invoke(IObjectPrx prx, in TInParams inParams, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(prx, completed, request, prx.IsOneway).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        public Task InvokeAsync(IObjectPrx prx, in TInParams inParams,
                                IReadOnlyDictionary<string, string>? context,
                                IProgress<bool>? progress,
                                CancellationToken cancel)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, prx.IsOneway).Invoke(
                request.Operation, request.Context, synchronous: false);
            return ReadVoidReturnValueAsync(prx, completed);

            static async Task ReadVoidReturnValueAsync(IObjectPrx prx,
                IObjectPrx.InvokeTaskCompletionCallback completed)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
        }
    }

    // publicly visible Ice-internal type used to make request with input parameters and no output parameters,
    // the input parameters are pass without the `in' modifier, this helper is used when there is a single
    // input parameter and the parameter is of a non structure type.
    public class OutgoingRequestWithNonStructParamAndVoidReturn<TInParams>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly FormatType? _format;
        private readonly OutputStreamWriter<TInParams> _writer;

        public OutgoingRequestWithNonStructParamAndVoidReturn(
            string operationName,
            bool idempotent,
            FormatType? format,
            OutputStreamWriter<TInParams> writer)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _format = format;
            _writer = writer;
        }

        public void Invoke(IObjectPrx prx, TInParams inParams, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(prx, completed, request, prx.IsOneway).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        public Task InvokeAsync(IObjectPrx prx, TInParams inParams,
                                IReadOnlyDictionary<string, string>? context,
                                IProgress<bool>? progress,
                                CancellationToken cancel)
        {
            var request = OutgoingRequestFrame.WithParamList(
                prx, _operationName, _idempotent, _format, context, inParams, _writer);

            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, prx.IsOneway).Invoke(
                request.Operation, request.Context, synchronous: false);
            return ReadVoidReturnValueAsync(prx, completed);

            static async Task ReadVoidReturnValueAsync(IObjectPrx prx,
                IObjectPrx.InvokeTaskCompletionCallback completed)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                if (!prx.IsOneway)
                {
                    response.ReadVoidReturnValue();
                }
            }
        }
    }

    // publicly visible Ice-internal type used to make request without input parameters and with
    // output parameters.
    public class OutgoingRequestWithEmptyParamList<TOutParams>
    {
        private readonly string _operationName;
        private readonly bool _idempotent;
        private readonly InputStreamReader<TOutParams> _reader;

        public OutgoingRequestWithEmptyParamList(
            string operationName,
            bool idempotent,
            InputStreamReader<TOutParams> reader)
        {
            _operationName = operationName;
            _idempotent = idempotent;
            _reader = reader;
        }

        public TOutParams Invoke(IObjectPrx prx, IReadOnlyDictionary<string, string>? context)
        {
            var request = OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context);
            var completed = new IObjectPrx.InvokeTaskCompletionCallback(null, default);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: true);
            try
            {
                IncomingResponseFrame response = completed.Task.Result;
                return response.ReadReturnValue(_reader);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        public Task<TOutParams> InvokeAsync(IObjectPrx prx,
                                            IReadOnlyDictionary<string, string>? context,
                                            IProgress<bool>? progress,
                                            CancellationToken cancel)
        {
            var request = OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context);
            var completed = new IObjectPrx.InvokeTaskCompletionCallback(progress, cancel);
            new OutgoingAsync(prx, completed, request, oneway: false).Invoke(
                request.Operation, request.Context, synchronous: false);

            return ReadReturnValueAsync(completed, _reader);

            static async Task<TOutParams> ReadReturnValueAsync(IObjectPrx.InvokeTaskCompletionCallback completed,
                                                               InputStreamReader<TOutParams> reader)
            {
                IncomingResponseFrame response = await completed.Task.ConfigureAwait(false);
                return response.ReadReturnValue(reader);
            }
        }
    }
}
