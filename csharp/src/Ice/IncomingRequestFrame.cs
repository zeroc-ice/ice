//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    public sealed class IncomingRequestFrame : IncomingFrame
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public IReadOnlyDictionary<string, string> Context { get; }
        /// <summary>The encoding of the frame payload.</summary>
        public override Encoding Encoding { get; }
        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }
        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }
        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>Creates a new IncomingRequestFrame.</summary>
        /// <param name="protocol">The Ice protocol.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="sizeMax">The maximum payload size, checked during decompress.</param>
        public IncomingRequestFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
            : base(protocol, data, sizeMax)
        {
            var istr = new InputStream(data, Protocol.GetEncoding());
            Identity = new Identity(istr);
            Facet = istr.ReadFacet();
            Operation = istr.ReadString();
            IsIdempotent = istr.ReadOperationMode() != OperationMode.Normal;
            Context = istr.ReadContext();
            Payload = data.Slice(istr.Pos);
            (int size, int sizeLength, Encoding encoding) =
                Payload.AsReadOnlySpan().ReadEncapsulationHeader(Protocol.GetEncoding());

            if (protocol == Protocol.Ice1)
            {
                if (size + 4 != Payload.Count)
                {
                    // The payload holds an encapsulation and the encapsulation must use up the full buffer with ice1.
                    // "4" corresponds to fixed-length size with the 1.1 encoding.
                    throw new InvalidDataException($"invalid request encapsulation size: {size}");
                }
            }
            else
            {
                // TODO: with ice2, the payload is followed by a context, and the size is not fixed-length.
            }

            Encoding = encoding;
            HasCompressedPayload = Encoding == Encoding.V2_0 && Payload[sizeLength + 2] != 0;
        }

        // TODO avoid copy payload (ToArray) creates a copy, that should be possible when
        // the frame has a single segment.
        internal IncomingRequestFrame(OutgoingRequestFrame frame, int sizeMax)
            : this(frame.Protocol, VectoredBufferExtensions.ToArray(frame.Data), sizeMax)
        {
        }

        /// <summary>Reads the empty parameter list, calling this methods ensure that the frame payload
        /// correspond to the empty parameter list.</summary>
        public void ReadEmptyParamList()
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }
            Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding());
        }

        /// <summary>Reads the request frame parameter list.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="reader">An InputStreamReader delegate used to read the request frame
        /// parameters.</param>
        /// <returns>The request parameters, when the frame parameter list contains multiple parameters
        /// they must be return as a tuple.</returns>
        public T ReadParamList<T>(Communicator communicator, InputStreamReader<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }
            return Payload.AsReadOnlyMemory().ReadEncapsulation(Protocol.GetEncoding(), communicator, reader);
        }
    }
}
