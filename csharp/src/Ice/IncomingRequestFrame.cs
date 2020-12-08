// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    public sealed class IncomingRequestFrame : IncomingFrame, IDisposable
    {
        /// <summary>The request context. Its initial value is computed when the request frame is created.</summary>
        public Dictionary<string, string> Context { get; }

        /// <summary>The deadline corresponds to the request's expiration time. Once the deadline is reached, the
        /// caller is no longer interested in the response and discards the request. The server-side runtime does not
        /// enforce this deadline - it's provided "for information" to the application. The Ice client runtime sets
        /// this deadline automatically using the proxy's invocation timeout and sends it with ice2 requests but not
        /// with ice1 requests. As a result, the deadline for an ice1 request is always <see cref="DateTime.MaxValue"/>
        /// on the server-side even though the invocation timeout is usually not infinite.</summary>
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
        public IReadOnlyList<string> Location { get; }

        /// <summary>The operation called on the Ice object.</summary>
        public string Operation { get; }

        /// <summary>The priority of this request.</summary>
        public Priority Priority { get; }

        // The optional socket stream. The stream is non-null if there's still data to read over the stream
        // after the reading of the request frame.
        private SocketStream? _socketStream;

        /// <summary>Constructs an incoming request frame.</summary>
        /// <param name="protocol">The Ice protocol.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompression.</param>
        public IncomingRequestFrame(Protocol protocol, ArraySegment<byte> data, int maxSize)
            : this(protocol, data, maxSize, null)
        {
        }

        /// <summary>Releases resources used by the request frame.</summary>
        public void Dispose() => _socketStream?.TryDispose();

        /// <summary>Reads the arguments from the request and makes sure this request carries no argument or only
        /// unknown tagged arguments.</summary>
        public void ReadEmptyArgs()
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream != null)
            {
                throw new InvalidDataException("stream data available for operation without stream parameter");
            }

            Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding());
        }

        /// <summary>Reads the arguments from a request.</summary>
        /// <paramtype name="T">The type of the arguments.</paramtype>
        /// <param name="connection">The current connection.</param>
        /// <param name="reader">The delegate used to read the arguments.</param>
        /// <returns>The request arguments.</returns>
        public T ReadArgs<T>(Connection connection, InputStreamReader<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream != null)
            {
                throw new InvalidDataException("stream data available for operation without stream parameter");
            }

            return Payload.AsReadOnlyMemory().ReadEncapsulation(Protocol.GetEncoding(), reader, connection: connection);
        }

        /// <summary>Reads a single stream argument from the request.</summary>
        /// <param name="reader">The delegate used to read the argument.</param>
        /// <returns>The request argument.</returns>
        public T ReadArgs<T>(Func<SocketStream, T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream == null)
            {
                throw new InvalidDataException("no stream data available for operation with stream parameter");
            }

            Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding());
            T value = reader(_socketStream);
            // Clear the socket stream to ensure it's not disposed with the request frame. It's now the
            // responsibility of the stream parameter object to dispose the socket stream.
            _socketStream = null;
            return value;
        }

        /// <summary>Reads the arguments from a request. The arguments include a stream argument.</summary>
        /// <paramtype name="T">The type of the arguments.</paramtype>
        /// <param name="connection">The current connection.</param>
        /// <param name="reader">The delegate used to read the arguments.</param>
        /// <returns>The request arguments.</returns>
        public T ReadArgs<T>(Connection connection, InputStreamReaderWithStreamable<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream == null)
            {
                throw new InvalidDataException("no stream data available for operation with stream parameter");
            }

            var istr = new InputStream(Payload.AsReadOnlyMemory(),
                                       Protocol.GetEncoding(),
                                       connection: connection,
                                       startEncapsulation: true);
            T value = reader(istr, _socketStream);
            // Clear the socket stream to ensure it's not disposed with the request frame. It's now the
            // responsibility of the stream parameter object to dispose the socket stream.
            _socketStream = null;
            istr.CheckEndOfBuffer(skipTaggedParams: true);
            return value;
        }

        /// <summary>Constructs an incoming request frame.</summary>
        /// <param name="protocol">The Ice protocol.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompression.</param>
        /// <param name="socketStream">The optional socket stream. The stream is non-null if there's still data to
        /// read on the stream after the reading the request frame.</param>
        internal IncomingRequestFrame(
            Protocol protocol,
            ArraySegment<byte> data,
            int maxSize,
            SocketStream? socketStream)
            : base(data, protocol, maxSize)
        {
            _socketStream = socketStream;

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
                Deadline = DateTime.MaxValue;
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
                if (requestHeaderBody.Deadline < -1 || requestHeaderBody.Deadline == 0)
                {
                    throw new InvalidDataException($"received invalid deadline value {requestHeaderBody.Deadline}");
                }
                // The infinite deadline is encoded as -1 and converted to DateTime.MaxValue
                Deadline = requestHeaderBody.Deadline == -1 ?
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

        private protected override ArraySegment<byte> GetEncapsulation() => Payload;
    }
}
