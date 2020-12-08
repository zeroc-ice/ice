// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Security;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The BufferedReceiveOverSingleStreamSocket is a wrapper around SingleStreamSocket to provide
    /// buffered data receive. This helps to limit the number of operating system Receive calls when the user
    /// needs to read only few bytes before reading more (typically to read a frame header) by receiving the
    /// data in a small buffer. It's similar to the C# System.IO.BufferedStream class. It's used to implement
    /// Slic and WebSocket.
    /// </summary>
    internal class BufferedReceiveOverSingleStreamSocket : SingleStreamSocket
    {
        public override Socket? Socket => Underlying.Socket;
        public override SslStream? SslStream => Underlying.SslStream;

        internal SingleStreamSocket Underlying { get; }

        // The buffered data.
        private ArraySegment<byte> _buffer;

        public override ValueTask CloseAsync(Exception exception, CancellationToken cancel) =>
            Underlying.CloseAsync(exception, cancel);

        public override ValueTask InitializeAsync(CancellationToken cancel) => Underlying.InitializeAsync(cancel);

        public override ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel) =>
            Underlying.ReceiveDatagramAsync(cancel);

        public override async ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel = default)
        {
            int received = 0;
            if (_buffer.Count > 0)
            {
                // If there's still data buffered for the payload, consume the buffered data.
                int length = Math.Min(_buffer.Count, buffer.Length);
                _buffer.Slice(0, length).AsMemory().CopyTo(buffer);
                _buffer = _buffer.Slice(length);
                received = length;
            }

            // Then, read the reminder from the underlying transport.
            if (received < buffer.Length)
            {
                received += await Underlying.ReceiveAsync(buffer.Slice(received), cancel).ConfigureAwait(false);
            }
            return received;
        }

        public override ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel = default) =>
            Underlying.SendAsync(buffer, cancel);

        /// <inheritdoc/>
        public override string? ToString() => Underlying.ToString();

        protected override void Dispose(bool disposing) => Underlying.Dispose();

        internal BufferedReceiveOverSingleStreamSocket(SingleStreamSocket underlying, int bufferSize = 256)
        {
            Underlying = underlying;

            // The _buffer data member holds the buffered data. There's no buffered data until we receive data
            // from the underlying socket so the array segment point to an empty segment.
            _buffer = new ArraySegment<byte>(new byte[bufferSize], 0, 0);
        }

        /// <summary>Returns buffered data. If there's no buffered data, the buffer is filled using the underlying
        /// socket to receive additional data. The method returns when the buffer contains at least byteCount
        /// data. If byteCount is set to zero, it returns all the buffered data.</summary>
        /// <param name="byteCount">The number of bytes of buffered data to return. It can be set to null to get all
        /// the buffered data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The buffered data. The returned data points to an internal buffer that is only valid until the next
        /// ReceiveAsync call.</return>
        internal async ValueTask<ReadOnlyMemory<byte>> ReceiveAsync(int byteCount, CancellationToken cancel = default)
        {
            if (byteCount > _buffer.Array!.Length)
            {
                throw new ArgumentOutOfRangeException(
                    $"{nameof(byteCount)} should be inferior to the buffer size of {_buffer.Array.Length} bytes");
            }

            // Receive additional data if there's not enough or no buffered data.
            if (_buffer.Count < byteCount || (byteCount == 0 && _buffer.Count == 0))
            {
                await ReceiveInBufferAsync(byteCount, cancel).ConfigureAwait(false);
                Debug.Assert(_buffer.Count >= byteCount);
            }

            if (byteCount == 0)
            {
                // Return all the buffered data.
                byteCount = _buffer.Count;
            }

            ReadOnlyMemory<byte> buffer = _buffer.Slice(0, byteCount);
            _buffer = _buffer.Slice(byteCount); // Remaining buffered data.
            return buffer;
        }

        /// <summary>Rewinds the buffered data. This can be used if too much buffered data has been read to add
        /// it back to the buffer.</summary>
        /// <param name="bytes">The number of bytes to unread from the buffer.</param>
        internal void Rewind(int bytes)
        {
            if (bytes > _buffer.Offset)
            {
                throw new ArgumentOutOfRangeException($"{nameof(bytes)} is too large");
            }

            _buffer = new ArraySegment<byte>(_buffer.Array!, _buffer.Offset - bytes, _buffer.Count + bytes);
        }

        private async ValueTask ReceiveInBufferAsync(int byteCount, CancellationToken cancel = default)
        {
            Debug.Assert(byteCount == 0 || _buffer.Count < byteCount);

            int offset = _buffer.Count;

            // If there's not enough data buffered for byteCount we need to receive additional data. We first need
            // to make sure there's enough space in the buffer to read it however.
            if (_buffer.Count == 0)
            {
                // Use the full buffer array if there's no more buffered data.
                _buffer = new ArraySegment<byte>(_buffer.Array!);
            }
            else if (_buffer.Offset + _buffer.Count + byteCount > _buffer.Array!.Length)
            {
                // There's still buffered data but not enough space left in the array to read the given bytes.
                // In theory, the number of bytes to read should always be lower than the un-used buffer space
                // at the start of the buffer. We move the data at the end of the buffer to the beginning to
                // make space to read the given number of bytes.
                _buffer.CopyTo(_buffer.Array!, 0);
                _buffer = new ArraySegment<byte>(_buffer.Array);
            }
            else
            {
                // There's still buffered data and enough space to read the given bytes after the buffered data.
                _buffer = new ArraySegment<byte>(
                    _buffer.Array,
                    _buffer.Offset,
                    _buffer.Array.Length - _buffer.Offset);
            }

            // Receive additional data.
            if (byteCount == 0)
            {
                // Perform a single receive and we're done.
                offset += await Underlying.ReceiveAsync(_buffer.Slice(offset), cancel).ConfigureAwait(false);
            }
            else
            {
                // Receive data until we have read at least "byteCount" bytes in the buffer.
                while (offset < byteCount)
                {
                    offset += await Underlying.ReceiveAsync(_buffer.Slice(offset), cancel).ConfigureAwait(false);
                }
            }

            // Set _buffer to the buffered data array segment.
            _buffer = _buffer.Slice(0, offset);
        }
    }
}
