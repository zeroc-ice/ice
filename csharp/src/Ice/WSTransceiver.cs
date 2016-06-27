// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Diagnostics;
    using System.Collections.Generic;
    using System.Net.Sockets;
    using System.Security.Cryptography;
    using System.Text;

    sealed class WSTransceiver : Transceiver
    {
        public Socket fd()
        {
            return _delegate.fd();
        }

        public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData)
        {
            //
            // Delegate logs exceptions that occur during initialize(), so there's no need to trap them here.
            //
            if(_state == StateInitializeDelegate)
            {
                int op = _delegate.initialize(readBuffer, writeBuffer, ref hasMoreData);
                if(op != 0)
                {
                    return op;
                }
                _state = StateConnected;
            }

            try
            {
                if(_state == StateConnected)
                {
                    //
                    // We don't know how much we'll need to read.
                    //
                    _readBuffer.resize(1024, true);
                    _readBuffer.b.position(0);
                    _readBufferPos = 0;

                    //
                    // The server waits for the client's upgrade request, the
                    // client sends the upgrade request.
                    //
                    _state = StateUpgradeRequestPending;
                    if(!_incoming)
                    {
                        //
                        // Compose the upgrade request.
                        //
                        StringBuilder @out = new StringBuilder();
                        @out.Append("GET " + _resource + " HTTP/1.1\r\n");
                        @out.Append("Host: " + _host + "\r\n");
                        @out.Append("Upgrade: websocket\r\n");
                        @out.Append("Connection: Upgrade\r\n");
                        @out.Append("Sec-WebSocket-Protocol: " + _iceProtocol + "\r\n");
                        @out.Append("Sec-WebSocket-Version: 13\r\n");
                        @out.Append("Sec-WebSocket-Key: ");

                        //
                        // The value for Sec-WebSocket-Key is a 16-byte random number,
                        // encoded with Base64.
                        //
                        byte[] key = new byte[16];
                        _rand.NextBytes(key);
                        _key = IceUtilInternal.Base64.encode(key);
                        @out.Append(_key + "\r\n\r\n"); // EOM

                        byte[] bytes = _utf8.GetBytes(@out.ToString());
                        _writeBuffer.resize(bytes.Length, false);
                        _writeBuffer.b.position(0);
                        _writeBuffer.b.put(bytes);
                        _writeBuffer.b.flip();
                    }
                }

                //
                // Try to write the client's upgrade request.
                //
                if(_state == StateUpgradeRequestPending && !_incoming)
                {
                    if(_writeBuffer.b.hasRemaining())
                    {
                        int s = _delegate.write(_writeBuffer);
                        if(s != 0)
                        {
                            return s;
                        }
                    }
                    Debug.Assert(!_writeBuffer.b.hasRemaining());
                    _state = StateUpgradeResponsePending;
                }

                while(true)
                {
                    if(_readBuffer.b.hasRemaining())
                    {
                        int s = _delegate.read(_readBuffer, ref hasMoreData);
                        if(s == SocketOperation.Write || _readBuffer.b.position() == 0)
                        {
                            return s;
                        }
                    }

                    //
                    // Try to read the client's upgrade request or the server's response.
                    //
                    if((_state == StateUpgradeRequestPending && _incoming) ||
                       (_state == StateUpgradeResponsePending && !_incoming))
                    {
                        //
                        // Check if we have enough data for a complete message.
                        //
                        int p = _parser.isCompleteMessage(_readBuffer.b, 0, _readBuffer.b.position());
                        if(p == -1)
                        {
                            if(_readBuffer.b.hasRemaining())
                            {
                                return SocketOperation.Read;
                            }

                            //
                            // Enlarge the buffer and try to read more.
                            //
                            int oldSize = _readBuffer.b.position();
                            if(oldSize + 1024 > _instance.messageSizeMax())
                            {
                                throw new Ice.MemoryLimitException();
                            }
                            _readBuffer.resize(oldSize + 1024, true);
                            _readBuffer.b.position(oldSize);
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
                    if(_state == StateUpgradeRequestPending && _incoming)
                    {
                        if(_parser.parse(_readBuffer.b, 0, _readBufferPos))
                        {
                            handleRequest(_writeBuffer);
                            _state = StateUpgradeResponsePending;
                        }
                        else
                        {
                            throw new Ice.ProtocolException("incomplete request message");
                        }
                    }

                    if(_state == StateUpgradeResponsePending)
                    {
                        if(_incoming)
                        {
                            if(_writeBuffer.b.hasRemaining())
                            {
                                int s = _delegate.write(_writeBuffer);
                                if(s != 0)
                                {
                                    return s;
                                }
                            }
                        }
                        else
                        {
                            //
                            // Parse the server's response
                            //
                            if(_parser.parse(_readBuffer.b, 0, _readBufferPos))
                            {
                                handleResponse();
                            }
                            else
                            {
                                throw new Ice.ProtocolException("incomplete response message");
                            }
                        }
                    }
                }
                catch(WebSocketException ex)
                {
                    throw new Ice.ProtocolException(ex.Message);
                }

                _state = StateOpened;
                _nextState = StateOpened;

                hasMoreData = _readBufferPos < _readBuffer.b.position();
            }
            catch(Ice.LocalException ex)
            {
                if(_instance.traceLevel() >= 2)
                {
                    _instance.logger().trace(_instance.traceCategory(),
                        protocol() + " connection HTTP upgrade request failed\n" + ToString() + "\n" + ex);
                }
                throw;
            }

            if(_instance.traceLevel() >= 1)
            {
                if(_incoming)
                {
                    _instance.logger().trace(_instance.traceCategory(),
                        "accepted " + protocol() + " connection HTTP upgrade request\n" + ToString());
                }
                else
                {
                    _instance.logger().trace(_instance.traceCategory(),
                        protocol() + " connection HTTP upgrade request accepted\n" + ToString());
                }
            }

            return SocketOperation.None;
        }

        public int closing(bool initiator, Ice.LocalException reason)
        {
            if(_instance.traceLevel() >= 1)
            {
                _instance.logger().trace(_instance.traceCategory(),
                    "gracefully closing " + protocol() + " connection\n" + ToString());
            }

            int s = _nextState == StateOpened ? _state : _nextState;

            if(s == StateClosingRequestPending && _closingInitiator)
            {
                //
                // If we initiated a close connection but also received a
                // close connection, we assume we didn't initiated the
                // connection and we send the close frame now. This is to
                // ensure that if both peers close the connection at the same
                // time we don't hang having both peer waiting for the close
                // frame of the other.
                //
                Debug.Assert(!initiator);
                _closingInitiator = false;
                return SocketOperation.Write;
            }
            else if(s >= StateClosingRequestPending)
            {
                return SocketOperation.None;
            }

            _closingInitiator = initiator;
            if(reason is Ice.CloseConnectionException)
            {
                _closingReason = CLOSURE_NORMAL;
            }
            else if(reason is Ice.ObjectAdapterDeactivatedException ||
                    reason is Ice.CommunicatorDestroyedException)
            {
                _closingReason = CLOSURE_SHUTDOWN;
            }
            else if(reason is Ice.ProtocolException)
            {
                _closingReason  = CLOSURE_PROTOCOL_ERROR;
            }
            else if(reason is Ice.MemoryLimitException)
            {
                _closingReason = CLOSURE_TOO_BIG;
            }

            if(_state == StateOpened)
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

        public void close()
        {
            _delegate.close();
            _state = StateClosed;

            //
            // Clear the buffers now instead of waiting for destruction.
            //
            if(!_readPending)
            {
                _readBuffer.clear();
            }
            if(!_writePending)
            {
                _writeBuffer.clear();
            }
        }

        public EndpointI bind()
        {
            Debug.Assert(false);
            return null;
        }

        public void destroy()
        {
            _delegate.destroy();
        }

        public int write(Buffer buf)
        {
            if(_writePending)
            {
                return SocketOperation.Write;
            }

            if(_state < StateOpened)
            {
                if(_state < StateConnected)
                {
                    return _delegate.write(buf);
                }
                else
                {
                    return _delegate.write(_writeBuffer);
                }
            }

            int s = SocketOperation.None;
            do
            {
                if(preWrite(buf))
                {
                    if(_writeState == WriteStateFlush)
                    {
                        //
                        // Invoke write() even though there's nothing to write.
                        //
                        Debug.Assert(!buf.b.hasRemaining());
                        s = _delegate.write(buf);
                    }

                    if(s == SocketOperation.None && _writeBuffer.b.hasRemaining())
                    {
                        s = _delegate.write(_writeBuffer);
                    }
                    else if(s == SocketOperation.None && _incoming && !buf.empty() && _writeState == WriteStatePayload)
                    {
                        s = _delegate.write(buf);
                    }
                }
            }
            while(postWrite(buf, s));

            if(s != SocketOperation.None)
            {
                return s;
            }
            if(_state == StateClosingResponsePending && !_closingInitiator)
            {
                return SocketOperation.Read;
            }
            return SocketOperation.None;
        }

        public int read(Buffer buf, ref bool hasMoreData)
        {
            if(_readPending)
            {
                return SocketOperation.Read;
            }

            if(_state < StateOpened)
            {
                if(_state < StateConnected)
                {
                    return _delegate.read(buf, ref hasMoreData);
                }
                else
                {
                    if(_delegate.read(_readBuffer, ref hasMoreData) == SocketOperation.Write)
                    {
                        return SocketOperation.Write;
                    }
                    else
                    {
                        return SocketOperation.None;
                    }
                }
            }

            if(!buf.b.hasRemaining())
            {
                hasMoreData |= _readBufferPos < _readBuffer.b.position();
                return SocketOperation.None;
            }

            int s = SocketOperation.None;
            do
            {
                if(preRead(buf))
                {
                    if(_readState == ReadStatePayload)
                    {
                        //
                        // If the payload length is smaller than what remains to be read, we read
                        // no more than the payload length. The remaining of the buffer will be
                        // sent over in another frame.
                        //
                        int readSz = _readPayloadLength - (buf.b.position() - _readStart);
                        if(buf.b.remaining() > readSz)
                        {
                            int size = buf.size();
                            buf.resize(buf.b.position() + readSz, true);
                            s = _delegate.read(buf, ref hasMoreData);
                            buf.resize(size, true);
                        }
                        else
                        {
                            s = _delegate.read(buf, ref hasMoreData);
                        }
                    }
                    else
                    {
                        s = _delegate.read(_readBuffer, ref hasMoreData);
                    }

                    if(s == SocketOperation.Write)
                    {
                        postRead(buf);
                        return s;
                    }
                }
            }
            while(postRead(buf));

            if(!buf.b.hasRemaining())
            {
                hasMoreData |= _readBufferPos < _readBuffer.b.position();
                s = SocketOperation.None;
            }
            else
            {
                hasMoreData = false;
                s = SocketOperation.Read;
            }

            if(((_state == StateClosingRequestPending && !_closingInitiator) ||
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

        public bool startRead(Buffer buf, AsyncCallback callback, object state)
        {
            _readPending = true;
            if(_state < StateOpened)
            {
                _finishRead = true;
                if(_state < StateConnected)
                {
                    return _delegate.startRead(buf, callback, state);
                }
                else
                {
                    return _delegate.startRead(_readBuffer, callback, state);
                }
            }

            if(preRead(buf))
            {
                _finishRead = true;
                if(_readState == ReadStatePayload)
                {
                    //
                    // If the payload length is smaller than what remains to be read, we read
                    // no more than the payload length. The remaining of the buffer will be
                    // sent over in another frame.
                    //
                    int readSz = _readPayloadLength  - (buf.b.position() - _readStart);
                    if(buf.b.remaining() > readSz)
                    {
                        int size = buf.size();
                        buf.resize(buf.b.position() + readSz, true);
                        bool completedSynchronously = _delegate.startRead(buf, callback, state);
                        buf.resize(size, true);
                        return completedSynchronously;
                    }
                    else
                    {
                        return _delegate.startRead(buf, callback, state);
                    }
                }
                else
                {
                    return _delegate.startRead(_readBuffer, callback, state);
                }
            }
            else
            {
                return true;
            }
        }

        public void finishRead(Buffer buf)
        {
            Debug.Assert(_readPending);
            _readPending = false;

            if(_state < StateOpened)
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                if(_state < StateConnected)
                {
                    _delegate.finishRead(buf);
                }
                else
                {
                    _delegate.finishRead(_readBuffer);
                }
                return;
            }

            if(!_finishRead)
            {
                // Nothing to do.
            }
            else if(_readState == ReadStatePayload)
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                _delegate.finishRead(buf);
            }
            else
            {
                Debug.Assert(_finishRead);
                _finishRead = false;
                _delegate.finishRead(_readBuffer);
            }

            if(_state == StateClosed)
            {
                _readBuffer.clear();
                return;
            }

            postRead(buf);
        }

        public bool startWrite(Buffer buf, AsyncCallback callback, object state,
                               out bool completed)
        {
            _writePending = true;
            if(_state < StateOpened)
            {
                if(_state < StateConnected)
                {
                    return _delegate.startWrite(buf, callback, state, out completed);
                }
                else
                {
                    return _delegate.startWrite(_writeBuffer, callback, state, out completed);
                }
            }

            if(preWrite(buf))
            {
                if(_writeBuffer.b.hasRemaining())
                {
                    return _delegate.startWrite(_writeBuffer, callback, state, out completed);
                }
                else
                {
                    Debug.Assert(_incoming);
                    return _delegate.startWrite(buf, callback, state, out completed);
                }
            }
            else
            {
                completed = true;
                return false;
            }
        }

        public void finishWrite(Buffer buf)
        {
            _writePending = false;

            if(_state < StateOpened)
            {
                if(_state < StateConnected)
                {
                    _delegate.finishWrite(buf);
                }
                else
                {
                    _delegate.finishWrite(_writeBuffer);
                }
                return;
            }

            if(_writeBuffer.b.hasRemaining())
            {
                _delegate.finishWrite(_writeBuffer);
            }
            else if(!buf.empty() && buf.b.hasRemaining())
            {
                Debug.Assert(_incoming);
                _delegate.finishWrite(buf);
            }

            if(_state == StateClosed)
            {
                _writeBuffer.clear();
                return;
            }

            postWrite(buf, SocketOperation.None);
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public Ice.ConnectionInfo getInfo()
        {
            Ice.WSConnectionInfo info = new Ice.WSConnectionInfo();
            info.headers = _parser.getHeaders();
            info.underlying = _delegate.getInfo();
            return info;
        }

        public void checkSendSize(Buffer buf)
        {
            _delegate.checkSendSize(buf);
        }

        public void setBufferSize(int rcvSize, int sndSize)
        {
            _delegate.setBufferSize(rcvSize, sndSize);
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public string toDetailedString()
        {
            return _delegate.toDetailedString();
        }

        internal
        WSTransceiver(ProtocolInstance instance, Transceiver del, string host, string resource)
        {
            init(instance, del);
            _host = host;
            _resource = resource;
            _incoming = false;

            //
            // Use a 16KB write buffer size. We use 16KB for the write
            // buffer size because all the data needs to be copied to the
            // write buffer for the purpose of masking. A 16KB buffer
            // appears to be a good compromise to reduce the number of
            // socket write calls and not consume too much memory.
            //
            _writeBufferSize = 16 * 1024;

            //
            // Write and read buffer size must be large enough to hold the frame header!
            //
            Debug.Assert(_writeBufferSize > 256);
            Debug.Assert(_readBufferSize > 256);
        }

        internal WSTransceiver(ProtocolInstance instance, Transceiver del)
        {
            init(instance, del);
            _host = "";
            _resource = "";
            _incoming = true;

            //
            // Write and read buffer size must be large enough to hold the frame header!
            //
            Debug.Assert(_writeBufferSize > 256);
            Debug.Assert(_readBufferSize > 256);
        }

        private void init(ProtocolInstance instance, Transceiver del)
        {
            _instance = instance;
            _delegate = del;
            _state = StateInitializeDelegate;
            _parser = new HttpParser();
            _readState = ReadStateOpcode;
            _readBuffer = new Buffer(ByteBuffer.ByteOrder.BIG_ENDIAN); // Network byte order
            _readBufferSize = 1024;
            _readLastFrame = true;
            _readOpCode = 0;
            _readHeaderLength = 0;
            _readPayloadLength = 0;
            _writeState = WriteStateHeader;
            _writeBuffer = new Buffer(ByteBuffer.ByteOrder.BIG_ENDIAN); // Network byte order
            _writeBufferSize = 1024;
            _readPending = false;
            _finishRead = false;
            _writePending = false;
            _readMask = new byte[4];
            _writeMask = new byte[4];
            _key = "";
            _pingPayload = new byte[0];
            _rand = new Random();
        }

        private void handleRequest(Buffer responseBuffer)
        {
            //
            // HTTP/1.1
            //
            if(_parser.versionMajor() != 1 || _parser.versionMinor() != 1)
            {
                throw new WebSocketException("unsupported HTTP version");
            }

            //
            // "An |Upgrade| header field containing the value 'websocket',
            //  treated as an ASCII case-insensitive value."
            //
            string val = _parser.getHeader("Upgrade", true);
            if(val == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if(!val.Equals("websocket"))
            {
                throw new WebSocketException("invalid value `" + val + "' for Upgrade field");
            }

            //
            // "A |Connection| header field that includes the token 'Upgrade',
            //  treated as an ASCII case-insensitive value.
            //
            val = _parser.getHeader("Connection", true);
            if(val == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if(val.IndexOf("upgrade") == -1)
            {
                throw new WebSocketException("invalid value `" + val + "' for Connection field");
            }

            //
            // "A |Sec-WebSocket-Version| header field, with a value of 13."
            //
            val = _parser.getHeader("Sec-WebSocket-Version", false);
            if(val == null)
            {
                throw new WebSocketException("missing value for WebSocket version");
            }
            else if(!val.Equals("13"))
            {
                throw new WebSocketException("unsupported WebSocket version `" + val + "'");
            }

            //
            // "Optionally, a |Sec-WebSocket-Protocol| header field, with a list
            //  of values indicating which protocols the client would like to
            //  speak, ordered by preference."
            //
            bool addProtocol = false;
            val = _parser.getHeader("Sec-WebSocket-Protocol", true);
            if(val != null)
            {
                string[] protocols = IceUtilInternal.StringUtil.splitString(val, ",");
                if(protocols == null)
                {
                    throw new WebSocketException("invalid value `" + val + "' for WebSocket protocol");
                }
                foreach(string p in protocols)
                {
                    if(!p.Trim().Equals(_iceProtocol))
                    {
                        throw new WebSocketException("unknown value `" + p + "' for WebSocket protocol");
                    }
                    addProtocol = true;
                }
            }

            //
            // "A |Sec-WebSocket-Key| header field with a base64-encoded
            //  value that, when decoded, is 16 bytes in length."
            //
            string key = _parser.getHeader("Sec-WebSocket-Key", false);
            if(key == null)
            {
                throw new WebSocketException("missing value for WebSocket key");
            }

            byte[] decodedKey = IceUtilInternal.Base64.decode(key);
            if(decodedKey.Length != 16)
            {
                throw new WebSocketException("invalid value `" + key + "' for WebSocket key");
            }

            //
            // Retain the target resource.
            //
            _resource = _parser.uri();

            //
            // Compose the response.
            //
            StringBuilder @out = new StringBuilder();
            @out.Append("HTTP/1.1 101 Switching Protocols\r\n");
            @out.Append("Upgrade: websocket\r\n");
            @out.Append("Connection: Upgrade\r\n");
            if(addProtocol)
            {
                @out.Append("Sec-WebSocket-Protocol: " + _iceProtocol + "\r\n");
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
            @out.Append("Sec-WebSocket-Accept: ");
            string input = key + _wsUUID;
            byte[] hash = SHA1.Create().ComputeHash(_utf8.GetBytes(input));
            @out.Append(IceUtilInternal.Base64.encode(hash) + "\r\n" + "\r\n"); // EOM

            byte[] bytes = _utf8.GetBytes(@out.ToString());
            Debug.Assert(bytes.Length == @out.Length);
            responseBuffer.resize(bytes.Length, false);
            responseBuffer.b.position(0);
            responseBuffer.b.put(bytes);
            responseBuffer.b.flip();
        }

        private void handleResponse()
        {
            string val;

            //
            // HTTP/1.1
            //
            if(_parser.versionMajor() != 1 || _parser.versionMinor() != 1)
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
            if(_parser.status() != 101)
            {
                StringBuilder @out = new StringBuilder("unexpected status value " + _parser.status());
                if(_parser.reason().Length > 0)
                {
                    @out.Append(":\n" + _parser.reason());
                }
                throw new WebSocketException(@out.ToString());
            }

            //
            // "If the response lacks an |Upgrade| header field or the |Upgrade|
            //  header field contains a value that is not an ASCII case-
            //  insensitive match for the value "websocket", the client MUST
            //  _Fail the WebSocket Connection_."
            //
            val = _parser.getHeader("Upgrade", true);
            if(val == null)
            {
                throw new WebSocketException("missing value for Upgrade field");
            }
            else if(!val.Equals("websocket"))
            {
                throw new WebSocketException("invalid value `" + val + "' for Upgrade field");
            }

            //
            // "If the response lacks a |Connection| header field or the
            //  |Connection| header field doesn't contain a token that is an
            //  ASCII case-insensitive match for the value "Upgrade", the client
            //  MUST _Fail the WebSocket Connection_."
            //
            val = _parser.getHeader("Connection", true);
            if(val == null)
            {
                throw new WebSocketException("missing value for Connection field");
            }
            else if(val.IndexOf("upgrade") == -1)
            {
                throw new WebSocketException("invalid value `" + val + "' for Connection field");
            }

            //
            // "If the response includes a |Sec-WebSocket-Protocol| header field
            //  and this header field indicates the use of a subprotocol that was
            //  not present in the client's handshake (the server has indicated a
            //  subprotocol not requested by the client), the client MUST _Fail
            //  the WebSocket Connection_."
            //
            val = _parser.getHeader("Sec-WebSocket-Protocol", true);
            if(val != null && !val.Equals(_iceProtocol))
            {
                throw new WebSocketException("invalid value `" + val + "' for WebSocket protocol");
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
            val = _parser.getHeader("Sec-WebSocket-Accept", false);
            if(val == null)
            {
                throw new WebSocketException("missing value for Sec-WebSocket-Accept");
            }

            string input = _key + _wsUUID;
            byte[] hash = SHA1.Create().ComputeHash(_utf8.GetBytes(input));
            if(!val.Equals(IceUtilInternal.Base64.encode(hash)))
            {
                throw new WebSocketException("invalid value `" + val + "' for Sec-WebSocket-Accept");
            }
        }

        private bool preRead(Buffer buf)
        {
            while(true)
            {
                if(_readState == ReadStateOpcode)
                {
                    //
                    // Is there enough data available to read the opcode?
                    //
                    if(!readBuffered(2))
                    {
                        return true;
                    }

                    //
                    // Most-significant bit indicates whether this is the
                    // last frame. Least-significant four bits hold the
                    // opcode.
                    //
                    int ch = _readBuffer.b.get(_readBufferPos++);
                    _readOpCode = ch & 0xf;

                    //
                    // Remember if last frame if we're going to read a data or
                    // continuation frame, this is only for protocol
                    // correctness checking purpose.
                    //
                    if(_readOpCode == OP_DATA)
                    {
                        if(!_readLastFrame)
                        {
                            throw new Ice.ProtocolException("invalid data frame, no FIN on previous frame");
                        }
                        _readLastFrame = (ch & FLAG_FINAL) == FLAG_FINAL;
                    }
                    else if(_readOpCode == OP_CONT)
                    {
                        if(_readLastFrame)
                        {
                            throw new Ice.ProtocolException("invalid continuation frame, previous frame FIN set");
                        }
                        _readLastFrame = (ch & FLAG_FINAL) == FLAG_FINAL;
                    }

                    ch = _readBuffer.b.get(_readBufferPos++);

                    //
                    // Check the MASK bit. Messages sent by a client must be masked;
                    // messages sent by a server must not be masked.
                    //
                    bool masked = (ch & FLAG_MASKED) == FLAG_MASKED;
                    if(masked != _incoming)
                    {
                        throw new Ice.ProtocolException("invalid masking");
                    }

                    //
                    // Extract the payload length, which can have the following values:
                    //
                    // 0-125: The payload length
                    // 126:   The subsequent two bytes contain the payload length
                    // 127:   The subsequent eight bytes contain the payload length
                    //
                    _readPayloadLength = (ch & 0x7f);
                    if(_readPayloadLength < 126)
                    {
                        _readHeaderLength = 0;
                    }
                    else if(_readPayloadLength == 126)
                    {
                        _readHeaderLength = 2; // Need to read a 16-bit payload length.
                    }
                    else
                    {
                        _readHeaderLength = 8; // Need to read a 64-bit payload length.
                    }
                    if(masked)
                    {
                        _readHeaderLength += 4; // Need to read a 32-bit mask.
                    }

                    _readState = ReadStateHeader;
                }

                if(_readState == ReadStateHeader)
                {
                    //
                    // Is there enough data available to read the header?
                    //
                    if(_readHeaderLength > 0 && !readBuffered(_readHeaderLength))
                    {
                        return true;
                    }

                    if(_readPayloadLength == 126)
                    {
                        _readPayloadLength = _readBuffer.b.getShort(_readBufferPos); // Uses network byte order.
                        if(_readPayloadLength < 0)
                        {
                            _readPayloadLength += 65536;
                        }
                        _readBufferPos += 2;
                    }
                    else if(_readPayloadLength == 127)
                    {
                        long l = _readBuffer.b.getLong(_readBufferPos); // Uses network byte order.
                        _readBufferPos += 8;
                        if(l < 0 || l > Int32.MaxValue)
                        {
                            throw new Ice.ProtocolException("invalid WebSocket payload length: " + l);
                        }
                        _readPayloadLength = (int)l;
                    }

                    //
                    // Read the mask if this is an incoming connection.
                    //
                    if(_incoming)
                    {
                        //
                        // We must have needed to read the mask.
                        //
                        Debug.Assert(_readBuffer.b.position() - _readBufferPos >= 4);
                        for(int i = 0; i < 4; ++i)
                        {
                            _readMask[i] = _readBuffer.b.get(_readBufferPos++); // Copy the mask.
                        }
                    }

                    switch(_readOpCode)
                    {
                    case OP_TEXT: // Text frame
                    {
                        throw new Ice.ProtocolException("text frames not supported");
                    }
                    case OP_DATA: // Data frame
                    case OP_CONT: // Continuation frame
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(), "received " + protocol() +
                                                     (_readOpCode == OP_DATA ? " data" : " continuation") +
                                                     " frame with payload length of " + _readPayloadLength +
                                                     " bytes\n" + ToString());
                        }

                        if(_readPayloadLength <= 0)
                        {
                            throw new Ice.ProtocolException("payload length is 0");
                        }
                        _readState = ReadStatePayload;
                        Debug.Assert(buf.b.hasRemaining());
                        _readFrameStart = buf.b.position();
                        break;
                    }
                    case OP_CLOSE: // Connection close
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "received " + protocol() + " connection close frame\n" + ToString());
                        }

                        _readState = ReadStateControlFrame;
                        int s = _nextState == StateOpened ? _state : _nextState;
                        if(s == StateClosingRequestPending)
                        {
                            //
                            // If we receive a close frame while we were actually
                            // waiting to send one, change the role and send a
                            // close frame response.
                            //
                            if(!_closingInitiator)
                            {
                                _closingInitiator = true;
                            }
                            if(_state == StateClosingRequestPending)
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
                            throw new Ice.ConnectionLostException();
                        }
                    }
                    case OP_PING:
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "received " + protocol() + " connection ping frame\n" + ToString());
                        }
                        _readState = ReadStateControlFrame;
                        break;
                    }
                    case OP_PONG: // Pong
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "received " + protocol() + " connection pong frame\n" + ToString());
                        }
                        _readState = ReadStateControlFrame;
                        break;
                    }
                    default:
                    {
                        throw new Ice.ProtocolException("unsupported opcode: " + _readOpCode);
                    }
                    }
                }

                if(_readState == ReadStateControlFrame)
                {
                    if(_readPayloadLength > 0 && !readBuffered(_readPayloadLength))
                    {
                        return true;
                    }

                    if(_readPayloadLength > 0 && _readOpCode == OP_PING)
                    {
                        _pingPayload = new byte[_readPayloadLength];
                        System.Buffer.BlockCopy(_readBuffer.b.rawBytes(), _readBufferPos, _pingPayload, 0,
                                                _readPayloadLength);
                    }

                    _readBufferPos += _readPayloadLength;
                    _readPayloadLength = 0;

                    if(_readOpCode == OP_PING)
                    {
                        if(_state == StateOpened)
                        {
                            _state = StatePongPending; // Send pong frame now
                        }
                        else if(_nextState < StatePongPending)
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

                if(_readState == ReadStatePayload)
                {
                    //
                    // This must be assigned before the check for the buffer. If the buffer is empty
                    // or already read, postRead will return false.
                    //
                    _readStart = buf.b.position();

                    if(buf.empty() || !buf.b.hasRemaining())
                    {
                        return false;
                    }

                    int n = Math.Min(_readBuffer.b.position() - _readBufferPos, buf.b.remaining());
                    if(n > _readPayloadLength)
                    {
                        n = _readPayloadLength;
                    }
                    if(n > 0)
                    {
                        System.Buffer.BlockCopy(_readBuffer.b.rawBytes(), _readBufferPos, buf.b.rawBytes(),
                                                buf.b.position(), n);
                        buf.b.position(buf.b.position() + n);
                        _readBufferPos += n;
                    }

                    //
                    // Continue reading if we didn't read the full message, otherwise give back
                    // the control to the connection
                    //
                    return buf.b.hasRemaining() && n < _readPayloadLength;
                }
            }
        }

        private bool postRead(Buffer buf)
        {
            if(_readState != ReadStatePayload)
            {
                return _readStart < _readBuffer.b.position(); // Returns true if data was read.
            }

            if(_readStart == buf.b.position())
            {
                return false; // Nothing was read or nothing to read.
            }
            Debug.Assert(_readStart < buf.b.position());

            if(_incoming)
            {
                //
                // Unmask the data we just read.
                //
                int pos = buf.b.position();
                byte[] arr = buf.b.rawBytes();
                for(int n = _readStart; n < pos; ++n)
                {
                    arr[n] = (byte)(arr[n] ^ _readMask[(n - _readFrameStart) % 4]);
                }
            }

            _readPayloadLength -= buf.b.position() - _readStart;
            _readStart = buf.b.position();
            if(_readPayloadLength == 0)
            {
                //
                // We've read the complete payload, we're ready to read a new frame.
                //
                _readState = ReadStateOpcode;
            }
            return buf.b.hasRemaining();
        }

        private bool preWrite(Buffer buf)
        {
            if(_writeState == WriteStateHeader)
            {
                if(_state == StateOpened)
                {
                    if(buf.empty() || !buf.b.hasRemaining())
                    {
                        return false;
                    }

                    Debug.Assert(buf.b.position() == 0);
                    prepareWriteHeader((byte)OP_DATA, buf.size());

                    _writeState = WriteStatePayload;
                }
                else if(_state == StatePingPending)
                {
                    prepareWriteHeader((byte)OP_PING, 0); // Don't send any payload

                    _writeState = WriteStateControlFrame;
                    _writeBuffer.b.flip();
                }
                else if(_state == StatePongPending)
                {
                    prepareWriteHeader((byte)OP_PONG, _pingPayload.Length);
                    if(_pingPayload.Length > _writeBuffer.b.remaining())
                    {
                        int pos = _writeBuffer.b.position();
                        _writeBuffer.resize(pos + _pingPayload.Length, false);
                        _writeBuffer.b.position(pos);
                    }
                    _writeBuffer.b.put(_pingPayload);
                    _pingPayload = new byte[0];

                    _writeState = WriteStateControlFrame;
                    _writeBuffer.b.flip();
                }
                else if((_state == StateClosingRequestPending && !_closingInitiator) ||
                        (_state == StateClosingResponsePending && _closingInitiator))
                {
                    prepareWriteHeader((byte)OP_CLOSE, 2);

                    // Write closing reason
                    _writeBuffer.b.putShort((short)_closingReason);

                    if(!_incoming)
                    {
                        byte b;
                        int pos = _writeBuffer.b.position() - 2;
                        b = (byte)(_writeBuffer.b.get(pos) ^ _writeMask[0]);
                        _writeBuffer.b.put(pos, b);
                        pos++;
                        b = (byte)(_writeBuffer.b.get(pos) ^ _writeMask[1]);
                        _writeBuffer.b.put(pos, b);
                    }

                    _writeState = WriteStateControlFrame;
                    _writeBuffer.b.flip();
                }
                else
                {
                    Debug.Assert(_state != StateClosed);
                    return false; // Nothing to write in this state
                }

                _writePayloadLength = 0;
            }

            if(_writeState == WriteStatePayload)
            {
                //
                // For an outgoing connection, each message must be masked with a random
                // 32-bit value, so we copy the entire message into the internal buffer
                // for writing. For incoming connections, we just copy the start of the
                // message in the internal buffer after the hedaer. If the message is
                // larger, the reminder is sent directly from the message buffer to avoid
                // copying.
                //
                if(!_incoming && (_writePayloadLength == 0 || !_writeBuffer.b.hasRemaining()))
                {
                    if(!_writeBuffer.b.hasRemaining())
                    {
                        _writeBuffer.b.position(0);
                    }

                    int n = buf.b.position();
                    int sz = buf.size();
                    int pos = _writeBuffer.b.position();
                    int count = Math.Min(sz - n, _writeBuffer.b.remaining());
                    byte[] src = buf.b.rawBytes();
                    byte[] dest = _writeBuffer.b.rawBytes();
                    for(int i = 0; i < count; ++i, ++n, ++pos)
                    {
                        dest[pos] = (byte)(src[n] ^ _writeMask[n % 4]);
                    }
                    _writeBuffer.b.position(pos);
                    _writePayloadLength = n;

                    _writeBuffer.b.flip();
                }
                else if(_writePayloadLength == 0)
                {
                    Debug.Assert(_incoming);
                    if(_writeBuffer.b.hasRemaining())
                    {
                        Debug.Assert(buf.b.position() == 0);
                        int n = Math.Min(_writeBuffer.b.remaining(), buf.b.remaining());
                        int pos = _writeBuffer.b.position();
                        System.Buffer.BlockCopy(buf.b.rawBytes(), 0, _writeBuffer.b.rawBytes(), pos, n);
                        _writeBuffer.b.position(pos + n);
                        _writePayloadLength = n;
                   }
                    _writeBuffer.b.flip();
                }
                return true;
            }
            else if(_writeState == WriteStateControlFrame)
            {
                return _writeBuffer.b.hasRemaining();
            }
            else
            {
                Debug.Assert(_writeState == WriteStateFlush);
                return true;
            }
        }

        private bool postWrite(Buffer buf, int status)
        {
            if(_state > StateOpened && _writeState == WriteStateControlFrame)
            {
                if(!_writeBuffer.b.hasRemaining())
                {
                    if(_state == StatePingPending)
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "sent " + protocol() + " connection ping frame\n" + ToString());
                        }
                    }
                    else if(_state == StatePongPending)
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "sent " + protocol() + " connection pong frame\n" + ToString());
                        }
                    }
                    else if((_state == StateClosingRequestPending && !_closingInitiator) ||
                            (_state == StateClosingResponsePending && _closingInitiator))
                    {
                        if(_instance.traceLevel() >= 2)
                        {
                            _instance.logger().trace(_instance.traceCategory(),
                                "sent " + protocol() + " connection close frame\n" + ToString());
                        }

                        if(_state == StateClosingRequestPending && !_closingInitiator)
                        {
                            _writeState = WriteStateHeader;
                            _state = StateClosingResponsePending;
                            return false;
                        }
                        else
                        {
                            throw new Ice.ConnectionLostException();
                        }
                    }
                    else if(_state == StateClosed)
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

            if((!_incoming || buf.b.position() == 0) && _writePayloadLength > 0)
            {
                if(!_writeBuffer.b.hasRemaining())
                {
                    buf.b.position(_writePayloadLength);
                }
            }

            if(status == SocketOperation.Write && !buf.b.hasRemaining() && !_writeBuffer.b.hasRemaining())
            {
                //
                // Our buffers are empty but the delegate needs another call to write().
                //
                _writeState = WriteStateFlush;
                return false;
            }
            else if(!buf.b.hasRemaining())
            {
                _writeState = WriteStateHeader;
                if(_state == StatePingPending ||
                   _state == StatePongPending ||
                   (_state == StateClosingRequestPending && !_closingInitiator) ||
                   (_state == StateClosingResponsePending && _closingInitiator))
                {
                    return true;
                }
            }
            else if(_state == StateOpened)
            {
                return status == SocketOperation.None;
            }

            return false;
        }

        private bool readBuffered(int sz)
        {
            if(_readBufferPos == _readBuffer.b.position())
            {
                _readBuffer.resize(_readBufferSize, true);
                _readBufferPos = 0;
                _readBuffer.b.position(0);
            }
            else
            {
                int available = _readBuffer.b.position() - _readBufferPos;
                if(available < sz)
                {
                    if(_readBufferPos > 0)
                    {
                        _readBuffer.b.limit(_readBuffer.b.position());
                        _readBuffer.b.position(_readBufferPos);
                        _readBuffer.b.compact();
                        Debug.Assert(_readBuffer.b.position() == available);
                    }
                    _readBuffer.resize(Math.Max(_readBufferSize, sz), true);
                    _readBufferPos = 0;
                    _readBuffer.b.position(available);
                }
            }

            _readStart = _readBuffer.b.position();
            if(_readBufferPos + sz > _readBuffer.b.position())
            {
                return false; // Not enough read.
            }
            Debug.Assert(_readBuffer.b.position() > _readBufferPos);
            return true;
        }

        private void prepareWriteHeader(byte opCode, int payloadLength)
        {
            //
            // We need to prepare the frame header.
            //
            _writeBuffer.resize(_writeBufferSize, false);
            _writeBuffer.b.limit(_writeBufferSize);
            _writeBuffer.b.position(0);

            //
            // Set the opcode - this is the one and only data frame.
            //
            _writeBuffer.b.put((byte)(opCode | FLAG_FINAL));

            //
            // Set the payload length.
            //
            if(payloadLength <= 125)
            {
                _writeBuffer.b.put((byte)payloadLength);
            }
            else if(payloadLength > 125 && payloadLength <= 65535)
            {
                //
                // Use an extra 16 bits to encode the payload length.
                //
                _writeBuffer.b.put((byte)126);
                _writeBuffer.b.putShort((short)payloadLength);
            }
            else if(payloadLength > 65535)
            {
                //
                // Use an extra 64 bits to encode the payload length.
                //
                _writeBuffer.b.put((byte)127);
                _writeBuffer.b.putLong(payloadLength);
            }

            if(!_incoming)
            {
                //
                // Add a random 32-bit mask to every outgoing frame, copy the payload data,
                // and apply the mask.
                //
                _writeBuffer.b.put(1, (byte)(_writeBuffer.b.get(1) | FLAG_MASKED));
                _rand.NextBytes(_writeMask);
                _writeBuffer.b.put(_writeMask);
            }
        }

        private ProtocolInstance _instance;
        private Transceiver _delegate;
        private string _host;
        private string _resource;
        private bool _incoming;

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

        private HttpParser _parser;
        private string _key;

        private const int ReadStateOpcode = 0;
        private const int ReadStateHeader = 1;
        private const int ReadStateControlFrame = 2;
        private const int ReadStatePayload = 3;

        private int _readState;
        private Buffer _readBuffer;
        private int _readBufferPos;
        private int _readBufferSize;

        private bool _readLastFrame;
        private int _readOpCode;
        private int _readHeaderLength;
        private int _readPayloadLength;
        private int _readStart;
        private int _readFrameStart;
        private byte[] _readMask;

        private const int WriteStateHeader = 0;
        private const int WriteStatePayload = 1;
        private const int WriteStateControlFrame = 2;
        private const int WriteStateFlush = 3;

        private int _writeState;
        private Buffer _writeBuffer;
        private int _writeBufferSize;
        private byte[] _writeMask;
        private int _writePayloadLength;

        private bool _closingInitiator;
        private int _closingReason;

        private bool _readPending;
        private bool _finishRead;
        private bool _writePending;

        private byte[] _pingPayload;

        private Random _rand;

        //
        // WebSocket opcodes
        //
        private const int OP_CONT     = 0x0;    // Continuation frame
        private const int OP_TEXT     = 0x1;    // Text frame
        private const int OP_DATA     = 0x2;    // Data frame
        private const int OP_RES_0x3  = 0x3;    // Reserved
        private const int OP_RES_0x4  = 0x4;    // Reserved
        private const int OP_RES_0x5  = 0x5;    // Reserved
        private const int OP_RES_0x6  = 0x6;    // Reserved
        private const int OP_RES_0x7  = 0x7;    // Reserved
        private const int OP_CLOSE    = 0x8;    // Connection close
        private const int OP_PING     = 0x9;    // Ping
        private const int OP_PONG     = 0xA;    // Pong
        private const int OP_RES_0xB  = 0xB;    // Reserved
        private const int OP_RES_0xC  = 0xC;    // Reserved
        private const int OP_RES_0xD  = 0xD;    // Reserved
        private const int OP_RES_0xE  = 0xE;    // Reserved
        private const int OP_RES_0xF  = 0xF;    // Reserved
        private const int FLAG_FINAL  = 0x80;   // Last frame
        private const int FLAG_MASKED = 0x80;   // Payload is masked

        private const int CLOSURE_NORMAL         = 1000;
        private const int CLOSURE_SHUTDOWN       = 1001;
        private const int CLOSURE_PROTOCOL_ERROR = 1002;
        private const int CLOSURE_TOO_BIG        = 1009;

        private const string _iceProtocol = "ice.zeroc.com";
        private const string _wsUUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        private static System.Text.UTF8Encoding _utf8 = new System.Text.UTF8Encoding(false, true);
    }
}
