// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>Represents an ice1 or ice2 request frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutgoingFrame, IDisposable
    {
        /// <summary>The context of this request frame.</summary>
        public IReadOnlyDictionary<string, string> Context => _contextOverride ?? _initialContext;

        /// <summary>A cancellation token that receives the cancellation requests. The cancellation token takes into
        /// account the invocation timeout and the cancellation token provided by the application.</summary>
        public CancellationToken CancellationToken => _linkedCancellationSource.Token;

        /// <summary>ContextOverride is a writable version of Context, available only for ice2. Its entries are always
        /// the same as Context's entries.</summary>
        public SortedDictionary<string, string> ContextOverride
        {
            get
            {
                if (_contextOverride == null)
                {
                    if (Protocol == Protocol.Ice1)
                    {
                        throw new InvalidOperationException("cannot change the context of an ice1 request frame");
                    }
                    // lazy initialization
                    _contextOverride =
                        new SortedDictionary<string, string>((IDictionary<string, string>)_initialContext);
                }
                return _contextOverride;
            }
        }

        /// <summary>The deadline corresponds to the request's expiration time. Once the deadline is reached, the
        /// caller is no longer interested in the response and discards the request. The server-side runtime does not
        /// enforce this deadline - it's provided "for information" to the application. The Ice client runtime sets
        /// this deadline automatically using the proxy's invocation timeout and sends it with ice2 requests but not
        /// with ice1 requests. As a result, the deadline for an ice1 request is always <see cref="DateTime.MaxValue"/>
        /// on the server-side even though the invocation timeout is usually not infinite.</summary>
        public DateTime Deadline { get; }

        /// <summary>The encoding of the request payload.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>The location of the target Ice object. With ice1, it is always empty.</summary>
        public IReadOnlyList<string> Location { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        private SortedDictionary<string, string>? _contextOverride;
        private readonly ArraySegment<byte> _defaultBinaryContext;
        private readonly IReadOnlyDictionary<string, string> _initialContext;
        private readonly CancellationTokenSource? _invocationTimeoutCancellationSource;
        private readonly CancellationTokenSource _linkedCancellationSource;

        // When true, we always write Context in slot 0 of the binary context. This field is always false when
        // _defaultBinaryContext is empty.
        private readonly bool _writeSlot0;

        /// <inheritdoc/>
        public void Dispose()
        {
            _invocationTimeoutCancellationSource?.Dispose();
            _linkedCancellationSource.Dispose();
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/> for an operation with a single non-struct
        /// parameter.</summary>
        /// <typeparam name="T">The type of the operation's parameter.</typeparam>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>args</c> contains class
        /// instances.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="args">The argument(s) to write into the frame.</param>
        /// <param name="writer">The <see cref="OutputStreamWriter{T}"/> that writes the arguments into the frame.
        /// </param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame WithArgs<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            T args,
            OutputStreamWriter<T> writer,
            CancellationToken cancel = default)
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context, cancel);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(),
                                        request.Data,
                                        request.PayloadStart,
                                        request.Encoding,
                                        format);
            writer(ostr, args);
            request.PayloadEnd = ostr.Finish();
            if (compress && proxy.Encoding == Encoding.V20)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/> for an operation with a single stream
        /// parameter.</summary>
        /// <typeparam name="T">The type of the operation's parameter.</typeparam>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>args</c> contains class
        /// instances.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="args">The argument(s) to write into the frame.</param>
        /// <param name="writer">The delegate that will send the streamable.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage(
            "Microsoft.Performance",
            "CA1801: Review unused parameters",
            Justification = "TODO")]
        public static OutgoingRequestFrame WithArgs<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            T args,
            Action<SocketStream, T, CancellationToken> writer,
            CancellationToken cancel = default)
        {
            OutgoingRequestFrame request = WithEmptyArgs(proxy, operation, idempotent, context, cancel);
            // TODO: deal with compress, format, and cancel paramters
            request.StreamDataWriter = socketStream => writer(socketStream, args, cancel);
            return request;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/> for an operation with multiple parameters or a
        /// single struct parameter.</summary>
        /// <typeparam name="T">The type of the operation's parameters; it's a tuple type for an operation with multiple
        /// parameters.</typeparam>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>args</c> contains class
        /// instances.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="args">The argument(s) to write into the frame.</param>
        /// <param name="writer">The <see cref="OutputStreamWriter{T}"/> that writes the arguments into the frame.
        /// </param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame WithArgs<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            in T args,
            OutputStreamValueWriter<T> writer,
            CancellationToken cancel = default) where T : struct
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context, cancel);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(),
                                        request.Data,
                                        request.PayloadStart,
                                        request.Encoding,
                                        format);
            writer(ostr, in args);
            request.PayloadEnd = ostr.Finish();
            if (compress && proxy.Encoding == Encoding.V20)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/> for an operation with multiple parameters where
        /// one of the parameter is a stream parameter.</summary>
        /// <typeparam name="T">The type of the operation's parameters; it's a tuple type for an operation with multiple
        /// parameters.</typeparam>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>args</c> contains class
        /// instances.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="args">The argument(s) to write into the frame.</param>
        /// <param name="writer">The delegate that writes the arguments into the frame.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame WithArgs<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            in T args,
            OutputStreamValueWriterWithStreamable<T> writer,
            CancellationToken cancel = default) where T : struct
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context, cancel);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(),
                                        request.Data,
                                        request.PayloadStart,
                                        request.Encoding,
                                        format);
            // TODO: deal with compress, format, and cancel paramters
            request.StreamDataWriter = writer(ostr, in args, cancel);
            request.PayloadEnd = ostr.Finish();
            if (compress && proxy.Encoding == Encoding.V20)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/> for an operation with no parameter.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A new OutgoingRequestFrame.</returns>
        public static OutgoingRequestFrame WithEmptyArgs(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            IReadOnlyDictionary<string, string>? context = null,
            CancellationToken cancel = default) =>
            new OutgoingRequestFrame(proxy,
                                     operation,
                                     idempotent,
                                     compress: false,
                                     context,
                                     cancel,
                                     writeEmptyArgs: true);

        /// <summary>Constructs an outgoing request frame from the given incoming request frame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet
        /// and context of this proxy to create the request frame.</param>
        /// <param name="request">The incoming request from which to create an outgoing request.</param>
        /// <param name="forwardBinaryContext">When true (the default), the new frame uses the incoming request frame's
        /// binary context as a fallback - all the entries in this binary context are added before the frame is sent,
        /// except for entries previously added by invocation interceptors.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public OutgoingRequestFrame(
            IObjectPrx proxy,
            IncomingRequestFrame request,
            bool forwardBinaryContext = true,
            CancellationToken cancel = default)
            : this(proxy, request.Operation, request.IsIdempotent, compress: false, request.Context, cancel)
        {
            if (request.Protocol == Protocol)
            {
                // Finish off current segment
                Data[^1] = Data[^1].Slice(0, PayloadStart.Offset);

                // We only include the encapsulation.
                Data.Add(request.Payload);
                PayloadEnd = new OutputStream.Position(Data.Count - 1, request.Payload.Count);

                if (Protocol == Protocol.Ice2 && forwardBinaryContext)
                {
                    bool hasSlot0 = request.BinaryContext.ContainsKey(0);

                    // If slot 0 is the only slot, we don't set _defaultBinaryContext: this way, Context always
                    // prevails in slot 0, even when it's empty and slot 0 is not written at all.

                    if (!hasSlot0 || request.BinaryContext.Count > 1)
                    {
                        _defaultBinaryContext = request.Data.Slice(
                            request.Payload.Offset - request.Data.Offset + request.Payload.Count);

                        // When slot 0 has an empty value, there is no need to always write it since the default
                        // (empty Context) is correctly represented by the entry in the _defaultBinaryContext.
                        _writeSlot0 = hasSlot0 && !request.BinaryContext[0].IsEmpty;
                    }
                }
            }
            else
            {
                // We forward the encapsulation and the string-string context. The context was marshaled by the
                // constructor (when Protocol == Ice1) or will be written by Finish (when Protocol == Ice2).
                // The payload encoding must remain the same since we cannot transcode the encoded bytes.

                int sizeLength = request.Protocol == Protocol.Ice1 ? 4 : request.Payload[0].ReadSizeLength20();

                OutputStream.Position tail =
                    OutputStream.WriteEncapsulationHeader(Data,
                                                          PayloadStart,
                                                          Protocol.GetEncoding(),
                                                          request.Payload.Count - sizeLength,
                                                          request.Encoding);

                // Finish off current segment
                Data[^1] = Data[^1].Slice(0, tail.Offset);

                // "2" below corresponds to the encoded length of the encoding.
                if (request.Payload.Count > sizeLength + 2)
                {
                    // Add encoded bytes, not including the header or binary context.
                    Data.Add(request.Payload.Slice(sizeLength + 2));

                    PayloadEnd = new OutputStream.Position(Data.Count - 1, request.Payload.Count - sizeLength - 2);
                }
                else
                {
                    PayloadEnd = tail;
                }
            }

            Size = Data.GetByteCount();
            IsSealed = Protocol == Protocol.Ice1;
        }

        // Finish prepares the frame for sending and writes the frame's context into slot 0 of the binary context.
        internal override void Finish()
        {
            if (!IsSealed)
            {
                if (Protocol == Protocol.Ice2 && !ContainsKey(0))
                {
                    if (Context.Count > 0 || _writeSlot0)
                    {
                        // When _writeSlot0 is true, we may write an empty string-string context, thus preventing base
                        // from writing a non-empty Context.
                        AddBinaryContextEntry(0, Context, (ostr, dictionary) =>
                            ostr.WriteDictionary(dictionary,
                                                 OutputStream.IceWriterFromString,
                                                 OutputStream.IceWriterFromString));
                    }
                }
                base.Finish();
            }
        }

        private protected override ArraySegment<byte> GetDefaultBinaryContext() => _defaultBinaryContext;

        private OutgoingRequestFrame(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            IReadOnlyDictionary<string, string>? context,
            CancellationToken cancel,
            bool writeEmptyArgs = false)
            : base(proxy.Protocol,
                   compress,
                   proxy.Communicator.CompressionLevel,
                   proxy.Communicator.CompressionMinSize,
                   new List<ArraySegment<byte>>())
        {
            Encoding = proxy.Encoding;
            Identity = proxy.Identity;
            Facet = proxy.Facet;
            Location = proxy.Location;
            Operation = operation;
            IsIdempotent = idempotent;

            Debug.Assert(proxy.InvocationTimeout != TimeSpan.Zero);
            Deadline = Protocol == Protocol.Ice1 || proxy.InvocationTimeout == Timeout.InfiniteTimeSpan ?
                DateTime.MaxValue : DateTime.UtcNow + proxy.InvocationTimeout;

            if (proxy.InvocationTimeout != Timeout.InfiniteTimeSpan)
            {
                _invocationTimeoutCancellationSource = new CancellationTokenSource(proxy.InvocationTimeout);
            }

            _linkedCancellationSource = CancellationTokenSource.CreateLinkedTokenSource(
                _invocationTimeoutCancellationSource?.Token ?? default,
                proxy.Communicator.CancellationToken,
                cancel);

            if (context != null)
            {
                // This makes a copy if context is not immutable.
                _initialContext = context.ToImmutableSortedDictionary();
            }
            else
            {
                IReadOnlyDictionary<string, string> currentContext = proxy.Communicator.CurrentContext;

                if (proxy.Context.Count == 0)
                {
                    _initialContext = currentContext;
                }
                else if (currentContext.Count == 0)
                {
                    _initialContext = proxy.Context;
                }
                else
                {
                    var combinedContext = new SortedDictionary<string, string>(
                            (IDictionary<string, string>)currentContext);
                    foreach ((string key, string value) in proxy.Context)
                    {
                        combinedContext[key] = value; // the proxy Context entry prevails.
                    }
                    _initialContext = combinedContext;
                }
            }

            var ostr = new OutputStream(proxy.Protocol.GetEncoding(), Data);

            if (Protocol == Protocol.Ice1)
            {
                ostr.WriteIce1RequestHeaderBody(Identity, Facet, Operation, IsIdempotent, _initialContext);
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                ostr.WriteIce2RequestHeaderBody(Identity,
                                                Facet,
                                                Location,
                                                Operation,
                                                IsIdempotent,
                                                Deadline);
            }
            PayloadStart = ostr.Tail;

            if (writeEmptyArgs)
            {
                PayloadEnd = ostr.WriteEmptyEncapsulation(Encoding);
            }
        }
    }
}
