//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>Represents an ice1 or ice2 request frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutgoingFrame
    {
        /// <summary>The context of this request frame.</summary>
        public IReadOnlyDictionary<string, string> Context => _contextOverride ?? _initialContext;

        /// <summary>ContextOverride is a writable version of Context, available only for ice2. Its entries are always
        /// the same as Context's entries.</summary>
        public Dictionary<string, string> ContextOverride
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
                    _contextOverride = new Dictionary<string, string>(_initialContext);
                }
                return _contextOverride;
            }
        }

        /// <summary>The encoding of the request payload.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }
        private Dictionary<string, string>? _contextOverride;

        private readonly ArraySegment<byte> _defaultBinaryContext;

        private readonly IReadOnlyDictionary<string, string> _initialContext;

        // When true, we always write Context in slot 0 of the binary context. This field is always false when
        // _defaultBinaryContext is empty.
        private readonly bool _writeSlot0;

        /// <summary>Create a new OutgoingRequestFrame for an operation with a single parameter.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format used to marshal classes.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="value">The parameter to marshal in the frame.</param>
        /// <param name="writer">The delegate into marshal the parameter to the frame.</param>
        /// <returns>A new OutgoingRequestFrame</returns>
        public static OutgoingRequestFrame WithSingleParam<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            T value,
            OutputStreamWriter<T> writer)
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(),
                                        request.Data,
                                        request.PayloadStart,
                                        request.Encoding,
                                        format);
            writer(ostr, value);
            request.PayloadEnd = ostr.Finish();
            if (compress && proxy.Encoding == Encoding.V2_0)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Create a new OutgoingRequestFrame for an operation with multiple parameters or a single struct
        /// parameter.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format used to marshal classes.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="value">The parameters to marshal in the frame.</param>
        /// <param name="writer">The delegate to marshal the parameters into the frame.</param>
        /// <returns>A new OutgoingRequestFrame</returns>
        public static OutgoingRequestFrame WithParamList<T>(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            bool compress,
            FormatType format,
            IReadOnlyDictionary<string, string>? context,
            in T value,
            OutputStreamValueWriter<T> writer) where T : struct
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(),
                                        request.Data,
                                        request.PayloadStart,
                                        request.Encoding,
                                        format);
            writer(ostr, value);
            request.PayloadEnd = ostr.Finish();
            if (compress && proxy.Encoding == Encoding.V2_0)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Creates a new outgoing request frame with no parameter.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public static OutgoingRequestFrame WithEmptyParamList(
            IObjectPrx proxy,
            string operation,
            bool idempotent,
            IReadOnlyDictionary<string, string>? context = null) =>
            new OutgoingRequestFrame(proxy,
                                     operation,
                                     idempotent,
                                     compress: false,
                                     context,
                                     writeEmptyParamList: true);

        /// <summary>Creates a new outgoing request frame from the given incoming request frame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet
        /// and context of this proxy to create the request frame.</param>
        /// <param name="request">The incoming request from which to create an outgoing request.</param>
        /// <param name="forwardBinaryContext">When true (the default), the new frame uses the incoming request frame's
        /// binary context as a fallback - all the entries in this binary context are added before the frame is sent,
        /// except for entries previously added by invocation interceptors.</param>
        public OutgoingRequestFrame(IObjectPrx proxy, IncomingRequestFrame request, bool forwardBinaryContext = true)
            : this(proxy, request.Operation, request.IsIdempotent, compress: false, request.Context)
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
            bool writeEmptyParamList = false)
            : base(proxy.Protocol,
                   compress,
                   proxy.Communicator.CompressionLevel,
                   proxy.Communicator.CompressionMinSize,
                   new List<ArraySegment<byte>>())
        {
            Encoding = proxy.Encoding;
            Identity = proxy.Identity;
            Facet = proxy.Facet;
            Operation = operation;
            IsIdempotent = idempotent;
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(), Data);
            Identity.IceWrite(ostr);
            ostr.WriteFacet(Facet);
            ostr.WriteString(operation);
            ostr.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);
            if (context != null)
            {
                _initialContext = context;
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
                    var combinedContext = new Dictionary<string, string>(currentContext);
                    foreach ((string key, string value) in proxy.Context)
                    {
                        combinedContext[key] = value; // the proxy Context entry prevails.
                    }
                    _initialContext = combinedContext;
                }
            }

            if (Protocol == Protocol.Ice1)
            {
                ostr.WriteDictionary(_initialContext,
                                     OutputStream.IceWriterFromString,
                                     OutputStream.IceWriterFromString);
            }
            PayloadStart = ostr.Tail;

            if (writeEmptyParamList)
            {
                PayloadEnd = ostr.WriteEmptyEncapsulation(Encoding);
            }
        }
    }

    /// <summary>An OutgoingRequestFrame factory for an operation with no parameter.</summary>
    public sealed class OutgoingRequestFrameFactory
    {
        private readonly bool _idempotent;
        private readonly string _operationName;

        /// <summary>Constructs an OutgoingRequestFrame factory for the given operation.</summary>
        /// <param name="operationName">The operation name.</param>
        /// <param name="idempotent"><c>True</c> if the requests are idempotent, <c>False</c> otherwise.</param>
        public OutgoingRequestFrameFactory(string operationName, bool idempotent)
        {
            _idempotent = idempotent;
            _operationName = operationName;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/>.</summary>
        /// <param name="prx">The proxy used to send the new request frame.</param>
        /// <param name="context">The Ice request context.</param>
        public OutgoingRequestFrame Create(IObjectPrx prx, IReadOnlyDictionary<string, string>? context) =>
            OutgoingRequestFrame.WithEmptyParamList(prx, _operationName, _idempotent, context);
    }

    /// <summary>An OutgoingRequestFrame factory for an operation with multiple parameters or a single Slice struct
    /// parameter.</summary>
    public class OutgoingRequestFrameFactory<TParamList> where TParamList : struct
    {
        private readonly bool _compress;
        private readonly FormatType _format;
        private readonly bool _idempotent;
        private readonly string _operationName;
        private readonly OutputStreamValueWriter<TParamList> _writer;

        /// <summary>Constructs an OutgoingRequestFrame factory for the given operation.</summary>
        /// <param name="operationName">The operation name.</param>
        /// <param name="idempotent"><c>True</c> if the requests are idempotent, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request's parameters are compressed during frame creation with
        /// ice2, or if entire request frame is compressed during sending with ice1; otherwise, <c>False</c>.</param>
        /// <param name="format">The format for class instances.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request parameters.</param>
        public OutgoingRequestFrameFactory(
            string operationName,
            bool idempotent,
            bool compress,
            FormatType format,
            OutputStreamValueWriter<TParamList> writer)
        {
            _compress = compress;
            _format = format;
            _idempotent = idempotent;
            _operationName = operationName;
            _writer = writer;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/>.</summary>
        /// <param name="prx">The proxy used to send the new request frame.</param>
        /// <param name="paramList">The request parameters.</param>
        /// <param name="context">The Ice request context.</param>
        public OutgoingRequestFrame Create(
            IObjectPrx prx,
            in TParamList paramList,
            IReadOnlyDictionary<string, string>? context) =>
            OutgoingRequestFrame.WithParamList(prx,
                                               _operationName,
                                               _idempotent,
                                               _compress,
                                               _format,
                                               context,
                                               in paramList,
                                               _writer);
    }

    /// <summary>An OutgoingRequestFrame factory for an operation with a single parameter.</summary>
    public class SingleParamOutgoingRequestFrameFactory<TParam>
    {
        private readonly bool _compress;
        private readonly FormatType _format;
        private readonly bool _idempotent;
        private readonly string _operationName;
        private readonly OutputStreamWriter<TParam> _writer;

        /// <summary>Constructs an OutgoingRequestFrame factory for the given operation.</summary>
        /// <param name="operationName">The operation name.</param>
        /// <param name="idempotent"><c>True</c> if the requests are idempotent, <c>False</c> otherwise.</param>
        /// <param name="compress"><c>True</c> if the request's parameter is compressed during frame creation with
        /// ice2, or if entire request frame is compressed during sending with ice1; otherwise, <c>False</c>.</param>
        /// <param name="format">The format for class instances.</param>
        /// <param name="writer">The <see cref="OutputStream"/> writer used to write the request parameter.</param>
        public SingleParamOutgoingRequestFrameFactory(
            string operationName,
            bool idempotent,
            bool compress,
            FormatType format,
            OutputStreamWriter<TParam> writer)
        {
            _compress = compress;
            _format = format;
            _idempotent = idempotent;
            _operationName = operationName;
            _writer = writer;
        }

        /// <summary>Creates a new <see cref="OutgoingRequestFrame"/>.</summary>
        /// <param name="prx">The proxy used to send the new request frame.</param>
        /// <param name="param">The parameter.</param>
        /// <param name="context">The Ice request context.</param>
        public OutgoingRequestFrame Create(
            IObjectPrx prx,
            TParam param,
            IReadOnlyDictionary<string, string>? context) =>
            OutgoingRequestFrame.WithSingleParam(prx,
                                               _operationName,
                                               _idempotent,
                                               _compress,
                                               _format,
                                               context,
                                               param,
                                               _writer);
    }
}
