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
        public IReadOnlyDictionary<string, string> Context { get; }
        public Encoding Encoding { get; }
        public string Facet { get; }
        public Identity Identity { get; }
        public bool IsIdempotent { get; }
        public string Operation { get; }

        public int Size => Data.Count;

        /// <summary>The payload of this request frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Payload { get; }

        internal ArraySegment<byte> Data { get; private set; }
        private readonly Communicator _communicator;

        public IncomingRequestFrame(Communicator communicator, ArraySegment<byte> data)
        {
            _communicator = communicator;
            Data = data;
            var istr = new InputStream(communicator, data);

            Identity = new Identity(istr);

            // For compatibility with the old FacetPath.
            string[] facetPath = istr.ReadStringArray();
            if (facetPath.Length > 1)
            {
                throw new InvalidDataException($"invalid facet path length: {facetPath.Length}");
            }
            Facet = facetPath.Length == 0 ? "" : facetPath[0];
            Operation = istr.ReadString();
            IsIdempotent = istr.ReadOperationMode() != OperationMode.Normal;
            Context = istr.ReadContext();
            Payload = Data.Slice(istr.Pos);
            Encoding = istr.StartEncapsulation();
        }

        public void ReadEmptyParamList()
        {
            var istr = new InputStream(_communicator, Payload);
            istr.StartEncapsulation();
            istr.EndEncapsulation();
        }

        public T ReadParamList<T>(InputStreamReader<T> reader)
        {
            var istr = new InputStream(_communicator, Payload);
            istr.StartEncapsulation();
            T paramList = reader(istr);
            istr.EndEncapsulation();
            return paramList;
        }
    }
}
