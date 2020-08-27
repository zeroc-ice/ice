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
    /// <summary>The BufferedReadTransceiver is a wrapper around the ITransceiver interface to provide buffered
    /// receive. This helps to limit the number of operating system Receive calls when the user needs to read
    /// only few bytes before reading more (typically to read a frame header) by receiving the data in a small
    /// buffer. It's similar to the C# System.IO.BufferedStream class. It's used to implement Slic and WebSocket.
    /// </summary>
    internal class BufferedReadTransceiver : ITransceiver
    {
        private ArraySegment<byte> _buffer;

        public Socket? Socket => Underlying.Socket;

        public ITransceiver Underlying { get; }

        public void CheckSendSize(int size) => Underlying.CheckSendSize(size);

        public ValueTask CloseAsync(Exception exception, CancellationToken cancel) =>
            Underlying.CloseAsync(exception, cancel);

        public ValueTask DisposeAsync() => Underlying.DisposeAsync();

        public ValueTask InitializeAsync(CancellationToken cancel) => Underlying.InitializeAsync(cancel);

        public async ValueTask<ArraySegment<byte>> ReceiveAsync(CancellationToken cancel)
        {
            if (_buffer.Count == 0)
            {
                await ReceiveInBufferAsync(0, cancel);
            }
            ArraySegment<byte> buffer = _buffer.ToArray();
            _buffer = new ArraySegment<byte>(_buffer.Array!, 0, 0);
            return buffer;
        }

        public async ValueTask<int> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel = default)
        {
            int received = 0;
            if (_buffer.Count > 0)
            {
                // If there's still data buffered for the payload, consume the buffered data.
                int length = Math.Min(_buffer.Count, buffer.Count);
                ArraySegment<byte> buffered = await ReceiveAsync(length, cancel).ConfigureAwait(false);
                buffered.CopyTo(buffer);
                received = length;
            }

            // Then, read the reminder from the underlying transport.
            if (received < buffer.Count)
            {
                received += await Underlying.ReceiveAsync(buffer.Slice(received), cancel).ConfigureAwait(false);
            }
            return received;
        }

        public async ValueTask<byte> ReceiveByteAsync(CancellationToken cancel = default)
        {
            if (_buffer.Count > 0)
            {
                byte b = _buffer[0];
                _buffer = _buffer.Slice(1);
                return b;
            }
            else
            {
                return (await ReceiveAsync(1, cancel).ConfigureAwait(false))[0];
            }
        }

        public async ValueTask<ArraySegment<byte>> ReceiveAsync(int byteCount, CancellationToken cancel = default)
        {
            if (byteCount > _buffer.Array!.Length)
            {
                throw new ArgumentOutOfRangeException(
                    $"byteCount should be inferior to the buffer size of {_buffer.Array.Length} bytes");
            }

            if (_buffer.Count < byteCount)
            {
                await ReceiveInBufferAsync(byteCount, cancel);
                Debug.Assert(_buffer.Count >= byteCount);
            }

            ArraySegment<byte> buffer = _buffer.Slice(0, byteCount);
            _buffer = _buffer.Slice(byteCount);
            return buffer;
        }

        public ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel = default) =>
            Underlying.SendAsync(buffer, cancel);

        /// <summary>Set the buffered data. This can be used if too much buffered data has been read to add
        /// it back to the buffer. This can only be used if the buffered data was all consumed.</summary>
        /// <param name="data">The data to add back to the buffer.</param>
        public void SetBuffer(ArraySegment<byte> data)
        {
            if (_buffer.Count != 0)
            {
                throw new ArgumentOutOfRangeException("buffer is not empty");
            }
            if (_buffer.Array!.Length < data.Count)
            {
                throw new ArgumentOutOfRangeException("data is too large");
            }

            _buffer.CopyTo(_buffer.Array!, 0);
            data.CopyTo(_buffer.Array!, _buffer.Count);
            _buffer = new ArraySegment<byte>(_buffer.Array, 0, _buffer.Count + data.Count);
        }
        public string ToDetailedString() => Underlying.ToDetailedString();

        public override string? ToString() => Underlying.ToString();

        internal BufferedReadTransceiver(ITransceiver underlying, int bufferSize = 256)
        {
            Underlying = underlying;
            _buffer = new ArraySegment<byte>(new byte[bufferSize], 0, 0);
        }

        private async ValueTask ReceiveInBufferAsync(int byteCount, CancellationToken cancel = default)
        {
            Debug.Assert(byteCount == 0 || _buffer.Count < byteCount);

            int offset = _buffer.Count;

            // If there's not enough data buffered for byteCount we read more data in the buffer. We first need
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
                // at the start of the buffer. We move the data at the end of the buffer to the begining to
                // make space to read the given number of bytes.
                Debug.Assert(_buffer.Offset >= byteCount);
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

            if (byteCount == 0)
            {
                offset += await Underlying.ReceiveAsync(_buffer.Slice(offset), cancel);
            }
            else
            {
                while (offset < byteCount)
                {
                    offset += await Underlying.ReceiveAsync(_buffer.Slice(offset), cancel);
                }
            }

            _buffer = _buffer.Slice(0, offset);
        }
    }
}
