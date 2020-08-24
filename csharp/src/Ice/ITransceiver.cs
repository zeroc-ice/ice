//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>A transceiver enables transmitting and receiving raw binary data over a transport such as TCP,
    /// UDP, TLS or WebSocket. More transports can be supported by implementing this interface.</summary>
    public interface ITransceiver : IAsyncDisposable
    {
        /// <summary>Gets the optional socket associated with this transceiver.</summary>
        Socket? Socket { get; }

        /// <summary>Checks if the transceiver can send messages of the given size. Throw if the message is too large.
        /// </summary>
        /// <param name="size">The size of the message to check.</param>
        // TODO: Remove this? This is used to ensure the user doesn't try to send a message which is larger
        // than the datagram size... UDP users would be better of not sending large datagrams instead of us
        // trying to softly error if the user sends a large datagram.
        void CheckSendSize(int size);

        /// <summary>Closes the transceiver. The transceiver might use this method to send a notification to the peer
        /// of the connection closure.</summary>
        /// <param name="exception">The reason of the connection closure.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        /// <summary>Initializes the transceiver. The transceiver might use this method to establish or accept the
        /// connection.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        ValueTask InitializeAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for datagram connections only.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The received data.</return>
        ValueTask<ArraySegment<byte>> ReceiveAsync(CancellationToken cancel);

        /// <summary>Receives data from the connection. This is used for stream based connections only.</summary>
        /// <param name="buffer">The buffer that holds the received data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes received.</return>
        ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel);

        /// <summary>Receive data from the connection.</summary>
        /// <param name="buffer">The buffer containing the data to send.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The number of bytes sent.</return>
        ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel);

        /// <summary>Gets a detailed description of the connection.</summary>
        /// <return>The detailed description.</return>
        string ToDetailedString();
    }

    internal class BufferedReadTransceiver : ITransceiver
    {
        private ArraySegment<byte> _buffer;

        public ITransceiver Underlying { get; }

        public Socket? Socket => Underlying.Socket;

        public void CheckSendSize(int size) => Underlying.CheckSendSize(size);

        public ValueTask CloseAsync(Exception exception, CancellationToken cancel) =>
            Underlying.CloseAsync(exception, cancel);

        public ValueTask DisposeAsync() => Underlying.DisposeAsync();

        public ValueTask InitializeAsync(CancellationToken cancel) => Underlying.InitializeAsync(cancel);

        public ValueTask<ArraySegment<byte>> ReceiveAsync(CancellationToken cancel) =>
            Underlying.ReceiveAsync(cancel);

        public async ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel)
        {
            int received = 0;
            if (_buffer.Count > 0)
            {
                // If there's still data buffered for the payload, consume the buffered data.
                int length = Math.Min(_buffer.Count, buffer.Length);
                ReadOnlyMemory<byte> buffered = await BufferedReceiveAsync(length, cancel).ConfigureAwait(false);
                buffered.CopyTo(buffer);
                received = length;
            }

            // Then, read the reminder from the underlying transport.
            if (received < buffer.Length)
            {
                received += await Underlying.ReceiveAsync(buffer.Slice(received), cancel).ConfigureAwait(false);
            }
            return received;
        }

        public ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel) =>
            Underlying.SendAsync(buffer, cancel);

        public string ToDetailedString() => Underlying.ToDetailedString();

        public async ValueTask<ReadOnlyMemory<byte>> BufferedReceiveAsync(int byteCount, CancellationToken cancel)
        {
            if (byteCount > _buffer.Array!.Length)
            {
                throw new ArgumentOutOfRangeException(
                    $"byteCount should be inferior to the buffer size of {_buffer.Array.Length} bytes");
            }

            int offset = _buffer.Count;
            if (_buffer.Count < byteCount)
            {
                // If there's not enough data buffered for byteCount we read more data in the buffer. We first
                // need to make sure there's enough space in the buffer to read it however.
                if (_buffer.Count == 0)
                {
                    // Use the full buffer array if there's no more buffered data.
                    _buffer = new ArraySegment<byte>(_buffer.Array!);
                }
                else if (_buffer.Offset + _buffer.Count + byteCount > _buffer.Array!.Length)
                {
                    // There's still buffered data but not enough space left in the array to read the given bytes.
                    // In theory, the number of bytes to read should always be lower than the un-used buffer space
                    // at the start of the buffer. We move the data at the end of the buffer to the begining to
                    // make space to read the given number of bytes.
                    Debug.Assert(_buffer.Offset >= byteCount);
                    _buffer.CopyTo(_buffer.Array!, 0);
                    _buffer = new ArraySegment<byte>(_buffer.Array);
                }
                else
                {
                    // There's still buffered data and enough space to read the given bytes after the buffered
                    // data.
                    _buffer = new ArraySegment<byte>(
                        _buffer.Array,
                        _buffer.Offset,
                        _buffer.Array.Length - _buffer.Offset);
                }

                while (offset < byteCount)
                {
                    offset += await Underlying.ReceiveAsync(_buffer.Slice(offset), cancel);
                }
            }

            ArraySegment<byte> buffer = _buffer.Slice(0, byteCount);
            if (byteCount < offset)
            {
                _buffer = _buffer.Slice(byteCount, offset - byteCount);
            }
            else
            {
                _buffer = _buffer.Slice(0, 0);
            }
            return buffer;
        }

        internal BufferedReadTransceiver(ITransceiver underlying, int bufferSize = 256)
        {
            Underlying = underlying;
            _buffer = new byte[bufferSize];
        }
    };

    internal class TransceiverReadStream : System.IO.Stream
    {
        private readonly ITransceiver _transceiver;

        public override long Position
        {
            get => throw new NotSupportedException();
            set => throw new NotSupportedException();
        }

        public override long Length => throw new NotSupportedException();
        public override bool CanWrite => false;
        public override bool CanRead => true;
        public override bool CanSeek => false;

        public override void Flush() => throw new NotSupportedException();
        public override int Read(byte[] buffer, int offset, int count) => throw new NotSupportedException();
        public override ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken cancellationToken) =>
            _transceiver.ReceiveAsync(buffer, cancellationToken);
        public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();
        public override void SetLength(long value) => throw new NotSupportedException();
        public override void Write(byte[] buffer, int offset, int count) => throw new NotSupportedException();
        public override ValueTask WriteAsync(ReadOnlyMemory<byte> buffer, CancellationToken cancellationToken) =>
            throw new NotSupportedException();

        internal TransceiverReadStream(ITransceiver transceiver) => _transceiver = transceiver;
    };
}
