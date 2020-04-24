//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    public sealed class IncomingRequestFrame
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public IReadOnlyDictionary<string, string> Context { get; }
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }
        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }
        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }
        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }
        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>The payload of this request frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Payload { get; }

        /// <summary>The frame byte count</summary>
        public int Size => Data.Count;

        internal ArraySegment<byte> Data { get; }
        private readonly Communicator _communicator;

        /// <summary>Creates a new OutgoingRequestFrame.</summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="data">The frame data as an array segment.</param>
        public IncomingRequestFrame(Communicator communicator, ArraySegment<byte> data)
        {
            _communicator = communicator;
            Data = data;
            var istr = new InputStream(communicator, data);

            Identity = new Identity(istr);
            Facet = istr.ReadFacet();
            Operation = istr.ReadString();
            IsIdempotent = istr.ReadOperationMode() != OperationMode.Normal;
            Context = istr.ReadContext();
            Payload = Data.Slice(istr.Pos);
            (Encoding encoding, int size) = istr.ReadEncapsulationHeader();
            if (size != Payload.Count)
            {
                throw new InvalidDataException($"invalid encapsulation size: {size}");
            }
            Encoding = encoding;
        }

        // TODO avoid copy payload (ToArray) creates a copy, that should be possible when
        // the frame has a single segment.
        internal IncomingRequestFrame(Communicator communicator, OutgoingRequestFrame frame)
            : this(communicator, VectoredBufferExtensions.ToArray(frame.Data))
        {
        }

        /// <summary>Reads the empty parameter list, calling this methods ensure that the frame payload
        /// correspond to the empty parameter list.</summary>
        public void ReadEmptyParamList() => InputStream.ReadEmptyEncapsulation(_communicator, Payload);

        /// <summary>Reads the request frame parameter list.</summary>
        /// <param name="reader">An InputStreamReader delegate used to read the request frame
        /// parameters.</param>
        /// <returns>The request parameters, when the frame parameter list contains multiple parameters
        /// they must be return as a tuple.</returns>
        public T ReadParamList<T>(InputStreamReader<T> reader) =>
            InputStream.ReadEncapsulation(_communicator, Payload, reader);
    }
}
