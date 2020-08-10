//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame sent by the application.</summary>
    public sealed class OutgoingRequestFrame : OutgoingFrame
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Dictionary<string, string> Context { get; }

        public override Encoding Encoding { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>Create a new OutgoingRequestFrame.</summary>
        /// <param name="proxy">A proxy to the target Ice object. This method uses the communicator, identity, facet,
        /// encoding and context of this proxy to create the request frame.</param>
        /// <param name="operation">The operation to invoke on the target Ice object.</param>
        /// <param name="idempotent">True when operation is idempotent, otherwise false.</param>
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
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
            FormatType? format,
            IReadOnlyDictionary<string, string>? context,
            T value,
            OutputStreamWriter<T> writer)
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(), request.Data, request._encapsulationStart,
                request.Encoding, format ?? proxy.Communicator.DefaultFormat);
            writer(ostr, value);
            request.FinishEncapsulation(ostr.Save());
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
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
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
            FormatType? format,
            IReadOnlyDictionary<string, string>? context,
            in T value,
            OutputStreamValueWriter<T> writer) where T : struct
        {
            var request = new OutgoingRequestFrame(proxy, operation, idempotent, compress, context);
            var ostr = new OutputStream(proxy.Protocol.GetEncoding(), request.Data, request._encapsulationStart,
                request.Encoding, format ?? proxy.Communicator.DefaultFormat);
            writer(ostr, value);
            request.FinishEncapsulation(ostr.Save());
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
        /// <param name="compress">True if the request should be compressed, false otherwise.</param>
        internal OutgoingRequestFrame(
            IObjectPrx proxy,
            IncomingRequestFrame request,
            bool compress)
            : this(proxy, request.Operation, request.IsIdempotent, compress, request.Context)
        {
            ArraySegment<byte> data = request.Data;
            if (data.Count < 6)
            {
                throw new ArgumentException(
                    $"payload should contain at least 6 bytes, but it contains {data.Count} bytes",
                    nameof(data));
            }
            int size = data.AsReadOnlySpan(0, 4).ReadFixedLengthSize(proxy.Protocol.GetEncoding());
            if (size != data.Count)
            {
                throw new ArgumentException($"invalid payload size `{size}' expected `{data.Count}'",
                    nameof(data));
            }

            if (data[4] != Encoding.Major || data[5] != Encoding.Minor)
            {
                throw new ArgumentException($"the payload encoding `{data[4]}.{data[5]}' must be the same " +
                                            $"as the proxy encoding `{Encoding.Major}.{Encoding.Minor}'",
                    nameof(data));
            }
            Data[^1] = Data[^1].Slice(0, _encapsulationStart.Offset);
            Data.Add(data);
            FinishEncapsulation(new OutputStream.Position(Data.Count - 1, data.Count));

            Size = Data.GetByteCount();
            IsSealed = true;
        }

        internal override void FinishBinaryContext()
        {
            if (Protocol == Protocol.Ice2 && !IsSealed)
            {
                if (Context.Count > 0)
                {
                    AddBinaryContextEntry(0, Context, ContextHelper.IceWriter);
                }
                base.FinishBinaryContext();
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

            if (Protocol == Protocol.Ice1)
            {
                ContextHelper.Write(ostr, Context);
            }
            _encapsulationStart = ostr.Tail;

            if (writeEmptyParamList)
            {
                FinishEncapsulation(ostr.WriteEmptyEncapsulation(Encoding));
            }
        }
    }
}
