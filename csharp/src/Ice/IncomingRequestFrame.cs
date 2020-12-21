// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;

namespace ZeroC.Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    public sealed class IncomingRequestFrame : IncomingFrame, IDisposable
    {
        /// <inheritdoc/>
        public override IReadOnlyDictionary<int, ReadOnlyMemory<byte>> BinaryContext { get; } =
            ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;

        /// <summary>The request context.</summary>
        public SortedDictionary<string, string> Context { get; }

        /// <summary>The deadline corresponds to the request's expiration time. Once the deadline is reached, the
        /// caller is no longer interested in the response and discards the request. The server-side runtime does not
        /// enforce this deadline - it's provided "for information" to the application. The Ice client runtime sets
        /// this deadline automatically using the proxy's invocation timeout and sends it with ice2 requests but not
        /// with ice1 requests. As a result, the deadline for an ice1 request is always <see cref="DateTime.MaxValue"/>
        /// on the server-side even though the invocation timeout is usually not infinite.</summary>
        public DateTime Deadline { get; }

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

        /// <inheritdoc/>
        public override Encoding PayloadEncoding { get; }

        /// <summary>The priority of this request.</summary>
        public Priority Priority { get; }

        // The optional socket stream. The stream is non-null if there's still data to read over the stream
        // after the reading of the request frame.
        internal SocketStream? SocketStream { get; set; }

        /// <summary>Constructs an incoming request frame.</summary>
        /// <param name="protocol">The Ice protocol.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompression.</param>
        public IncomingRequestFrame(Protocol protocol, ArraySegment<byte> data, int maxSize)
            : this(protocol, data, maxSize, null)
        {
        }

        /// <summary>Releases resources used by the request frame.</summary>
        public void Dispose() => SocketStream?.TryDispose();

        /// <summary>Reads the arguments from the request and makes sure this request carries no argument or only
        /// unknown tagged arguments.</summary>
        public void ReadEmptyArgs()
        {
            if (PayloadCompressionFormat != CompressionFormat.Decompressed)
            {
                DecompressPayload();
            }

            if (SocketStream != null)
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
            if (PayloadCompressionFormat != CompressionFormat.Decompressed)
            {
                DecompressPayload();
            }

            if (SocketStream != null)
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
            if (PayloadCompressionFormat != CompressionFormat.Decompressed)
            {
                DecompressPayload();
            }

            if (SocketStream == null)
            {
                throw new InvalidDataException("no stream data available for operation with stream parameter");
            }

            Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding());
            T value = reader(SocketStream);
            // Clear the socket stream to ensure it's not disposed with the request frame. It's now the
            // responsibility of the stream parameter object to dispose the socket stream.
            SocketStream = null;
            return value;
        }

        /// <summary>Reads the arguments from a request. The arguments include a stream argument.</summary>
        /// <paramtype name="T">The type of the arguments.</paramtype>
        /// <param name="connection">The current connection.</param>
        /// <param name="reader">The delegate used to read the arguments.</param>
        /// <returns>The request arguments.</returns>
        public T ReadArgs<T>(Connection connection, InputStreamReaderWithStreamable<T> reader)
        {
            if (PayloadCompressionFormat != CompressionFormat.Decompressed)
            {
                DecompressPayload();
            }

            if (SocketStream == null)
            {
                throw new InvalidDataException("no stream data available for operation with stream parameter");
            }

            var istr = new InputStream(Payload.AsReadOnlyMemory(),
                                       Protocol.GetEncoding(),
                                       connection: connection,
                                       startEncapsulation: true);
            T value = reader(istr, SocketStream);
            // Clear the socket stream to ensure it's not disposed with the request frame. It's now the
            // responsibility of the stream parameter object to dispose the socket stream.
            SocketStream = null;
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
            : base(protocol, maxSize)
        {
            SocketStream = socketStream;

            var istr = new InputStream(data, Protocol.GetEncoding());

            if (Protocol == Protocol.Ice1)
            {
                var requestHeader = new Ice1RequestHeader(istr);
                Identity = requestHeader.Identity;
                Facet = Ice1Definitions.GetFacet(requestHeader.FacetPath);
                Location = Array.Empty<string>();
                Operation = requestHeader.Operation;
                IsIdempotent = requestHeader.OperationMode != OperationMode.Normal;
                Context = requestHeader.Context;
                Priority = default;
                Deadline = DateTime.MaxValue;
            }
            else
            {
                int headerSize = istr.ReadSize();
                int startPos = istr.Pos;

                // We use the generated code for the header body and read the rest of the header "by hand".
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
                Context = requestHeaderBody.Context ?? new SortedDictionary<string, string>();

                BinaryContext = istr.ReadBinaryContext();

                if (istr.Pos - startPos != headerSize)
                {
                    throw new InvalidDataException(
                        @$"received invalid request header: expected {headerSize} bytes but read {istr.Pos - startPos
                        } bytes");
                }

                if (Location.Any(segment => segment.Length == 0))
                {
                    throw new InvalidDataException("received request with an empty location segment");
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

            Payload = data.Slice(istr.Pos);

            PayloadEncoding = istr.ReadEncapsulationHeader(checkFullBuffer: true).Encoding;
            if (PayloadEncoding == Encoding.V20)
            {
                PayloadCompressionFormat = istr.ReadCompressionFormat();
            }
        }

        /// <summary>Constructs an incoming request frame from an outgoing request frame. Used for colocated calls.
        /// </summary>
        /// <param name="request">The outgoing request frame.</param>
        internal IncomingRequestFrame(OutgoingRequestFrame request)
            : base(request.Protocol, int.MaxValue)
        {
            Identity = request.Identity;
            Facet = request.Facet;
            Location = request.Location;
            Operation = request.Operation;
            IsIdempotent = request.IsIdempotent;
            Context = new SortedDictionary<string, string>((IDictionary<string, string>)request.Context);

            Priority = default;
            Deadline = request.Deadline;

            if (Protocol == Protocol.Ice2)
            {
                BinaryContext = request.GetBinaryContext();
            }

            PayloadEncoding = request.PayloadEncoding;

            Payload = request.Payload.AsArraySegment();
            PayloadCompressionFormat = request.PayloadCompressionFormat;
        }
    }
}
