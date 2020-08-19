//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutgoingFrame
    {
        /// <summary>The context of this request frame.</summary>
        public IReadOnlyDictionary<string, string> Context => ContextOverride ?? _initialContext;

        /// <summary>When non-null, ContextOverride provides the value for Context.</summary>
        public Dictionary<string, string>? ContextOverride
        {
            get => _contextOverride;
            set
            {
                if (Protocol == Protocol.Ice1)
                {
                    throw new InvalidOperationException("cannot change the context of an ice1 request frame");
                }
                _contextOverride = value;
            }
        }

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
        private readonly IReadOnlyDictionary<string, string> _initialContext;

        /// <summary>Create a new OutgoingRequestFrame.</summary>
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
        public static OutgoingRequestFrame WithParamList<T>(
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
                                        request._encapsulationStart,
                                        request.Encoding,
                                        format);
            writer(ostr, value);
            request.FinishEncapsulation(ostr.Finish());
            if (compress && proxy.Encoding == Encoding.V2_0)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Create a new OutgoingRequestFrame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format used to marshal classes.</param>
        /// <param name="context">An optional explicit context. When non null, it overrides both the context of the
        /// proxy and the communicator's current context (if any).</param>
        /// <param name="value">The parameter to marshal in the frame, when the request frame contain multiple
        /// parameters they must be passed as a tuple.</param>
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
                                        request._encapsulationStart,
                                        request.Encoding,
                                        format);
            writer(ostr, value);
            request.FinishEncapsulation(ostr.Finish());
            if (compress && proxy.Encoding == Encoding.V2_0)
            {
                request.CompressPayload();
            }
            return request;
        }

        /// <summary>Creates a new outgoing request frame with no parameters.</summary>
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
                Data[^1] = Data[^1].Slice(0, _encapsulationStart.Offset);

                // We only include the encapsulation.
                Data.Add(request.Data.Slice(
                    request.Encapsulation.Offset - request.Data.Offset, request.Encapsulation.Count));
                _encapsulationEnd = new OutputStream.Position(Data.Count - 1, request.Encapsulation.Count);

                if (Protocol == Protocol.Ice2 && forwardBinaryContext)
                {
                    // Can be empty
                    _defaultBinaryContext = request.Data.Slice(
                        request.Encapsulation.Offset - request.Data.Offset + request.Encapsulation.Count);
                }
            }
            else
            {
                // We forward the encapsulation and the string-string context. The context was marshaled by the
                // constructor (when Protocol == Ice1) or will be written by Finish (when Protocol == Ice2).
                // The payload encoding must remain the same since we cannot transcode the encoded bytes.

                int sizeLength = request.Protocol == Protocol.Ice1 ? 4 : (1 << (request.Encapsulation[0] & 0x03));

                OutputStream.Position tail =
                    OutputStream.WriteEncapsulationHeader(Data,
                                                          _encapsulationStart,
                                                          Protocol.GetEncoding(),
                                                          request.Encapsulation.Count - sizeLength,
                                                          request.Encoding);

                // Finish off current segment
                Data[^1] = Data[^1].Slice(0, tail.Offset);

                // "2" below corresponds to the encoded length of the encoding.
                if (request.Encapsulation.Count > sizeLength + 2)
                {
                    // Add encoded bytes, not including the header or binary context.
                    Data.Add(request.Encapsulation.Slice(sizeLength + 2));

                    _encapsulationEnd =
                        new OutputStream.Position(Data.Count - 1, request.Encapsulation.Count - sizeLength - 2);
                }
                else
                {
                    _encapsulationEnd = tail;
                }
            }

            Size = Data.GetByteCount();
            IsSealed = Protocol == Protocol.Ice1;
        }

        // Finish prepare the frame for sending, write the frame's context into the slot 0 of the binary context.
        internal override void Finish()
        {
            if (!IsSealed)
            {
                if (Protocol == Protocol.Ice2 && !_binaryContextKeys.Contains(0))
                {
                    if (ContextOverride != null && ContextOverride.Count == 0)
                    {
                       // Make sure the slot is used so base does not write anything in slot 0 when forwarding the
                       // binary context
                       _binaryContextKeys.Add(0);
                    }
                    else if (Context.Count > 0)
                    {
                        // Write the context now. This is necessary even when _defaultBinaryContext is not empty,
                        // because _initialContext can be different from slot 0 in _defaultBinaryContext.
                        AddBinaryContextEntry(0, Context, ContextHelper.IceWriter);
                    }
                    // else we don't write Context since it's empty
                }
                base.Finish();
            }
        }

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
                ContextHelper.Write(ostr, _initialContext);
            }
            _encapsulationStart = ostr.Tail;

            if (writeEmptyParamList)
            {
                FinishEncapsulation(ostr.WriteEmptyEncapsulation(Encoding));
            }
        }
    }
}
