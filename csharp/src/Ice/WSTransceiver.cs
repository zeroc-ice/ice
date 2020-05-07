//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;

namespace IceInternal
{
    // TODO: refactor and eliminate these exceptions
    public sealed class WSProtocolException : Exception
    {
        internal WSProtocolException(string message)
            : base(message)
        {
        }
    }

    public sealed class WSMemoryLimitException : Exception
    {
        internal WSMemoryLimitException()
        {
        }
    }

    internal sealed class WSTransceiver : ITransceiver
    {
        public Socket? Fd() => _delegate.Fd();

        public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
        {
            //
            // Delegate logs exceptions that occur during initialize(), so there's no need to trap them here.
            //
            if (_state == StateInitializeDelegate)
            {
                int op = _delegate.Initialize(ref readBuffer, writeBuffer);
                if (op != 0)
                {
                    return op;
                }
                _state = StateConnected;
            }

            try
            {
                if (_state == StateConnected)
                {
                    //
                    // We don't know how much we'll need to read.
                    //
                    _readBuffer = new byte[1024];
                    _readBufferOffset = 0;
                    _readBufferPos = 0;

                    //
                    // The server waits for the client's upgrade request, the
                    // client sends the upgrade request.
                    //
                    _state = StateUpgradeRequestPending;
                    if (!_incoming)
                    {
                        //
                        // Compose the upgrade request.
                        //
                        var sb = new StringBuilder();
                        sb.Append("GET " + _resource + " HTTP/1.1\r\n");
                        sb.Append("Host: " + _host + "\r\n");
                        sb.Append("Upgrade: websocket\r\n");
                        sb.Append("Connection: Upgrade\r\n");
                        sb.Append("Sec-WebSocket-Protocol: " + IceProtocol + "\r\n");
                        sb.Append("Sec-WebSocket-Version: 13\r\n");
                        sb.Append("Sec-WebSocket-Key: ");

                        //
                        // The value for Sec-WebSocket-Key is a 16-byte random number,
                        // encoded with Base64.
                        //
                        byte[] key = new byte[16];
                        _rand.NextBytes(key);
                        _key = Convert.ToBase64String(key);
                        sb.Append(_key + "\r\n\r\n"); // EOM
                        Debug.Assert(_writeBufferSize == 0);
                        byte[] data = _utf8.GetBytes(sb.ToString());
                        _writeBuffer.Add(data);
                        _writeBufferSize = data.Length;
                        _writeBufferOffset = 0;
                    }
                }

                //
                // Try to write the client's upgrade request.
                //
                if (_state == StateUpgradeRequestPending && !_incoming)
                {
                    if (_writeBufferOffset < _writeBufferSize)
                    {
                        int socketOperation = _delegate.Write(_writeBuffer, ref _writeBufferOffset);
                        if (socketOperation != 0)
                        {
                            return socketOperation;
                        }
                    }
                    Debug.Assert(_writeBufferOffset == _writeBufferSize);
                    _state = StateUpgradeResponsePending;
                }

                while (true)
                {
                    if (_readBufferOffset < _readBuffer.Count)
                    {
                        int s = _delegate.Read(ref _readBuffer, ref _readBufferOffset);
                        if (s == SocketOperation.Write || _readBufferOffset == 0)
                        {
                            return s;
                        }
                    }

                    //
                    // Try to read the client's upgrade request or the server's response.
                    //
                    if ((_state == StateUpgradeRequestPending && _incoming) ||
                        (_state == StateUpgradeResponsePending && !_incoming))
                    {
                        //
                        // Check if we have enough data for a complete message.
                        //
                        int p = HttpParser.IsCompleteMessage(_readBuffer.AsSpan(0, _readBufferOffset));
                        if (p == -1)
                        {
                            if (_readBufferOffset < _readBuffer.Count)
                            {
                                return SocketOperation.Read;
                            }

                            //
                            // Enlarge the buffer and try to read more.
                            //
                            if (_readBufferOffset + 1024 > _instance.MessageSizeMax)
                            {
                                throw new WSMemoryLimitException();
                            }
                            byte[] tmpBuffer = new byte[_readBufferOffset + 1024];
                            _readBuffer.AsSpan(0, _readBufferOffset).CopyTo(tmpBuffer);
                            _readBuffer = tmpBuffer;
                            continue; // Try again to read the response/request
                        }

                        //
                        // Set _readBufferPos at the end of the response/request message.
                        //
                        _readBufferPos = p;
                    }

                    //
                    // We're done, the client's upgrade request or server's response is read.
                    //
                    break;
                }

                try
                {
                    //
                    // Parse the client's upgrade request.
                    //
                    if (_state == StateUpgradeRequestPending && _incoming)
                    {
                        if (_parser.Parse(_readBuffer.Slice(0, _readBufferPos)))
                        {
                            HandleRequest();
                            _state = StateUpgradeResponsePending;
                        }
                        else
                        {
                            throw new WSProtocolException("incomplete request message");
                        }
                    }

                    if (_state == StateUpgradeResponsePending)
                    {
                        if (_incoming)
                        {
                            if (_writeBufferOffset < _writeBufferSize)
                            {
                                int socketOperation = _delegate.Write(_writeBuffer, ref _writeBufferOffset);
                                if (socketOperation != 0)
                                {
                                    return socketOperation;
                                }
                            }
                        }
                        else
                        {
                            //
                            // Parse the server's response
                            //
                            if (_parser.Parse(_readBuffer.Slice(0, _readBufferPos)))
                            {
                                HandleResponse();
                            }
                            else
                            {
                                throw new WSProtocolException("incomplete response message");
                            }
                        }
                    }
                }
                catch (WebSocketException ex)
                {
                    throw new WSProtocolException(ex.Message);
                }
                _state = StateOpened;
                _nextState = StateOpened;
            }
            catch (System.Exception ex)
            {
                if (_instance.TraceLevel >= 2)
                {
                    _instance.Logger.Trace(_instance.TraceCategory,
                        $"{Transport()} connection HTTP upgrade request failed\n{this}\n{ex}");
                }
                throw;
            }

            if (_instance.TraceLevel >= 1)
            {
                if (_incoming)
                {
                    _instance.Logger.Trace(_instance.TraceCategory,
                        $"accepted {Transport()} connection HTTP upgrade request\n{this}");
                }
                else
                {
                    _instance.Logger.Trace(_instance.TraceCategory, $"{Transport()} connection HTTP upgrade request accepted\n{this}");
                }
            }

            return SocketOperation.None;
        }

        public int Closing(bool initiator, System.Exception? reason)
        {
            if (_instance.TraceLevel >= 1)
            {
                _instance.Logger.Trace(_instance.TraceCategory, $"gracefully closing {Transport()} connection\n{this}");
            }

            int s = _nextState == StateOpened ? _state : _nextState;

            if (s == StateClosingRequestPending && _closingInitiator)
            {
                //
                // If we initiated a close connection but also received a
                // connection closed, we assume we didn't initiated the
                // connection and we send the close frame now. This is to
                // ensure that if both peers close the connection at the same
                // time we don't hang having both peer waiting for the close
                // frame of the other.
                //
                Debug.Assert(!initiator);
                _closingInitiator = false;
                return SocketOperation.Write;
            }
            else if (s >= StateClosingRequestPending)
            {
                return SocketOperation.None;
            }

            _closingInitiator = initiator;
            if (reason is ConnectionClosedByPeerException)
            {
                _closingReason = CLOSURE_NORMAL;
            }
            else if (reason is ObjectAdapterDeactivatedException ||
                    reason is CommunicatorDestroyedException)
            {
                _closingReason = CLOSURE_SHUTDOWN;
            }
            else if (reason is WSProtocolException)
            {
                _closingReason = CLOSURE_PROTOCOL_ERROR;
            }
            else if (reason is WSMemoryLimitException)
            {
                _closingReason = CLOSURE_TOO_BIG;
            }

            if (_state == StateOpened)
            {
                _state = StateClosingRequestPending;
                return initiator ? SocketOperation.Read : SocketOperation.Write;
            }
            else
            {
                _nextState = StateClosingRequestPending;
                return SocketOperation.None;
            }
        }

        public void Close()
        {
            _delegate.Close();
            _state = StateClosed;

            //
            // Clear the buffers now instead of waiting for destruction.
            //
            if (!_readPending)
            {
                _readBuffer = ArraySegment<byte>.Empty;
            }
            if (!_writePending)
            {
                _writeBuffer.Clear();
            }
        }

        public Endpoint Bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void Destroy() => _delegate.Destroy();

        public int Write(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            if (_writePending)
            {
                return SocketOperation.Write;
            }

            if (_state < StateOpened)
            {
                if (_state < StateConnected)
                {
                    return _delegate.Write(buffer, ref offset);
                }
                else
                {
                    return _delegate.Write(_writeBuffer, ref _writeBufferOffset);
                }
            }

            int socketOperation = SocketOperation.None;
            int size = buffer.GetByteCount();
            do
            {
                if (PreWrite(buffer, size, offset))
                {
                    if (_writeState == WriteStateFlush)
                    {
                        //
                        // Invoke write() even though there's nothing to write.
                        //
                        Debug.Assert(size == offset);
                        socketOperation = _delegate.Write(buffer, ref offset);
                    }

                    if (socketOperation == SocketOperation.None && _writeBufferOffset < _writeBufferSize)
                    {
                        socketOperation = _delegate.Write(_writeBuffer, ref _writeBufferOffset);
                    }
                    else if (socketOperation == SocketOperation.None && _incoming && size > 0 && _writeState == WriteStatePayload)
                    {
                        socketOperation = _delegate.Write(buffer, ref offset);
                    }
                }
            }
            while (PostWrite(size, ref offset, socketOperation));

            if (socketOperation != SocketOperation.None)
            {
                return socketOperation;
            }
            if (_state == StateClosingResponsePending && !_closingInitiator)
            {
                return SocketOperation.Read;
            }
            return SocketOperation.None;
        }

        public int Read(ref ArraySegment<byte> buffer, ref int offset)
        {
            if (_readPending)
            {
                return SocketOperation.Read;
            }

            if (_state < StateOpened)
            {
                if (_state < StateConnected)
                {
                    return _delegate.Read(ref buffer, ref offset);
                }
                else
                {
                    if (_delegate.Read(ref _readBuffer, ref _readBufferOffset) == SocketOperation.Write)
                    {
                        return SocketOperation.Write;
                    }
                    else
                    {
                        return SocketOperation.None;
                    }
                }
            }

            if (offset == buffer.Count)
            {
                return SocketOperation.None;
            }

            int s;
            do
            {
                if (PreRead(buffer, ref offset))
                {
                    if (_readState == ReadStatePayload)
                    {
                        //
                        // If the payload length is smaller than what remains to be read, we read
                        // no more than the payload length. The remaining of the buffer will be
                        // sent over in another frame.
                        //
                        int readSz = _readPayloadLength - (offset - _readStart);
                        if (buffer.Count - offset > readSz)
                        {
                            int size = buffer.Count;
                            buffer = buffer.Slice(0, offset + readSz);
                            s = _delegate.Read(ref buffer, ref offset);
                            Debug.Assert(buffer.Array != null);
                            buffer = new ArraySegment<byte>(buffer.Array, 0, size);
                        }
                        else
                        {
                            s = _delegate.Read(ref buffer, ref offset);
                        }
                    }
                    else
                    {
                        s = _delegate.Read(ref _readBuffer, ref _readBufferOffset);
                    }

                    if (s == SocketOperation.Write)
                    {
                        PostRead(buffer, offset);
                        return s;
                    }
                }
            }
            while (PostRead(buffer, offset));

            s = offset < buffer.Count ? SocketOperation.Read : SocketOperation.None;
            if (((_state == StateClosingRequestPending && !_closingInitiator) ||
                 (_state == StateClosingResponsePending && _closingInitiator) ||
                 _state == StatePingPending ||
                 _state == StatePongPending) &&
                _writeState == WriteStateHeader)
            {
                // We have things to write, ask to be notified when writes are ready.
                s |= SocketOperation.Write;
            }

            return s;
        }

        public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, AsyncCallback callback, object state)
        {
            _readPending = true;
            if (_state < StateOpened)
            {
                _finishRead = true;
                if (_state < StateConnected)
                {
                    return _delegate.StartRead(ref buffer, ref offset, callback, state);
                }
                else
                {
                    return _delegate.StartRead(ref _readBuffer, ref _readBufferOffset, callback, state);
                }
            }

            if (PreRead(buffer, ref offset))
            {
                _finishRead = true;
                if (_readState == ReadStatePayload)
                {
                    //
                    // If the payload length is smaller than what remains to be read, we read
                    // no more than the payload length. The remaining of the buffer will be
                    // sent over in another frame.
                    //
                    int readSz = _readPayloadLength - (offset - _readStart);
                    if (buffer.Count - offset > readSz)
                    {
                        int size = buffer.Count;
                        buffer = buffer.Slice(0, offset + readSz);
                        bool completedSynchronously = _delegate.StartRead(ref buffer, ref offset, callback, state);
                        Debug.Assert(buffer.Array != null);
                        buffer = new ArraySegment<byte>(buffer.Array, 0, size);
                        return completedSynchronously;
                    }
                    else
                    {
                        return _delegate.StartRead(ref buffer, ref offset, callback, state);
                    }
                }
                else
                {
                    return _delegate.StartRead(ref _readBuffer, ref _readBufferOffset, callback, state);
                }
            }
            else
            {
                return true;
            }
        }

        public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
        {
            Debug.Assert(_readPending);
            _readPending = false;

            if (_state < StateOpened)
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                if (_state < StateConnected)
                {
                    _delegate.FinishRead(ref buffer, ref offset);
                }
                else
                {
                    _delegate.FinishRead(ref _readBuffer, ref _readBufferOffset);
                }
                return;
            }

            if (!_finishRead)
            {
                // Nothing to do.
            }
            else if (_readState == ReadStatePayload)
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                _delegate.FinishRead(ref buffer, ref offset);
            }
            else
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                _delegate.FinishRead(ref _readBuffer, ref _readBufferOffset);
            }

            if (_state == StateClosed)
            {
                _readBuffer = ArraySegment<byte>.Empty;
                return;
            }

            PostRead(buffer, offset);
        }

        public bool
        StartWrite(IList<ArraySegment<byte>> buf, int offset, AsyncCallback callback, object state, out bool completed)
        {
            _writePending = true;
            if (_state < StateOpened)
            {
                if (_state < StateConnected)
                {
                    return _delegate.StartWrite(buf, offset, callback, state, out completed);
                }
                else
                {
                    return _delegate.StartWrite(_writeBuffer, _writeBufferOffset, callback, state, out completed);
                }
            }

            int size = buf.GetByteCount();
            if (PreWrite(buf, size, offset))
            {
                if (_writeBufferOffset < _writeBufferSize)
                {
                    return _delegate.StartWrite(_writeBuffer, _writeBufferOffset, callback, state, out completed);
                }
                else
                {
                    Debug.Assert(_incoming);
                    return _delegate.StartWrite(buf, offset, callback, state, out completed);
                }
            }
            else
            {
                completed = true;
                return false;
            }
        }

        public void FinishWrite(IList<ArraySegment<byte>> buffer, ref int offset)
        {
            _writePending = false;
            if (_state < StateOpened)
            {
                if (_state < StateConnected)
                {
                    _delegate.FinishWrite(buffer, ref offset);
                }
                else
                {
                    _delegate.FinishWrite(_writeBuffer, ref _writeBufferOffset);
                }
                return;
            }

            int size = buffer.GetByteCount();
            if (_writeBufferOffset < _writeBufferSize)
            {
                _delegate.FinishWrite(_writeBuffer, ref _writeBufferOffset);
            }
            else if (size > 0 && offset < size)
            {
                Debug.Assert(_incoming);
                _delegate.FinishWrite(buffer, ref offset);
            }

            if (_state == StateClosed)
            {
                _writeBuffer.Clear();
                _writeBufferOffset = 0;
                _writeBufferSize = 0;
                return;
            }

            PostWrite(size, ref offset, SocketOperation.None);
        }

        public string Transport() => _instance.Transport;

        public ConnectionInfo GetInfo()
        {
            var info = new WSConnectionInfo();
            info.Headers = _parser.GetHeaders();
            info.Underlying = _delegate.GetInfo();
            return info;
        }

        public void CheckSendSize(int size) => _delegate.CheckSendSize(size);

        public void SetBufferSize(int rcvSize, int sndSize) => _delegate.SetBufferSize(rcvSize, sndSize);

        public override string ToString() => _delegate.ToString()!;

        public string ToDetailedString() => _delegate.ToDetailedString();

        internal
        WSTransceiver(TransportInstance instance, ITransceiver del, string host, string resource) :
            this(instance, del)
        {
            _host = host;
            _resource = resource;
            _incoming = false;

            //
            // Write and read buffer size must be large enough to hold the frame header!
            //
            Debug.Assert(_readBufferSize > 256);
        }

        internal WSTransceiver(TransportInstance instance, ITransceiver del)
        {
            _instance = instance;
            _delegate = del;
            _state = StateInitializeDelegate;
            _parser = new HttpParser();
            _readState = ReadStateOpcode;
            _readBuffer = ArraySegment<byte>.Empty;
            _readBufferSize = 1024;
            _readLastFrame = true;
            _readOpCode = 0;
            _readHeaderLength = 0;
            _readPayloadLength = 0;
            _writeState = WriteStateHeader;
            _writeBuffer = new List<ArraySegment<byte>>();
            _readPending = false;
            _finishRead = false;
            _writePending = false;
            _readMask = new byte[4];
            _writeMask = new byte[4];
            _key = "";
            _pingPayload = Array.Empty<byte>();
            _rand = new Random();
            _host = "";
            _resource = "";
            _incoming = true;

            //
            // Write and read buffer size must be large enough to hold the frame header!
            //
            Debug.Assert(_readBufferSize > 256);
        }

        private void HandleRequest()
        {
            //
            // HTTP/1.1
            //
            if (_parser.VersionMajor() != 1 || _parser.VersionMinor() != 1)
            {
                throw new WebSocketException("unsupported HTTP version");
            }

            //
            // "An |Upgrade| header field containing the value 'websocket',
            //  treated as an ASCII case-insensitive value."
            //
            string? value = _parser.GetHeader("Upgrade", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if (!value.Equals("websocket"))
            {
                throw new WebSocketException($"invalid value `{value}' for Upgrade field");
            }

            //
            // "A |Connection| header field that includes the token 'Upgrade',
            //  treated as an ASCII case-insensitive value.
            //
            value = _parser.GetHeader("Connection", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if (value.IndexOf("upgrade") == -1)
            {
                throw new WebSocketException($"invalid value `{value}' for Connection field");
            }

            //
            // "A |Sec-WebSocket-Version| header field, with a value of 13."
            //
            value = _parser.GetHeader("Sec-WebSocket-Version", false);
            if (value == null)
            {
                throw new WebSocketException("missing value for WebSocket version");
            }
            else if (!value.Equals("13"))
            {
                throw new WebSocketException($"unsupported WebSocket version `{value}'");
            }

            //
            // "Optionally, a |Sec-WebSocket-Protocol| header field, with a list
            //  of values indicating which protocols the client would like to
            //  speak, ordered by preference."
            //
            bool addProtocol = false;
            value = _parser.GetHeader("Sec-WebSocket-Protocol", true);
            if (value != null)
            {
                string[]? protocols = IceUtilInternal.StringUtil.SplitString(value, ",");
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

            //
            // "A |Sec-WebSocket-Key| header field with a base64-encoded
            //  value that, when decoded, is 16 bytes in length."
            //
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

            //
            // Retain the target resource.
            //
            _resource = _parser.Uri();

            //
            // Compose the response.
            //
            var sb = new StringBuilder();
            sb.Append("HTTP/1.1 101 Switching Protocols\r\n");
            sb.Append("Upgrade: websocket\r\n");
            sb.Append("Connection: Upgrade\r\n");
            if (addProtocol)
            {
                sb.Append($"Sec-WebSocket-Protocol: {IceProtocol}\r\n");
            }

            //
            // The response includes:
            //
            // "A |Sec-WebSocket-Accept| header field.  The value of this
            //  header field is constructed by concatenating /key/, defined
            //  above in step 4 in Section 4.2.2, with the string "258EAFA5-
            //  E914-47DA-95CA-C5AB0DC85B11", taking the SHA-1 hash of this
            //  concatenated value to obtain a 20-byte value and base64-
            //  encoding (see Section 4 of [RFC4648]) this 20-byte hash.
            //
            sb.Append("Sec-WebSocket-Accept: ");
            string input = key + WsUUID;
#pragma warning disable CA5350 // Do Not Use Weak Cryptographic Algorithms
            byte[] hash = SHA1.Create().ComputeHash(_utf8.GetBytes(input));
#pragma warning restore CA5350 // Do Not Use Weak Cryptographic Algorithms
            sb.Append(Convert.ToBase64String(hash) + "\r\n" + "\r\n"); // EOM
            _writeBuffer.Clear();
            byte[] data = _utf8.GetBytes(sb.ToString());
            _writeBuffer.Add(data);
            _writeBufferSize = data.Length;
            _writeBufferOffset = 0;
        }

        private void HandleResponse()
        {
            //
            // HTTP/1.1
            //
            if (_parser.VersionMajor() != 1 || _parser.VersionMinor() != 1)
            {
                throw new WebSocketException("unsupported HTTP version");
            }

            //
            // "If the status code received from the server is not 101, the
            //  client handles the response per HTTP [RFC2616] procedures.  In
            //  particular, the client might perform authentication if it
            //  receives a 401 status code; the server might redirect the client
            //  using a 3xx status code (but clients are not required to follow
            //  them), etc."
            //
            if (_parser.Status() != 101)
            {
                var @out = new StringBuilder("unexpected status value " + _parser.Status());
                if (_parser.Reason().Length > 0)
                {
                    @out.Append(":\n" + _parser.Reason());
                }
                throw new WebSocketException(@out.ToString());
            }

            //
            // "If the response lacks an |Upgrade| header field or the |Upgrade|
            //  header field contains a value that is not an ASCII case-
            //  insensitive match for the value "websocket", the client MUST
            //  _Fail the WebSocket Connection_."
            //
            string? value = _parser.GetHeader("Upgrade", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if (!value.Equals("websocket"))
            {
                throw new WebSocketException($"invalid value `{value}' for Upgrade field");
            }

            //
            // "If the response lacks a |Connection| header field or the
            //  |Connection| header field doesn't contain a token that is an
            //  ASCII case-insensitive match for the value "Upgrade", the client
            //  MUST _Fail the WebSocket Connection_."
            //
            value = _parser.GetHeader("Connection", true);
            if (value == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if (value.IndexOf("upgrade") == -1)
            {
                throw new WebSocketException($"invalid value `{value}' for Connection field");
            }

            //
            // "If the response includes a |Sec-WebSocket-Protocol| header field
            //  and this header field indicates the use of a subprotocol that was
            //  not present in the client's handshake (the server has indicated a
            //  subprotocol not requested by the client), the client MUST _Fail
            //  the WebSocket Connection_."
            //
            value = _parser.GetHeader("Sec-WebSocket-Protocol", true);
            if (value != null && !value.Equals(IceProtocol))
            {
                throw new WebSocketException($"invalid value `{value}' for WebSocket protocol");
            }

            //
            // "If the response lacks a |Sec-WebSocket-Accept| header field or
            //  the |Sec-WebSocket-Accept| contains a value other than the
            //  base64-encoded SHA-1 of the concatenation of the |Sec-WebSocket-
            //  Key| (as a string, not base64-decoded) with the string "258EAFA5-
            //  E914-47DA-95CA-C5AB0DC85B11" but ignoring any leading and
            //  trailing whitespace, the client MUST _Fail the WebSocket
            //  Connection_."
            //
            value = _parser.GetHeader("Sec-WebSocket-Accept", false);
            if (value == null)
            {
                throw new WebSocketException("missing value for Sec-WebSocket-Accept");
            }

            string input = _key + WsUUID;
#pragma warning disable CA5350 // Do Not Use Weak Cryptographic Algorithms
            byte[] hash = SHA1.Create().ComputeHash(_utf8.GetBytes(input));
#pragma warning restore CA5350 // Do Not Use Weak Cryptographic Algorithms
            if (!value.Equals(Convert.ToBase64String(hash)))
            {
                throw new WebSocketException($"invalid value `{value}' for Sec-WebSocket-Accept");
            }
        }

        private bool PreRead(ArraySegment<byte> buffer, ref int offset)
        {
            while (true)
            {
                if (_readState == ReadStateOpcode)
                {
                    //
                    // Is there enough data available to read the opcode?
                    //
                    if (!ReadBuffered(2))
                    {
                        return true;
                    }

                    //
                    // Most-significant bit indicates whether this is the
                    // last frame. Least-significant four bits hold the
                    // opcode.
                    //
                    int ch = _readBuffer[_readBufferPos++];
                    _readOpCode = ch & 0xf;

                    //
                    // Remember if last frame if we're going to read a data or
                    // continuation frame, this is only for protocol
                    // correctness checking purpose.
                    //
                    if (_readOpCode == OP_DATA)
                    {
                        if (!_readLastFrame)
                        {
                            throw new WSProtocolException("invalid data frame, no FIN on previous frame");
                        }
                        _readLastFrame = (ch & FLAG_FINAL) == FLAG_FINAL;
                    }
                    else if (_readOpCode == OP_CONT)
                    {
                        if (_readLastFrame)
                        {
                            throw new WSProtocolException("invalid continuation frame, previous frame FIN set");
                        }
                        _readLastFrame = (ch & FLAG_FINAL) == FLAG_FINAL;
                    }

                    ch = _readBuffer[_readBufferPos++];
                    //
                    // Check the MASK bit. Messages sent by a client must be masked;
                    // messages sent by a server must not be masked.
                    //
                    bool masked = (ch & FLAG_MASKED) == FLAG_MASKED;
                    if (masked != _incoming)
                    {
                        throw new WSProtocolException("invalid masking");
                    }

                    //
                    // Extract the payload length, which can have the following values:
                    //
                    // 0-125: The payload length
                    // 126:   The subsequent two bytes contain the payload length
                    // 127:   The subsequent eight bytes contain the payload length
                    //
                    _readPayloadLength = ch & 0x7f;
                    if (_readPayloadLength < 126)
                    {
                        _readHeaderLength = 0;
                    }
                    else if (_readPayloadLength == 126)
                    {
                        _readHeaderLength = 2; // Need to read a 16-bit payload length.
                    }
                    else
                    {
                        _readHeaderLength = 8; // Need to read a 64-bit payload length.
                    }
                    if (masked)
                    {
                        _readHeaderLength += 4; // Need to read a 32-bit mask.
                    }

                    _readState = ReadStateHeader;
                }

                if (_readState == ReadStateHeader)
                {
                    //
                    // Is there enough data available to read the header?
                    //
                    if (_readHeaderLength > 0 && !ReadBuffered(_readHeaderLength))
                    {
                        return true;
                    }

                    if (_readPayloadLength == 126)
                    {
                        // Uses network byte order
                        _readPayloadLength = System.Net.IPAddress.NetworkToHostOrder(
                            InputStream.ReadShort(_readBuffer.Slice(_readBufferPos, 2)));
                        if (_readPayloadLength < 0)
                        {
                            _readPayloadLength += 65536;
                        }
                        _readBufferPos += 2;
                    }
                    else if (_readPayloadLength == 127)
                    {
                        // Uses network byte order.
                        long length = System.Net.IPAddress.NetworkToHostOrder(
                            InputStream.ReadLong(_readBuffer.Slice(_readBufferPos, 8)));
                        _readBufferPos += 8;
                        if (length < 0 || length > int.MaxValue)
                        {
                            throw new WSProtocolException($"invalid WebSocket payload length: {length}");
                        }
                        _readPayloadLength = (int)length;
                    }

                    //
                    // Read the mask if this is an incoming connection.
                    //
                    if (_incoming)
                    {
                        //
                        // We must have needed to read the mask.
                        //
                        Debug.Assert(_readBufferOffset - _readBufferPos >= 4);
                        for (int i = 0; i < 4; ++i)
                        {
                            _readMask[i] = _readBuffer[_readBufferPos++]; // Copy the mask.
                        }
                    }

                    switch (_readOpCode)
                    {
                        case OP_TEXT: // Text frame
                            {
                                throw new WSProtocolException("text frames not supported");
                            }
                        case OP_DATA: // Data frame
                        case OP_CONT: // Continuation frame
                            {
                                if (_instance.TraceLevel >= 2)
                                {
                                    _instance.Logger.Trace(_instance.TraceCategory, "received " + Transport() +
                                                             (_readOpCode == OP_DATA ? " data" : " continuation") +
                                                             " frame with payload length of " + _readPayloadLength +
                                                             " bytes\n" + ToString());
                                }

                                if (_readPayloadLength <= 0)
                                {
                                    throw new WSProtocolException("payload length is 0");
                                }
                                _readState = ReadStatePayload;
                                Debug.Assert(offset < buffer.Count);
                                _readFrameStart = offset;
                                break;
                            }
                        case OP_CLOSE: // Connection close
                            {
                                if (_instance.TraceLevel >= 2)
                                {
                                    _instance.Logger.Trace(_instance.TraceCategory,
                                        $"received {Transport()} connection close frame\n{this}");
                                }

                                _readState = ReadStateControlFrame;
                                int s = _nextState == StateOpened ? _state : _nextState;
                                if (s == StateClosingRequestPending)
                                {
                                    //
                                    // If we receive a close frame while we were actually
                                    // waiting to send one, change the role and send a
                                    // close frame response.
                                    //
                                    if (!_closingInitiator)
                                    {
                                        _closingInitiator = true;
                                    }
                                    if (_state == StateClosingRequestPending)
                                    {
                                        _state = StateClosingResponsePending;
                                    }
                                    else
                                    {
                                        _nextState = StateClosingResponsePending;
                                    }
                                    return false; // No longer interested in reading
                                }
                                else
                                {
                                    throw new ConnectionLostException();
                                }
                            }
                        case OP_PING:
                            {
                                if (_instance.TraceLevel >= 2)
                                {
                                    _instance.Logger.Trace(_instance.TraceCategory,
                                        $"received {Transport()} connection ping frame\n{this}");
                                }
                                _readState = ReadStateControlFrame;
                                break;
                            }
                        case OP_PONG: // Pong
                            {
                                if (_instance.TraceLevel >= 2)
                                {
                                    _instance.Logger.Trace(_instance.TraceCategory,
                                        $"received {Transport()} connection pong frame\n{this}");
                                }
                                _readState = ReadStateControlFrame;
                                break;
                            }
                        default:
                            {
                                throw new WSProtocolException($"unsupported opcode: {_readOpCode}");
                            }
                    }
                }

                if (_readState == ReadStateControlFrame)
                {
                    if (_readPayloadLength > 0 && !ReadBuffered(_readPayloadLength))
                    {
                        return true;
                    }

                    if (_readPayloadLength > 0 && _readOpCode == OP_PING)
                    {
                        _pingPayload = new byte[_readPayloadLength];
                        _readBuffer.Slice(_readBufferPos, _readPayloadLength).CopyTo(_pingPayload);
                    }

                    _readBufferPos += _readPayloadLength;
                    _readPayloadLength = 0;

                    if (_readOpCode == OP_PING)
                    {
                        if (_state == StateOpened)
                        {
                            _state = StatePongPending; // Send pong frame now
                        }
                        else if (_nextState < StatePongPending)
                        {
                            _nextState = StatePongPending; // Send pong frame next
                        }
                    }

                    //
                    // We've read the payload of the PING/PONG frame, we're ready
                    // to read a new frame.
                    //
                    _readState = ReadStateOpcode;
                }

                if (_readState == ReadStatePayload)
                {
                    //
                    // This must be assigned before the check for the buffer. If the buffer is empty
                    // or already read, postRead will return false.
                    //
                    _readStart = offset;
                    if (buffer.Count == 0 || offset == buffer.Count)
                    {
                        return false;
                    }

                    int n = Math.Min(_readBufferOffset - _readBufferPos, buffer.Count - offset);
                    if (n > _readPayloadLength)
                    {
                        n = _readPayloadLength;
                    }

                    if (n > 0)
                    {
                        _readBuffer.Slice(_readBufferPos, n).CopyTo(buffer.Slice(offset));
                        offset += n;
                        _readBufferPos += n;
                    }

                    //
                    // Continue reading if we didn't read the full message, otherwise give back
                    // the control to the connection
                    //
                    return offset < buffer.Count && n < _readPayloadLength;
                }
            }
        }

        private bool PostRead(ArraySegment<byte> buffer, int offset)
        {
            if (_readState != ReadStatePayload)
            {
                return _readStart < _readBufferOffset; // Returns true if data was read.
            }

            if (_readStart == offset)
            {
                return false; // Nothing was read or nothing to read.
            }
            Debug.Assert(_readStart < offset);

            if (_incoming)
            {
                //
                // Unmask the data we just read.
                //
                for (int n = _readStart; n < offset; ++n)
                {
                    buffer[n] = (byte)(buffer[n] ^ _readMask[(n - _readFrameStart) % 4]);
                }
            }

            _readPayloadLength -= offset - _readStart;
            _readStart = offset;
            if (_readPayloadLength == 0)
            {
                //
                // We've read the complete payload, we're ready to read a new frame.
                //
                _readState = ReadStateOpcode;
            }
            return offset < buffer.Count;
        }

        private bool PreWrite(IList<ArraySegment<byte>> buf, int size, int offset)
        {
            if (_writeState == WriteStateHeader)
            {
                if (_state == StateOpened)
                {
                    if (size == 0 || size == offset)
                    {
                        return false;
                    }
                    Debug.Assert(offset == 0);
                    PrepareWriteHeader(OP_DATA, size);

                    _writeState = WriteStatePayload;
                }
                else if (_state == StatePingPending)
                {
                    PrepareWriteHeader(OP_PING, 0); // Don't send any payload

                    _writeState = WriteStateControlFrame;
                }
                else if (_state == StatePongPending)
                {
                    PrepareWriteHeader(OP_PONG, _pingPayload.Length);
                    if (_pingPayload.Length > 0)
                    {
                        _writeBuffer.Add(_pingPayload);
                    }

                    _pingPayload = Array.Empty<byte>();
                    _writeState = WriteStateControlFrame;
                }
                else if ((_state == StateClosingRequestPending && !_closingInitiator) ||
                         (_state == StateClosingResponsePending && _closingInitiator))
                {
                    PrepareWriteHeader(OP_CLOSE, 2);
                    byte[] buffer = new byte[2];

                    short reason = System.Net.IPAddress.HostToNetworkOrder((short)_closingReason);
                    // Write closing reason
                    MemoryMarshal.Write(buffer.AsSpan(0, 2), ref reason);

                    if (!_incoming)
                    {
                        buffer[0] = (byte)(buffer[0] ^ _writeMask[0]);
                        buffer[1] = (byte)(buffer[1] ^ _writeMask[1]);
                    }
                    _writeBuffer.Add(buffer);
                    _writeState = WriteStateControlFrame;
                    _writeBufferSize += buffer.Length;
                }
                else
                {
                    Debug.Assert(_state != StateClosed);
                    return false; // Nothing to write in this state
                }

                _writePayloadLength = 0;
            }

            if (_writeState == WriteStatePayload)
            {
                //
                // For an outgoing connection, each message must be masked with a random
                // 32-bit value, so we copy the entire message into the internal buffer
                // for writing. For incoming connections we borrow the segments and add
                // them after the header.
                //
                if (_writePayloadLength == 0)
                {
                    if (_incoming)
                    {
                        Debug.Assert(_incoming);
                        int n = 0;
                        foreach (ArraySegment<byte> segment in buf)
                        {
                            _writeBuffer.Add(segment); // Borrow data from the buffer
                            n += segment.Count;
                        }
                        _writeBufferSize += n;
                        _writePayloadLength = n;
                    }
                    else
                    {
                        int n = 0;
                        foreach (ArraySegment<byte> segment in buf)
                        {
                            byte[] data = new byte[segment.Count];
                            for (int i = 0; i < segment.Count; ++i, ++n)
                            {
                                data[i] = (byte)(segment[i] ^ _writeMask[n % 4]);
                            }
                            _writeBuffer.Add(data);
                        }
                        _writeBufferSize += n;
                        _writePayloadLength = n;
                    }
                }
                return true;
            }
            else if (_writeState == WriteStateControlFrame)
            {
                return _writeBufferOffset < _writeBufferSize;
            }
            else
            {
                Debug.Assert(_writeState == WriteStateFlush);
                return true;
            }
        }

        private bool PostWrite(int size, ref int offset, int status)
        {
            if (_state > StateOpened && _writeState == WriteStateControlFrame)
            {
                if (_writeBufferOffset == _writeBufferSize)
                {
                    if (_state == StatePingPending)
                    {
                        if (_instance.TraceLevel >= 2)
                        {
                            _instance.Logger.Trace(_instance.TraceCategory,
                                $"sent {Transport()} connection ping frame\n{this}");
                        }
                    }
                    else if (_state == StatePongPending)
                    {
                        if (_instance.TraceLevel >= 2)
                        {
                            _instance.Logger.Trace(_instance.TraceCategory,
                                $"sent {Transport()} connection pong frame\n{this}");
                        }
                    }
                    else if ((_state == StateClosingRequestPending && !_closingInitiator) ||
                            (_state == StateClosingResponsePending && _closingInitiator))
                    {
                        if (_instance.TraceLevel >= 2)
                        {
                            _instance.Logger.Trace(_instance.TraceCategory,
                                $"sent {Transport()} connection close frame\n{this}");
                        }

                        if (_state == StateClosingRequestPending && !_closingInitiator)
                        {
                            _writeState = WriteStateHeader;
                            _state = StateClosingResponsePending;
                            return false;
                        }
                        else
                        {
                            throw new ConnectionLostException();
                        }
                    }
                    else if (_state == StateClosed)
                    {
                        return false;
                    }

                    _state = _nextState;
                    _nextState = StateOpened;
                    _writeState = WriteStateHeader;
                }
                else
                {
                    return status == SocketOperation.None;
                }
            }

            if (_writePayloadLength > 0 && _writeBufferOffset == _writeBufferSize)
            {
                offset = _writePayloadLength;
            }

            if (status == SocketOperation.Write && size == offset && _writeBufferSize == _writeBufferOffset)
            {
                //
                // Our buffers are empty but the delegate needs another call to write().
                //
                _writeState = WriteStateFlush;
                return false;
            }
            else if (size == offset)
            {
                _writeState = WriteStateHeader;
                if (_state == StatePingPending ||
                   _state == StatePongPending ||
                   (_state == StateClosingRequestPending && !_closingInitiator) ||
                   (_state == StateClosingResponsePending && _closingInitiator))
                {
                    return true;
                }
            }
            else if (_state == StateOpened)
            {
                return status == SocketOperation.None;
            }
            return false;
        }

        private bool ReadBuffered(int size)
        {
            if (_readBufferPos == _readBufferOffset)
            {
                _readBufferPos = 0;
                _readBufferOffset = 0;
            }
            else
            {
                int available = _readBufferOffset - _readBufferPos;
                if (available < size)
                {
                    if (_readBufferPos > 0)
                    {
                        _readBuffer.Slice(_readBufferPos, available).CopyTo(_readBuffer);
                    }
                    // The size should be always smaller than _readBufferSize,
                    // All control frames MUST have a payload length of 125 bytes
                    // or less https://tools.ietf.org/html/rfc6455#section-5.5
                    Debug.Assert(size < _readBufferSize);
                    _readBufferPos = 0;
                    _readBufferOffset = available;
                }
            }
            _readStart = _readBufferOffset;
            if (_readBufferPos + size > _readBufferOffset)
            {
                return false; // Not enough read.
            }
            Debug.Assert(_readBufferOffset > _readBufferPos);
            return true;
        }

        private void PrepareWriteHeader(byte opCode, int payloadLength)
        {
            //
            // We need to prepare the frame header.
            //
            byte[] buffer = new byte[16];
            int i = 0;
            //
            // Set the opcode - this is the one and only data frame.
            //
            buffer[i++] = (byte)(opCode | FLAG_FINAL);
            //
            // Set the payload length.
            //
            if (payloadLength <= 125)
            {
                buffer[i++] = (byte)payloadLength;
            }
            else if (payloadLength > 125 && payloadLength <= 65535)
            {
                //
                // Use an extra 16 bits to encode the payload length.
                //
                buffer[i++] = 126;
                short length = System.Net.IPAddress.HostToNetworkOrder((short)payloadLength);
                MemoryMarshal.Write(buffer.AsSpan(i, 2), ref length);
                i += 2;
            }
            else if (payloadLength > 65535)
            {
                //
                // Use an extra 64 bits to encode the payload length.
                //
                buffer[i++] = 127;
                long length = System.Net.IPAddress.HostToNetworkOrder((long)payloadLength);
                MemoryMarshal.Write(buffer.AsSpan(i, 8), ref length);
                i += 8;
            }

            if (!_incoming)
            {
                //
                // Add a random 32-bit mask to every outgoing frame, copy the payload data,
                // and apply the mask.
                //
                buffer[1] = (byte)(buffer[1] | FLAG_MASKED);
                _rand.NextBytes(_writeMask);
                Buffer.BlockCopy(_writeMask, 0, buffer, i, _writeMask.Length);
                i += _writeMask.Length;
            }
            _writeBuffer.Clear();
            _writeBuffer.Add(new ArraySegment<byte>(buffer, 0, i));
            _writeBufferSize = i;
            _writeBufferOffset = 0;
        }

        private readonly TransportInstance _instance;
        private readonly ITransceiver _delegate;
        private readonly string _host;
        private string _resource;
        private readonly bool _incoming;

        private const int StateInitializeDelegate = 0;
        private const int StateConnected = 1;
        private const int StateUpgradeRequestPending = 2;
        private const int StateUpgradeResponsePending = 3;
        private const int StateOpened = 4;
        private const int StatePingPending = 5;
        private const int StatePongPending = 6;
        private const int StateClosingRequestPending = 7;
        private const int StateClosingResponsePending = 8;
        private const int StateClosed = 9;

        private int _state;
        private int _nextState;

        private readonly HttpParser _parser;
        private string _key;

        private const int ReadStateOpcode = 0;
        private const int ReadStateHeader = 1;
        private const int ReadStateControlFrame = 2;
        private const int ReadStatePayload = 3;

        private int _readState;
        private ArraySegment<byte> _readBuffer;
        private int _readBufferOffset;
        private int _readBufferPos;
        private readonly int _readBufferSize;

        private bool _readLastFrame;
        private int _readOpCode;
        private int _readHeaderLength;
        private int _readPayloadLength;
        private int _readStart;
        private int _readFrameStart;
        private readonly byte[] _readMask;

        private const int WriteStateHeader = 0;
        private const int WriteStatePayload = 1;
        private const int WriteStateControlFrame = 2;
        private const int WriteStateFlush = 3;

        private int _writeState;
        private readonly IList<ArraySegment<byte>> _writeBuffer;
        private int _writeBufferSize;
        private int _writeBufferOffset;
        private readonly byte[] _writeMask;
        private int _writePayloadLength;

        private bool _closingInitiator;
        private int _closingReason;

        private bool _readPending;
        private bool _finishRead;
        private bool _writePending;

        private byte[] _pingPayload;

        private readonly Random _rand;

        //
        // WebSocket opcodes
        //
        private const int OP_CONT = 0x0;    // Continuation frame
        private const int OP_TEXT = 0x1;    // Text frame
        private const int OP_DATA = 0x2;    // Data frame
        // private const int OP_RES_0x3 = 0x3;    // Reserved
        // private const int OP_RES_0x4 = 0x4;    // Reserved
        // private const int OP_RES_0x5 = 0x5;    // Reserved
        // private const int OP_RES_0x6 = 0x6;    // Reserved
        // private const int OP_RES_0x7 = 0x7;    // Reserved
        private const int OP_CLOSE = 0x8;    // Connection close
        private const int OP_PING = 0x9;    // Ping
        private const int OP_PONG = 0xA;    // Pong
        // private const int OP_RES_0xB = 0xB;    // Reserved
        // private const int OP_RES_0xC = 0xC;    // Reserved
        // private const int OP_RES_0xD = 0xD;    // Reserved
        // private const int OP_RES_0xE = 0xE;    // Reserved
        // private const int OP_RES_0xF = 0xF;    // Reserved
        private const int FLAG_FINAL = 0x80;   // Last frame
        private const int FLAG_MASKED = 0x80;   // Payload is masked

        private const int CLOSURE_NORMAL = 1000;
        private const int CLOSURE_SHUTDOWN = 1001;
        private const int CLOSURE_PROTOCOL_ERROR = 1002;
        private const int CLOSURE_TOO_BIG = 1009;

        private const string IceProtocol = "ice.zeroc.com";
        private const string WsUUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        private static readonly UTF8Encoding _utf8 = new UTF8Encoding(false, true);
    }
}
