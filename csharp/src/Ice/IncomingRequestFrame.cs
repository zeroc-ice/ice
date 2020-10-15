// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    public sealed class IncomingRequestFrame : IncomingFrame
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Dictionary<string, string> Context { get; }

        /// <summary>The request deadline. The peer sets the deadline to the absolute time at which its invocation
        /// timeout will be triggered. With Ice1 the peer doesn't send a deadline and it always has the default value.
        /// </summary>
        public DateTime Deadline { get; }

        /// <summary>The encoding of the frame payload.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The facet of the target Ice object.</summary>
        public string Facet { get; }

        /// <summary>The identity of the target Ice object.</summary>
        public Identity Identity { get; }

        /// <summary>When true, the operation is idempotent.</summary>
        public bool IsIdempotent { get; }

        /// <summary>The location of the target Ice object. With ice1, it is always empty.</summary>
        public string[] Location { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>The priority of this request.</summary>
        public Priority Priority { get; }

        /// <summary>Constructs an incoming request frame.</summary>
        /// <param name="protocol">The Ice protocol.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="sizeMax">The maximum payload size, checked during decompression.</param>
        public IncomingRequestFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
            : base(data, protocol, sizeMax)
        {
            var istr = new InputStream(Data, Protocol.GetEncoding());

            if (Protocol == Protocol.Ice1)
            {
                var requestHeaderBody = new Ice1RequestHeaderBody(istr);
                Identity = requestHeaderBody.Identity;
                Facet = Ice1Definitions.GetFacet(requestHeaderBody.FacetPath);
                Location = Array.Empty<string>();
                Operation = requestHeaderBody.Operation;
                IsIdempotent = requestHeaderBody.OperationMode != OperationMode.Normal;
                Context = requestHeaderBody.Context;
                Priority = default;
            }
            else
            {
                var requestHeaderBody = new Ice2RequestHeaderBody(istr);
                Identity = requestHeaderBody.Identity;
                Facet = requestHeaderBody.Facet ?? "";
                Location = requestHeaderBody.Location ?? Array.Empty<string>();
                Operation = requestHeaderBody.Operation;
                IsIdempotent = requestHeaderBody.Idempotent ?? false;
                Priority = requestHeaderBody.Priority ?? default;
                var deadline = TimeSpan.FromMilliseconds(requestHeaderBody.Deadline);
                // The infinite invocation timeout is encoded as 0 and converted to DateTime.MaxValue
                Deadline = deadline == Timeout.InfiniteTimeSpan ?
                    DateTime.MaxValue : DateTime.UnixEpoch + TimeSpan.FromMilliseconds(requestHeaderBody.Deadline);
                Context = null!; // initialized below

                if (Location.Any(segment => segment.Length == 0))
                {
                   throw new InvalidDataException("received request with empty location segment");
                }
            }

            if (Identity.Name.Length == 0)
            {
                throw new InvalidDataException("received request with null identity");
            }

            if (Operation.Length == 0)
            {
                throw new InvalidDataException("received request with empty operation name");
            }

            (int size, int sizeLength, Encoding encoding) =
                Data.Slice(istr.Pos).AsReadOnlySpan().ReadEncapsulationHeader(Protocol.GetEncoding());

            Payload = Data.Slice(istr.Pos, size + sizeLength); // the payload is the encapsulation

            if (Protocol == Protocol.Ice2)
            {
                // BinaryContext is a computed property that depends on Payload.
                if (BinaryContext.TryGetValue(0, out ReadOnlyMemory<byte> value))
                {
                    Context = value.Read(istr => istr.ReadDictionary(minKeySize: 1,
                                                                     minValueSize: 1,
                                                                     InputStream.IceReaderIntoString,
                                                                     InputStream.IceReaderIntoString));
                }
                else
                {
                    Context = new Dictionary<string, string>();
                }
            }

            if (protocol == Protocol.Ice1 && size + 4 + istr.Pos != data.Count)
            {
                // The payload holds an encapsulation and the encapsulation must use up the full buffer with ice1.
                // "4" corresponds to fixed-length size with the 1.1 encoding.
                throw new InvalidDataException($"invalid request encapsulation size: {size}");
            }

            Encoding = encoding;
            HasCompressedPayload = Encoding == Encoding.V20 && Payload[sizeLength + 2] != 0;
        }

        /// <summary>Reads the arguments from the request and makes sure this request carries no argument or only
        /// unknown tagged arguments.</summary>
        public void ReadEmptyArgs()
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }
            Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding());
        }

        /// <summary>Reads the arguments from a request.</summary>
        /// <paramtype name="T">The type of the arguments.</paramtype>
        /// <param name="communicator">The communicator.</param>
        /// <param name="reader">The delegate used to read the arguments.</param>
        /// <returns>The request arguments.</returns>
        public T ReadArgs<T>(Communicator communicator, InputStreamReader<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }
            return Payload.AsReadOnlyMemory().ReadEncapsulation(Protocol.GetEncoding(), communicator, reader);
        }

        private protected override ArraySegment<byte> GetEncapsulation() => Payload;
    }
}
