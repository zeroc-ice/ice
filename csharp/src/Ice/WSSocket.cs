// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Security;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class WSSocket : SingleStreamSocket
    {
        public override Socket? Socket => _underlying.Socket;
        public override SslStream? SslStream => (_underlying.Underlying as SslSocket)?.SslStream;

        internal IReadOnlyDictionary<string, string> Headers => _parser.GetHeaders();

        private enum OpCode : byte
        {
            Continuation = 0x0,
            Text = 0x1,
            Data = 0x2,
            Close = 0x8,
            Ping = 0x9,
            Pong = 0xA
        }

        private enum ClosureStatusCode : short
        {
            Normal = 1000,
            Shutdown = 1001
        }

        private const byte FlagFinal = 0x80;   // Last frame
        private const byte FlagMasked = 0x80;   // Payload is masked
        private const string IceProtocol = "ice.zeroc.com";
        private const string WsUUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        private static readonly UTF8Encoding _utf8 = new UTF8Encoding(false, true);

        private bool _closing;
        private readonly Communicator _communicator;
        private readonly bool _incoming;
        private readonly string _host;
        private string _key;
        private readonly HttpParser _parser;
        private readonly object _mutex = new object();
        private readonly BufferedReceiveOverSingleStreamSocket _underlying;
        private readonly Random _rand;
        private bool _receiveLastFrame;
        private readonly byte[] _receiveMask = new byte[4];
        private int _receivePayloadLength;
        private int _receivePayloadOffset;
        private string _resource;
        private readonly string _transportName;
        private readonly byte[] _sendMask;
        private readonly IList<ArraySegment<byte>> _sendBuffer;
        private Task _sendTask = Task.CompletedTask;

        public override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            byte[] payload = new byte[2];
            short reason = System.Net.IPAddress.HostToNetworkOrder(
                (short)(exception is ObjectDisposedException ? ClosureStatusCode.Shutdown : ClosureStatusCode.Normal));
            MemoryMarshal.Write(payload, ref reason);

            _closing = true;

            // Send the close frame.
            await SendImplAsync(OpCode.Close, new List<ArraySegment<byte>> { payload }, cancel).ConfigureAwait(false);
        }

        public override async ValueTask InitializeAsync(CancellationToken cancel)
        {
            await _underlying.InitializeAsync(cancel).ConfigureAwait(false);

            try
            {
                // The server waits for the client's upgrade request, the client sends the upgrade request.
                if (!_incoming)
                {
                    // Compose the upgrade request.
                    var sb = new StringBuilder();
                    sb.Append("GET " + _resource + " HTTP/1.1\r\n");
                    sb.Append("Host: " + _host + "\r\n");
                    sb.Append("Upgrade: websocket\r\n");
                    sb.Append("Connection: Upgrade\r\n");
                    sb.Append("Sec-WebSocket-Protocol: " + IceProtocol + "\r\n");
                    sb.Append("Sec-WebSocket-Version: 13\r\n");
                    sb.Append("Sec-WebSocket-Key: ");

                    // The value for Sec-WebSocket-Key is a 16-byte random number, encoded with Base64.
                    byte[] key = new byte[16];
                    _rand.NextBytes(key);
                    _key = Convert.ToBase64String(key);
                    sb.Append(_key + "\r\n\r\n"); // EOM
                    byte[] data = _utf8.GetBytes(sb.ToString());
                    _sendBuffer.Add(data);

                    await _underlying.SendAsync(_sendBuffer, cancel).ConfigureAwait(false);
                }
                _sendBuffer.Clear();

                // Try to read the client's upgrade request or the server's response.
                var httpBuffer = new ArraySegment<byte>();
                while (true)
                {
                    ReadOnlyMemory<byte> buffer = await _underlying.ReceiveAsync(0, cancel).ConfigureAwait(false);
                    if (httpBuffer.Count + buffer.Length > _communicator.IncomingFrameMaxSize)
                    {
                        throw new InvalidDataException(
                            "WebSocket frame size is greater than the configured IncomingFrameMaxSize value");
                    }

                    ArraySegment<byte> tmpBuffer = new byte[httpBuffer.Count + buffer.Length];
                    if (httpBuffer.Count > 0)
                    {
                        httpBuffer.CopyTo(tmpBuffer);
                    }
                    buffer.CopyTo(tmpBuffer.Slice(httpBuffer.Count));
                    httpBuffer = tmpBuffer;

                    // Check if we have enough data for a complete frame.
                    int endPos = HttpParser.IsCompleteMessage(httpBuffer);
                    if (endPos != -1)
                    {
                        // Add back the un-consumed data to the buffer.
                        _underlying.Rewind(httpBuffer.Count - endPos);
                        httpBuffer = httpBuffer.Slice(0, endPos);
                        break; // Done
                    }
                }

                try
                {
                    if (_parser.Parse(httpBuffer))
                    {
                        if (_incoming)
                        {
                            (bool addProtocol, string key) = ReadUpgradeRequest();

                            // Compose the response.
                            var sb = new StringBuilder();
                            sb.Append("HTTP/1.1 101 Switching Protocols\r\n");
                            sb.Append("Upgrade: websocket\r\n");
                            sb.Append("Connection: Upgrade\r\n");
                            if (addProtocol)
                            {
                                sb.Append($"Sec-WebSocket-Protocol: {IceProtocol}\r\n");
                            }

                            // The response includes:
                            //
                            // "A |Sec-WebSocket-Accept| header field.  The value of this header field is constructed
                            // by concatenating /key/, defined above in step 4 in Section 4.2.2, with the string
                            // "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", taking the SHA-1 hash of this concatenated value
                            // to obtain a 20-byte value and base64-encoding (see Section 4 of [RFC4648]) this 20-byte
                            // hash.
                            sb.Append("Sec-WebSocket-Accept: ");
                            string input = key + WsUUID;
#pragma warning disable CA5350 // Do Not Use Weak Cryptographic Algorithms
                            using var sha1 = SHA1.Create();
                            byte[] hash = sha1.ComputeHash(_utf8.GetBytes(input));
#pragma warning restore CA5350 // Do Not Use Weak Cryptographic Algorithms
                            sb.Append(Convert.ToBase64String(hash) + "\r\n" + "\r\n"); // EOM

                            Debug.Assert(_sendBuffer.Count == 0);
                            byte[] data = _utf8.GetBytes(sb.ToString());
                            _sendBuffer.Add(data);
                            await _underlying.SendAsync(_sendBuffer, cancel).ConfigureAwait(false);
                            _sendBuffer.Clear();
                        }
                        else
                        {
                            ReadUpgradeResponse();
                        }
                    }
                    else
                    {
                        throw new InvalidDataException("incomplete WebSocket request frame");
                    }
                }
                catch (WebSocketException ex)
                {
                    throw new InvalidDataException(ex.Message, ex);
                }
            }
            catch (Exception ex)
            {
                if (_communicator.TraceLevels.Transport >= 2)
                {
                    _communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"{_transportName} connection HTTP upgrade request failed\n{this}\n{ex}");
                }
                throw;
            }

            if (_communicator.TraceLevels.Transport >= 1)
            {
                if (_incoming)
                {
                    _communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"accepted {_transportName} connection HTTP upgrade request\n{this}");
                }
                else
                {
                    _communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"{_transportName} connection HTTP upgrade request accepted\n{this}");
                }
            }
        }
        public override ValueTask<ArraySegment<byte>> ReceiveDatagramAsync(CancellationToken cancel) =>
            throw new InvalidOperationException("only supported by datagram transports");

        public override async ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel)
        {
            // If we've fully read the previous DATA frame payload, read a new frame
            if (_receivePayloadOffset == _receivePayloadLength)
            {
                _receivePayloadLength = await ReceiveFrameAsync(cancel).ConfigureAwait(false);
                _receivePayloadOffset = 0;
            }

            if (_receivePayloadLength == 0)
            {
                throw new ConnectionLostException(RetryPolicy.AfterDelay(TimeSpan.Zero));
            }

            // Read the payload
            int length = Math.Min(_receivePayloadLength, buffer.Length);
            int received = await _underlying.ReceiveAsync(buffer[0..length], cancel).ConfigureAwait(false);

            if (_incoming)
            {
                Unmask(buffer, _receivePayloadOffset, received);
            }
            _receivePayloadOffset += received;
            return received;
        }

        public override ValueTask<int> SendAsync(IList<ArraySegment<byte>> buffers, CancellationToken cancel) =>
             SendImplAsync(OpCode.Data, buffers, cancel);

        public override string ToString() => _underlying.ToString()!;

        protected override void Dispose(bool disposing) => _underlying.Dispose();

        internal WSSocket(
            Communicator communicator,
            SingleStreamSocket del,
            string host,
            string resource)
            : this(communicator, del)
        {
            _host = host;
            _resource = resource;
            _incoming = false;
            _transportName = (del is SslSocket) ? "wss" : "ws";
        }

        internal WSSocket(Communicator communicator, SingleStreamSocket underlying)
        {
            _communicator = communicator;
            _underlying = new BufferedReceiveOverSingleStreamSocket(underlying);
            _parser = new HttpParser();
            _receiveLastFrame = true;
            _sendBuffer = new List<ArraySegment<byte>>();
            _sendMask = new byte[4];
            _key = "";
            _rand = new Random();
            _host = "";
            _resource = "";
            _incoming = true;
            _transportName = (underlying is SslSocket) ? "wss" : "ws";
        }

        private ArraySegment<byte> PrepareHeaderForSend(OpCode opCode, int payloadLength)
        {
            // Prepare the frame header.
            byte[] buffer = new byte[16];
            int i = 0;

            // Set the opcode - this is the one and only data frame.
            buffer[i++] = (byte)((byte)opCode | FlagFinal);

            // Set the payload length.
            if (payloadLength <= 125)
            {
                buffer[i++] = (byte)payloadLength;
            }
            else if (payloadLength > 125 && payloadLength <= 65535)
            {
                // Use an extra 16 bits to encode the payload length.
                buffer[i++] = 126;
                short length = System.Net.IPAddress.HostToNetworkOrder((short)payloadLength);
                MemoryMarshal.Write(buffer.AsSpan(i, 2), ref length);
                i += 2;
            }
            else if (payloadLength > 65535)
            {
                // Use an extra 64 bits to encode the payload length.
                buffer[i++] = 127;
                long length = System.Net.IPAddress.HostToNetworkOrder((long)payloadLength);
                MemoryMarshal.Write(buffer.AsSpan(i, 8), ref length);
                i += 8;
            }

            if (!_incoming)
            {
                // Add a random 32-bit mask to every outgoing frame, copy the payload data, and apply the mask.
                buffer[1] = (byte)(buffer[1] | FlagMasked);
                _rand.NextBytes(_sendMask);
                Buffer.BlockCopy(_sendMask, 0, buffer, i, _sendMask.Length);
                i += _sendMask.Length;
            }
            return new ArraySegment<byte>(buffer, 0, i);
        }

        private async ValueTask<int> ReceiveFrameAsync(CancellationToken cancel)
        {
            while (true)
            {
                // Read the first 2 bytes of the WS frame header
                ReadOnlyMemory<byte> header = await _underlying.ReceiveAsync(2, cancel).ConfigureAwait(false);
                // Most-significant bit indicates if this is the last frame, least-significant four bits hold the opcode.
                var opCode = (OpCode)(header.Span[0] & 0xf);

                // Check if the OpCode is compatible of the FIN flag of the previous frame.
                if (opCode == OpCode.Data && !_receiveLastFrame)
                {
                    throw new InvalidDataException("invalid WebSocket data frame, no FIN on previous frame");
                }
                else if (opCode == OpCode.Continuation && _receiveLastFrame)
                {
                    throw new InvalidDataException("invalid WebSocket continuation frame, previous frame FIN set");
                }

                // Remember the FIN flag of this frame for the previous check.
                _receiveLastFrame = (header.Span[0] & FlagFinal) == FlagFinal;

                // Messages sent by a client must be masked; frames sent by a server must not be masked.
                bool masked = (header.Span[1] & FlagMasked) == FlagMasked;
                if (masked != _incoming)
                {
                    throw new InvalidDataException("invalid WebSocket masking");
                }

                // Extract the payload length, which can have the following values:
                // 0-125: The payload length
                // 126:   The subsequent two bytes contain the payload length
                // 127:   The subsequent eight bytes contain the payload length
                int payloadLength = header.Span[1] & 0x7f;
                if (payloadLength == 126)
                {
                    header = await _underlying.ReceiveAsync(2, cancel).ConfigureAwait(false);
                    ushort length = header.Span.ReadUShort();
                    payloadLength = (ushort)System.Net.IPAddress.NetworkToHostOrder((short)length);
                }
                else if (payloadLength == 127)
                {
                    header = await _underlying.ReceiveAsync(8, cancel).ConfigureAwait(false);
                    long length = System.Net.IPAddress.NetworkToHostOrder(header.Span.ReadLong());
                    if (length > int.MaxValue)
                    {
                        // We never send payloads with such length, we shouldn't get any.
                        throw new InvalidDataException("WebSocket payload length is not supported");
                    }
                    payloadLength = (int)length;
                }

                if (_incoming)
                {
                    // Read the mask if this is an incoming connection.
                    (await _underlying.ReceiveAsync(4, cancel).ConfigureAwait(false)).CopyTo(_receiveMask);
                }

                if (_communicator.TraceLevels.Transport >= 3)
                {
                    _communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"received {_transportName} {opCode} frame with {payloadLength} bytes payload\n{this}");
                }

                switch (opCode)
                {
                    case OpCode.Text:
                    {
                        throw new InvalidDataException("WebSocket text frames not supported");
                    }
                    case OpCode.Data:
                    case OpCode.Continuation:
                    {
                        if (payloadLength <= 0)
                        {
                            throw new InvalidDataException("WebSocket payload length is invalid");
                        }
                        return payloadLength;
                    }
                    case OpCode.Close:
                    {
                        // Read the Close frame payload.
                        ReadOnlyMemory<byte> payloadBuffer =
                            await _underlying.ReceiveAsync(payloadLength, cancel).ConfigureAwait(false);

                        byte[] payload = payloadBuffer.ToArray();
                        if (_incoming)
                        {
                            Unmask(payload, 0, payload.Length);
                        }

                        // If we've received a close frame and we were waiting for it, notify the task. Otherwise,
                        // we didn't send a close frame and we should reply back with a close frame.
                        if (_closing)
                        {
                            return 0;
                        }
                        else
                        {
                            var sendBuffer = new List<ArraySegment<byte>> { payload };
                            await SendImplAsync(OpCode.Close, sendBuffer, cancel).ConfigureAwait(false);
                        }
                        break;
                    }
                    case OpCode.Ping:
                    {
                        // Read the ping payload.
                        ReadOnlyMemory<byte> payload =
                            await _underlying.ReceiveAsync(payloadLength, cancel).ConfigureAwait(false);

                        // Send a Pong frame with the received payload.
                        var sendBuffer = new List<ArraySegment<byte>> { payload.ToArray() };
                        await SendImplAsync(OpCode.Pong, sendBuffer, cancel).ConfigureAwait(false);
                        break;
                    }
                    case OpCode.Pong:
                    {
                        // Read the pong payload.
                        ReadOnlyMemory<byte> payload =
                            await _underlying.ReceiveAsync(payloadLength, cancel).ConfigureAwait(false);

                        // Nothing to do, this can be received even if we don't send a ping frame if the peer sends
                        // an unidirectional heartbeat.
                        break;
                    }
                    default:
                    {
                        throw new InvalidDataException($"unsupported WebSocket opcode: {opCode}");
                    }
                }
            }
        }

        private (bool, string) ReadUpgradeRequest()
        {
            // HTTP/1.1
            if (_parser.VersionMajor() != 1 || _parser.VersionMinor() != 1)
            {
                throw new WebSocketException("unsupported HTTP version");
            }

            // "An |Upgrade| header field containing the value 'websocket', treated as an ASCII case-insensitive value."
            string? value = _parser.GetHeader("Upgrade", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if (!value.Equals("websocket"))
            {
                throw new WebSocketException($"invalid value `{value}' for Upgrade field");
            }

            // "A |Connection| header field that includes the token 'Upgrade', treated as an ASCII case-insensitive
            // value.
            value = _parser.GetHeader("Connection", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if (!value.Contains("upgrade"))
            {
                throw new WebSocketException($"invalid value `{value}' for Connection field");
            }

            // "A |Sec-WebSocket-Version| header field, with a value of 13."
            value = _parser.GetHeader("Sec-WebSocket-Version", false);
            if (value == null)
            {
                throw new WebSocketException("missing value for WebSocket version");
            }
            else if (!value.Equals("13"))
            {
                throw new WebSocketException($"unsupported WebSocket version `{value}'");
            }

            // "Optionally, a |Sec-WebSocket-Protocol| header field, with a list of values indicating which protocols
            // the client would like to speak, ordered by preference."
            bool addProtocol = false;
            value = _parser.GetHeader("Sec-WebSocket-Protocol", true);
            if (value != null)
            {
                string[]? protocols = StringUtil.SplitString(value, ",");
                if (protocols == null)
                {
                    throw new WebSocketException($"invalid value `{value}' for WebSocket protocol");
                }

                foreach (string protocol in protocols)
                {
                    if (!protocol.Trim().Equals(IceProtocol))
                    {
                        throw new WebSocketException($"unknown value `{protocol}' for WebSocket protocol");
                    }
                    addProtocol = true;
                }
            }

            // "A |Sec-WebSocket-Key| header field with a base64-encoded value that, when decoded, is 16 bytes in
            // length."
            string? key = _parser.GetHeader("Sec-WebSocket-Key", false);
            if (key == null)
            {
                throw new WebSocketException("missing value for WebSocket key");
            }

            byte[] decodedKey = Convert.FromBase64String(key);
            if (decodedKey.Length != 16)
            {
                throw new WebSocketException($"invalid value `{key}' for WebSocket key");
            }

            // Retain the target resource.
            _resource = _parser.Uri();

            return (addProtocol, key);
        }

        private void ReadUpgradeResponse()
        {
            // HTTP/1.1
            if (_parser.VersionMajor() != 1 || _parser.VersionMinor() != 1)
            {
                throw new WebSocketException("unsupported HTTP version");
            }

            // "If the status code received from the server is not 101, the client handles the response per HTTP
            // [RFC2616] procedures. In particular, the client might perform authentication if it receives a 401 status
            // code; the server might redirect the client using a 3xx status code (but clients are not required to
            // follow them), etc."
            if (_parser.Status() != 101)
            {
                var sb = new StringBuilder("unexpected status value " + _parser.Status());
                if (_parser.Reason().Length > 0)
                {
                    sb.Append(":\n" + _parser.Reason());
                }
                throw new WebSocketException(sb.ToString());
            }

            // "If the response lacks an |Upgrade| header field or the |Upgrade| header field contains a value that is
            // not an ASCII case-insensitive match for the value "websocket", the client MUST_Fail the WebSocket
            // Connection_."
            string? value = _parser.GetHeader("Upgrade", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if (!value.Equals("websocket"))
            {
                throw new WebSocketException($"invalid value `{value}' for Upgrade field");
            }

            // "If the response lacks a |Connection| header field or the |Connection| header field doesn't contain a
            // token that is an ASCII case-insensitive match for the value "Upgrade", the client MUST _Fail the
            // WebSocket Connection_."
            value = _parser.GetHeader("Connection", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if (!value.Contains("upgrade"))
            {
                throw new WebSocketException($"invalid value `{value}' for Connection field");
            }

            // "If the response includes a |Sec-WebSocket-Protocol| header field and this header field indicates the
            // use of a subprotocol that was not present in the client's handshake (the server has indicated a
            // subprotocol not requested by the client), the client MUST _Fail the WebSocket Connection_."
            value = _parser.GetHeader("Sec-WebSocket-Protocol", true);
            if (value != null && !value.Equals(IceProtocol))
            {
                throw new WebSocketException($"invalid value `{value}' for WebSocket protocol");
            }

            // "If the response lacks a |Sec-WebSocket-Accept| header field or the |Sec-WebSocket-Accept| contains a
            // value other than the base64-encoded SHA-1 of the concatenation of the |Sec-WebSocket-Key| (as a string,
            // not base64-decoded) with the string "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" but ignoring any leading and
            // trailing whitespace, the client MUST _Fail the WebSocket Connection_."
            value = _parser.GetHeader("Sec-WebSocket-Accept", false);
            if (value == null)
            {
                throw new WebSocketException("missing value for Sec-WebSocket-Accept");
            }

            string input = _key + WsUUID;
#pragma warning disable CA5350 // Do Not Use Weak Cryptographic Algorithms
            using var sha1 = SHA1.Create();
            byte[] hash = sha1.ComputeHash(_utf8.GetBytes(input));
#pragma warning restore CA5350 // Do Not Use Weak Cryptographic Algorithms
            if (!value.Equals(Convert.ToBase64String(hash)))
            {
                throw new WebSocketException($"invalid value `{value}' for Sec-WebSocket-Accept");
            }
        }

        private async ValueTask<int> SendImplAsync(
            OpCode opCode,
            IList<ArraySegment<byte>> buffers,
            CancellationToken cancel)
        {
            // Write can be called concurrently because it's called from both ReadAsync and WriteAsync. For example,
            // the reading of a ping frame requires writing a pong frame.
            Task<int> task;
            lock (_mutex)
            {
                ValueTask<int> writeTask = PerformWriteAsync(opCode, buffers, cancel);

                // Optimization: we check if the write completed already and avoid creating a Task if it did.
                if (writeTask.IsCompletedSuccessfully)
                {
                    _sendTask = Task.CompletedTask;
                    return writeTask.Result;
                }

                task = writeTask.AsTask();
                _sendTask = task;
            }
            return await task.ConfigureAwait(false);

            async ValueTask<int> PerformWriteAsync(
                OpCode opCode,
                IList<ArraySegment<byte>> buffers,
                CancellationToken cancel)
            {
                // Wait for the current write to be done.
                await _sendTask.ConfigureAwait(false);

                // Write the given buffer.
                Debug.Assert(_sendBuffer.Count == 0);
                int size = buffers.GetByteCount();
                _sendBuffer.Add(PrepareHeaderForSend(opCode, size));
                if (_communicator.TraceLevels.Transport >= 3)
                {
                    _communicator.Logger.Trace(TraceLevels.TransportCategory,
                        $"sending {_transportName} {opCode} frame with {size} bytes payload\n{this}");
                }

                if (_incoming || opCode == OpCode.Pong)
                {
                    foreach (ArraySegment<byte> segment in buffers)
                    {
                        _sendBuffer.Add(segment); // Borrow data from the buffer
                    }
                }
                else
                {
                    // For an outgoing connection, each frame must be masked with a random 32-bit value.
                    int n = 0;
                    foreach (ArraySegment<byte> segment in buffers)
                    {
                        byte[] data = new byte[segment.Count];
                        for (int i = 0; i < segment.Count; ++i, ++n)
                        {
                            data[i] = (byte)(segment[i] ^ _sendMask[n % 4]);
                        }
                        _sendBuffer.Add(data);
                    }
                }
                await _underlying.SendAsync(_sendBuffer, cancel).ConfigureAwait(false);
                _sendBuffer.Clear();
                return size;
            }
        }

        private void Unmask(Memory<byte> buffer, int offset, int length)
        {
            Span<byte> bufferAsSpan = buffer.Span;
            for (int i = 0; i < length; ++i)
            {
                bufferAsSpan[i] = (byte)(bufferAsSpan[i] ^ _receiveMask[offset++ % 4]);
            }
        }
    }
}
