//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    /// <summary>Represents a request protocol frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Dictionary<string, string> Context { get; }

        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private set; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>Returns a list of array segments with the contents of the request frame payload.
        /// The request payload correspond to the encapsulation parameters, the Payload data is lazy
        /// initialize from the frame data the first time it is accessed, if the Payload has not been
        /// written it returns an empty list.</summary>
        public IList<ArraySegment<byte>> Payload
        {
            get
            {
                // thread-safe lazy initialization reference assignment is atomic
                if (_payload == null)
                {
                    var payload = new List<ArraySegment<byte>>();
                    OutputStream.Position payloadEnd = _payloadEnd!.Value;
                    ArraySegment<byte> segment = Data[_payloadStart.Segment].Slice(_payloadStart.Offset);
                    if (segment.Count > 0)
                    {
                        payload.Add(segment);
                    }
                    for (int i = _payloadStart.Segment + 1; i < payloadEnd.Segment; i++)
                    {
                        payload.Add(Data[i]);
                    }
                    segment = Data[payloadEnd.Segment].Slice(0, payloadEnd.Offset);
                    if (segment.Count > 0)
                    {
                        payload.Add(segment);
                    }
                    _payload = payload;
                }

                return _payload;
            }
        }

        /// <summary>The frame byte count.</summary>
        public int Size { get; private set; }

        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        // Store the Payload property data
        private List<ArraySegment<byte>>? _payload;

        // Position of the end of the payload, for Ice1 this is always the frame end.
        private OutputStream.Position? _payloadEnd;

        // Position of the start of the payload.
        private OutputStream.Position _payloadStart;

        /// <summary>Create a new OutgoingRequestFrame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="value">The parameter to marshal in the frame.</param>
        /// <param name="writer">The delegate into marshal the parameter to the frame.</param>
        /// <returns>A new OutgoingRequestFrame</returns>
        public static OutgoingRequestFrame WithParamList<T>(
            IObjectPrx proxy, string operation, bool idempotent, FormatType? format,
            IReadOnlyDictionary<string, string>? context,
            T value, OutputStreamWriter<T> writer)
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, context);
            var ostr = new OutputStream(request.Encoding, request.Data, request._payloadStart,
                format ?? proxy.Communicator.DefaultFormat);
            writer(ostr, value);
            request.Finish(ostr.Save());
            return request;
        }

        /// <summary>Create a new OutgoingRequestFrame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="value">The parameter to marshal in the frame, when the request frame contain multiple
        /// parameters they must be passed as a tuple.</param>
        /// <param name="writer">The delegate to marshal the parameters into the frame.</param>
        /// <returns>A new OutgoingRequestFrame</returns>
        public static OutgoingRequestFrame WithParamList<T>(
            IObjectPrx proxy, string operation, bool idempotent, FormatType? format,
            IReadOnlyDictionary<string, string>? context,
            in T value, OutputStreamStructWriter<T> writer) where T : struct
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, context);
            var ostr = new OutputStream(request.Encoding, request.Data, request._payloadStart,
                format ?? proxy.Communicator.DefaultFormat);
            writer(ostr, value);
            request.Finish(ostr.Save());
            return request;
        }

        /// <summary>Creates a new outgoing request frame with no parameters.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        public static OutgoingRequestFrame WithEmptyParamList(IObjectPrx proxy, string operation, bool idempotent,
                                                              IReadOnlyDictionary<string, string>? context = null)
            => new OutgoingRequestFrame(proxy, operation, idempotent, context, writeEmptyParamList: true);

        /// <summary>Creates a new outgoing request frame with the given payload.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet
        /// and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="payload">The payload of this request frame, which represents the marshaled in-parameters.
        /// </param>
        // TODO: should we pass the payload as a list of segments, or maybe has a separate
        // ctor that accepts a list of segments instead of a single segment
        public OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent,
                                    IReadOnlyDictionary<string, string>? context, ArraySegment<byte> payload)
            : this(proxy, operation, idempotent, context)
        {
            if (payload.Count < 6)
            {
                throw new ArgumentException(
                    $"payload should contain at least 6 bytes, but it contains {payload.Count} bytes",
                    nameof(payload));
            }
            int size = InputStream.ReadInt(payload.AsSpan(0, 4));
            if (size != payload.Count)
            {
                throw new ArgumentException($"invalid payload size `{size}' expected `{payload.Count}'",
                    nameof(payload));
            }

            if (payload[4] != Encoding.Major || payload[5] != Encoding.Minor)
            {
                throw new ArgumentException($"the payload encoding `{payload[4]}.{payload[5]}' must be the same " +
                                            $"as the proxy encoding `{Encoding.Major}.{Encoding.Minor}'",
                    nameof(payload));
            }
            Data[^1] = Data[^1].Slice(0, _payloadStart.Offset);
            Data.Add(payload);
            _payloadEnd = new OutputStream.Position(Data.Count - 1, payload.Count);
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        private OutgoingRequestFrame(IObjectPrx proxy, string operation, bool idempotent,
                                     IReadOnlyDictionary<string, string>? context,
                                     bool writeEmptyParamList = false)
        {
            Encoding = proxy.Encoding;
            Data = new List<ArraySegment<byte>>();
            proxy.IceReference.Protocol.CheckSupported();

            Identity = proxy.Identity;
            Facet = proxy.Facet;
            Operation = operation;
            IsIdempotent = idempotent;
            var ostr = new OutputStream(Encoding.V1_1, Data, new OutputStream.Position(0, 0));
            Identity.IceWrite(ostr);
            ostr.WriteFacet(Facet);
            ostr.WriteString(operation);
            ostr.Write(idempotent ? OperationMode.Idempotent : OperationMode.Normal);

            if (context != null)
            {
                Context = new Dictionary<string, string>(context);
            }
            else
            {
                Context = new Dictionary<string, string>(proxy.Communicator.CurrentContext);
                foreach ((string key, string value) in proxy.Context)
                {
                    Context[key] = value; // the proxy Context entry prevails.
                }
            }

            ContextHelper.Write(ostr, Context);
            _payloadStart = ostr.Tail;

            if (writeEmptyParamList)
            {
                Encoding.CheckSupported();
                _payloadEnd = ostr.WriteEmptyEncapsulation(Encoding);
                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        private void Finish(OutputStream.Position payloadEnd)
        {
            Size = Data.GetByteCount();
            _payloadEnd = payloadEnd;
            IsSealed = true;
        }
    }
}
