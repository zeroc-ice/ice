// Copyright (c) ZeroC, Inc.

#include "WSTransceiver.h"
#include "Base64.h"
#include "Endian.h"
#include "HttpParser.h"
#include "Ice/Buffer.h"
#include "Ice/Connection.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/StringUtil.h"
#include "ProtocolInstance.h"
#include "Random.h"
#include "SHA1.h"

#include <climits>
#include <cstdint>

using namespace std;
using namespace Ice;
using namespace IceInternal;

//
// WebSocket opcodes
//
#define OP_CONT 0x0      // Continuation frame
#define OP_TEXT 0x1      // Text frame
#define OP_DATA 0x2      // Data frame
#define OP_RES_0x3 0x3   // Reserved
#define OP_RES_0x4 0x4   // Reserved
#define OP_RES_0x5 0x5   // Reserved
#define OP_RES_0x6 0x6   // Reserved
#define OP_RES_0x7 0x7   // Reserved
#define OP_CLOSE 0x8     // Connection close
#define OP_PING 0x9      // Ping
#define OP_PONG 0xA      // Pong
#define OP_RES_0xB 0xB   // Reserved
#define OP_RES_0xC 0xC   // Reserved
#define OP_RES_0xD 0xD   // Reserved
#define OP_RES_0xE 0xE   // Reserved
#define OP_RES_0xF 0xF   // Reserved
#define FLAG_FINAL 0x80  // Last frame
#define FLAG_MASKED 0x80 // Payload is masked

#define CLOSURE_NORMAL 1000
#define CLOSURE_SHUTDOWN 1001
#define CLOSURE_PROTOCOL_ERROR 1002

namespace
{
    const string _iceProtocol = "ice.zeroc.com";                   // NOLINT(cert-err58-cpp)
    const string _wsUUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // NOLINT(cert-err58-cpp)

    //
    // Rename to avoid conflict with OS 10.10 htonll
    //
    void ice_htonll(int64_t v, byte* dest)
    {
        //
        // Transfer a 64-bit integer in network (big-endian) order.
        //
        if constexpr (endian::native == endian::big)
        {
            const byte* src = reinterpret_cast<const byte*>(&v);
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest = *src;
        }
        else
        {
            const byte* src = reinterpret_cast<const byte*>(&v) + sizeof(int64_t) - 1;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest++ = *src--;
            *dest = *src;
        }
    }

    //
    // Rename to avoid conflict with OS 10.10 nlltoh
    //
    int64_t ice_nlltoh(const byte* src)
    {
        int64_t v;

        //
        // Extract a 64-bit integer in network (big-endian) order.
        //
        if constexpr (endian::native == endian::big)
        {
            byte* dest = reinterpret_cast<byte*>(&v);
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            *dest = *src;
        }
        else
        {
            byte* dest = reinterpret_cast<byte*>(&v) + sizeof(int64_t) - 1;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest-- = *src++;
            *dest = *src;
        }

        return v;
    }
}

NativeInfoPtr
IceInternal::WSTransceiver::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
AsyncInfo*
IceInternal::WSTransceiver::getAsyncInfo(SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

SocketOperation
IceInternal::WSTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    //
    // Delegate logs exceptions that occur during initialize(), so there's no need to trap them here.
    //
    if (_state == StateInitializeDelegate)
    {
        SocketOperation op = _delegate->initialize(readBuffer, writeBuffer);
        if (op != SocketOperationNone)
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
            _readBuffer.b.resize(_readBufferSize);
            _readI = _readBuffer.i = _readBuffer.b.begin();

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
                ostringstream out;
                out << "GET " << _resource << " HTTP/1.1\r\n"
                    << "Host: " << _host << "\r\n"
                    << "Upgrade: websocket\r\n"
                    << "Connection: Upgrade\r\n"
                    << "Sec-WebSocket-Protocol: " << _iceProtocol << "\r\n"
                    << "Sec-WebSocket-Version: 13\r\n"
                    << "Sec-WebSocket-Key: ";

                //
                // The value for Sec-WebSocket-Key is a 16-byte random number,
                // encoded with Base64.
                //
                vector<byte> key(16);
                IceInternal::generateRandom(reinterpret_cast<char*>(&key[0]), key.size());
                _key = IceInternal::Base64::encode(key);
                out << _key << "\r\n\r\n"; // EOM

                string str = out.str();
                _writeBuffer.b.resize(str.size());
                memcpy(&_writeBuffer.b[0], str.c_str(), str.size());
                _writeBuffer.i = _writeBuffer.b.begin();
            }
        }

        //
        // Try to write the client's upgrade request.
        //
        if (_state == StateUpgradeRequestPending && !_incoming)
        {
            if (_writeBuffer.i < _writeBuffer.b.end())
            {
                SocketOperation s = _delegate->write(_writeBuffer);
                if (s)
                {
                    return s;
                }
            }
            assert(_writeBuffer.i == _writeBuffer.b.end());
            _state = StateUpgradeResponsePending;

            if (_instance->traceLevel() >= 1)
            {
                Trace out(_instance->logger(), _instance->traceCategory());
                out << "sent " << protocol() << " connection HTTP upgrade request\n" << toString();
            }
        }

        while (true)
        {
            if (_readBuffer.i < _readBuffer.b.end())
            {
                SocketOperation s = _delegate->read(_readBuffer);
                if (s == SocketOperationWrite || _readBuffer.i == _readBuffer.b.begin())
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
                const byte* p = _parser->isCompleteMessage(&_readBuffer.b[0], _readBuffer.i);
                if (!p)
                {
                    if (_readBuffer.i < _readBuffer.b.end())
                    {
                        return SocketOperationRead;
                    }

                    //
                    // Enlarge the buffer and try to read more.
                    //
                    const auto oldSize = static_cast<size_t>(_readBuffer.i - _readBuffer.b.begin());
                    if (oldSize + 1024 > static_cast<size_t>(_instance->messageSizeMax()))
                    {
                        Ex::throwMemoryLimitException(__FILE__, __LINE__, oldSize + 1024, _instance->messageSizeMax());
                    }
                    _readBuffer.b.resize(oldSize + 1024);
                    _readBuffer.i = _readBuffer.b.begin() + oldSize;
                    continue; // Try again to read the response/request
                }

                //
                // Set _readI at the end of the response/request message.
                //
                _readI = _readBuffer.b.begin() + (p - &_readBuffer.b[0]);
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
                if (_parser->parse(&_readBuffer.b[0], _readI))
                {
                    handleRequest(_writeBuffer);
                    _state = StateUpgradeResponsePending;
                }
                else
                {
                    throw ProtocolException(__FILE__, __LINE__, "incomplete request message");
                }
            }

            if (_state == StateUpgradeResponsePending)
            {
                if (_incoming)
                {
                    if (_writeBuffer.i < _writeBuffer.b.end())
                    {
                        SocketOperation s = _delegate->write(_writeBuffer);
                        if (s)
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
                    if (_parser->parse(&_readBuffer.b[0], _readI))
                    {
                        handleResponse();
                    }
                    else
                    {
                        throw ProtocolException(__FILE__, __LINE__, "incomplete response message");
                    }
                }
            }
        }
        catch (const WebSocketException& ex)
        {
            throw ProtocolException(__FILE__, __LINE__, ex.reason);
        }

        _state = StateOpened;
        _nextState = StateOpened;

        if (_readI < _readBuffer.i)
        {
            _delegate->getNativeInfo()->ready(SocketOperationRead, true);
        }
    }
    catch (const Ice::LocalException& ex)
    {
        if (_instance->traceLevel() >= 2)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << protocol() << " connection HTTP upgrade request failed\n" << toString() << "\n" << ex;
        }
        throw;
    }

    if (_instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        if (_incoming)
        {
            out << "accepted " << protocol() << " connection HTTP upgrade request\n" << toString();
        }
        else
        {
            out << protocol() << " connection HTTP upgrade request accepted\n" << toString();
        }
    }

    return SocketOperationNone;
}

SocketOperation
IceInternal::WSTransceiver::closing(bool initiator, exception_ptr reason)
{
    if (_instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "gracefully closing " << protocol() << " connection\n" << toString();
    }

    State s = _nextState == StateOpened ? _state : _nextState;

    if (s == StateClosingRequestPending && _closingInitiator)
    {
        //
        // If we initiated a close connection but also received a
        // close connection, we assume we didn't initiated the
        // connection and we send the close frame now. This is to
        // ensure that if both peers close the connection at the same
        // time we don't hang having both peer waiting for the close
        // frame of the other.
        //
        assert(!initiator);
        _closingInitiator = false;
        return SocketOperationWrite;
    }
    else if (s >= StateClosingRequestPending)
    {
        return SocketOperationNone;
    }

    _closingInitiator = initiator;

    try
    {
        rethrow_exception(reason);
    }
    catch (const Ice::CloseConnectionException&)
    {
        _closingReason = CLOSURE_NORMAL;
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        _closingReason = CLOSURE_SHUTDOWN;
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        _closingReason = CLOSURE_SHUTDOWN;
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        _closingReason = CLOSURE_SHUTDOWN;
    }
    catch (const Ice::ProtocolException&)
    {
        _closingReason = CLOSURE_PROTOCOL_ERROR;
    }
    catch (...)
    {
        // no closing reason
    }

    if (_state == StateOpened)
    {
        _state = StateClosingRequestPending;
        return initiator ? SocketOperationRead : SocketOperationWrite;
    }
    else
    {
        _nextState = StateClosingRequestPending;
        return SocketOperationNone;
    }
}

void
IceInternal::WSTransceiver::close()
{
    _delegate->close();
    _state = StateClosed;

    //
    // Clear the buffers now instead of waiting for destruction.
    //
    if (!_writePending)
    {
        _writeBuffer.b.clear();
    }
    if (!_readPending)
    {
        _readBuffer.b.clear();
    }
}

SocketOperation
IceInternal::WSTransceiver::write(Buffer& buf)
{
    if (_writePending)
    {
        return SocketOperationWrite;
    }

    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            return _delegate->write(buf);
        }
        else
        {
            return _delegate->write(_writeBuffer);
        }
    }

    do
    {
        if (preWrite(buf))
        {
            if (_writeBuffer.i < _writeBuffer.b.end())
            {
                SocketOperation s = _delegate->write(_writeBuffer);
                if (s)
                {
                    return s;
                }
            }
            else if (_incoming && !buf.b.empty() && _writeState == WriteStatePayload)
            {
                SocketOperation s = _delegate->write(buf);
                if (s)
                {
                    return s;
                }
            }
        }
    } while (postWrite(buf));

    if (_state == StateClosingResponsePending && !_closingInitiator)
    {
        return SocketOperationRead;
    }
    return SocketOperationNone;
}

SocketOperation
IceInternal::WSTransceiver::read(Buffer& buf)
{
    if (_readPending)
    {
        return SocketOperationRead;
    }

    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            return _delegate->read(buf);
        }
        else
        {
            if (_delegate->read(_readBuffer) == SocketOperationWrite)
            {
                return SocketOperationWrite;
            }
            else
            {
                return SocketOperationNone;
            }
        }
    }

    //
    // If we read the full Ice message, handle it before trying
    // reading anymore data from the WS connection.
    //
    if (buf.i == buf.b.end())
    {
        if (_readI < _readBuffer.i)
        {
            _delegate->getNativeInfo()->ready(SocketOperationRead, true);
        }
        return SocketOperationNone;
    }

    SocketOperation s = SocketOperationNone;
    do
    {
        if (preRead(buf))
        {
            if (_readState == ReadStatePayload)
            {
                //
                // If the payload length is smaller than what remains to be read, we read
                // no more than the payload length. The remaining of the buffer will be
                // sent over in another frame.
                //
                size_t readSz = _readPayloadLength - static_cast<size_t>(buf.i - _readStart); // Already read
                if (static_cast<size_t>(buf.b.end() - buf.i) > readSz)
                {
                    size_t size = buf.b.size();
                    buf.b.resize(static_cast<size_t>(buf.i - buf.b.begin()) + readSz);
                    s = _delegate->read(buf);
                    buf.b.resize(size);
                }
                else
                {
                    s = _delegate->read(buf);
                }
            }
            else
            {
                s = _delegate->read(_readBuffer);
            }

            if (s == SocketOperationWrite)
            {
                postRead(buf);
                return s;
            }
        }
    } while (postRead(buf));

    if (buf.i == buf.b.end())
    {
        if (_readI < _readBuffer.i)
        {
            _delegate->getNativeInfo()->ready(SocketOperationRead, true);
        }
        s = SocketOperationNone;
    }
    else
    {
        _delegate->getNativeInfo()->ready(SocketOperationRead, false);
        s = SocketOperationRead;
    }

    if (((_state == StateClosingRequestPending && !_closingInitiator) ||
         (_state == StateClosingResponsePending && _closingInitiator) || _state == StatePingPending ||
         _state == StatePongPending) &&
        _writeState == WriteStateHeader)
    {
        // We have things to write, ask to be notified when writes are ready.
        s = static_cast<SocketOperation>(s | SocketOperationWrite);
    }
    return s;
}

#if defined(ICE_USE_IOCP)
bool
IceInternal::WSTransceiver::startWrite(Buffer& buf)
{
    _writePending = true;
    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            return _delegate->startWrite(buf);
        }
        else
        {
            return _delegate->startWrite(_writeBuffer);
        }
    }

    if (preWrite(buf))
    {
        if (_writeBuffer.i < _writeBuffer.b.end())
        {
            if (_delegate->startWrite(_writeBuffer))
            {
                return buf.b.size() == _writePayloadLength; // Return true only if we've written the whole buffer.
            }
            return false;
        }
        else
        {
            assert(_incoming);
            return _delegate->startWrite(buf);
        }
    }
    else
    {
        _delegate->getNativeInfo()->completed(IceInternal::SocketOperationWrite);
        return false;
    }
}

void
IceInternal::WSTransceiver::finishWrite(Buffer& buf)
{
    _writePending = false;

    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            _delegate->finishWrite(buf);
        }
        else
        {
            _delegate->finishWrite(_writeBuffer);
        }
        return;
    }

    if (_writeBuffer.i < _writeBuffer.b.end())
    {
        _delegate->finishWrite(_writeBuffer);
    }
    else if (!buf.b.empty() && buf.i != buf.b.end())
    {
        assert(_incoming);
        _delegate->finishWrite(buf);
    }

    if (_state == StateClosed)
    {
        _writeBuffer.b.clear();
        return;
    }

    postWrite(buf);
}

void
IceInternal::WSTransceiver::startRead(Buffer& buf)
{
    _readPending = true;
    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            _delegate->startRead(buf);
        }
        else
        {
            _delegate->startRead(_readBuffer);
        }
        return;
    }

    if (preRead(buf))
    {
        if (_readState == ReadStatePayload)
        {
            //
            // If the payload length is smaller than what remains to be read, we read
            // no more than the payload length. The remaining of the buffer will be
            // sent over in another frame.
            //
            size_t readSz = _readPayloadLength - (buf.i - _readStart);
            if (static_cast<size_t>(buf.b.end() - buf.i) > readSz)
            {
                size_t size = buf.b.size();
                buf.b.resize(buf.i - buf.b.begin() + readSz);
                _delegate->startRead(buf);
                buf.b.resize(size);
            }
            else
            {
                _delegate->startRead(buf);
            }
        }
        else
        {
            _delegate->startRead(_readBuffer);
        }
    }
    else
    {
        _delegate->getNativeInfo()->completed(IceInternal::SocketOperationRead);
    }
}

void
IceInternal::WSTransceiver::finishRead(Buffer& buf)
{
    _readPending = false;
    if (_state < StateOpened)
    {
        if (_state < StateConnected)
        {
            _delegate->finishRead(buf);
        }
        else
        {
            _delegate->finishRead(_readBuffer);
        }
        return;
    }

    if (buf.b.empty() || buf.i == buf.b.end())
    {
        // Nothing to do.
    }
    else if (_readState == ReadStatePayload)
    {
        _delegate->finishRead(buf);
    }
    else
    {
        _delegate->finishRead(_readBuffer);
    }

    if (_state == StateClosed)
    {
        _readBuffer.b.clear();
        return;
    }

    postRead(buf);
}
#endif

string
IceInternal::WSTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::WSTransceiver::toString() const
{
    return _delegate->toString();
}

string
IceInternal::WSTransceiver::toDetailedString() const
{
    return _delegate->toDetailedString();
}

Ice::ConnectionInfoPtr
IceInternal::WSTransceiver::getInfo(bool incoming, string adapterName, string connectionId) const
{
    return make_shared<WSConnectionInfo>(
        _delegate->getInfo(incoming, std::move(adapterName), std::move(connectionId)),
        _parser->getHeaders());
}

void
IceInternal::WSTransceiver::checkSendSize(const Buffer& buf)
{
    _delegate->checkSendSize(buf);
}

void
IceInternal::WSTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

IceInternal::WSTransceiver::WSTransceiver(
    ProtocolInstancePtr instance,
    TransceiverPtr del,
    string host,
    string resource)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _host(std::move(host)),
      _resource(std::move(resource)),
      _incoming(false),
      _state(StateInitializeDelegate),
      _parser(make_shared<HttpParser>()),
      _readState(ReadStateOpcode),
      _readBufferSize(1024),
      _readLastFrame(true),
      _readOpCode(0),
      _readHeaderLength(0),
      _readPayloadLength(0),
      _writeState(WriteStateHeader),
      _writeBufferSize(16 * 1024),
      _readPending(false),
      _writePending(false),
      _closingInitiator(false),
      _closingReason(CLOSURE_NORMAL)
{
    //
    // Use 1KB read and 16KB write buffer sizes. We use 16KB for the
    // write buffer size because all the data needs to be copied to
    // the write buffer for the purpose of masking. A 16KB buffer
    // appears to be a good compromise to reduce the number of socket
    // write calls and not consume too much memory.
    //
}

IceInternal::WSTransceiver::WSTransceiver(ProtocolInstancePtr instance, TransceiverPtr del)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _incoming(true),
      _state(StateInitializeDelegate),
      _parser(make_shared<HttpParser>()),
      _readState(ReadStateOpcode),
      _readBufferSize(1024),
      _readLastFrame(true),
      _readOpCode(0),
      _readHeaderLength(0),
      _readPayloadLength(0),
      _writeState(WriteStateHeader),
      _writeBufferSize(1024),
      _readPending(false),
      _writePending(false),
      _closingInitiator(false),
      _closingReason(CLOSURE_NORMAL)
{
    //
    // Use 1KB read and write buffer sizes.
    //
}

IceInternal::WSTransceiver::~WSTransceiver() = default;

void
IceInternal::WSTransceiver::handleRequest(Buffer& responseBuffer)
{
    string val;

    //
    // HTTP/1.1
    //
    if (_parser->versionMajor() != 1 || _parser->versionMinor() != 1)
    {
        throw WebSocketException("unsupported HTTP version");
    }

    //
    // "An |Upgrade| header field containing the value 'websocket',
    //  treated as an ASCII case-insensitive value."
    //
    if (!_parser->getHeader("Upgrade", val, true))
    {
        throw WebSocketException("missing value for Upgrade field");
    }
    else if (val != "websocket")
    {
        throw WebSocketException("invalid value '" + val + "' for Upgrade field");
    }

    //
    // "A |Connection| header field that includes the token 'Upgrade',
    //  treated as an ASCII case-insensitive value.
    //
    if (!_parser->getHeader("Connection", val, true))
    {
        throw WebSocketException("missing value for Connection field");
    }
    else if (val.find("upgrade") == string::npos)
    {
        throw WebSocketException("invalid value '" + val + "' for Connection field");
    }

    //
    // "A |Sec-WebSocket-Version| header field, with a value of 13."
    //
    if (!_parser->getHeader("Sec-WebSocket-Version", val, false))
    {
        throw WebSocketException("missing value for WebSocket version");
    }
    else if (val != "13")
    {
        throw WebSocketException("unsupported WebSocket version '" + val + "'");
    }

    //
    // "Optionally, a |Sec-WebSocket-Protocol| header field, with a list
    //  of values indicating which protocols the client would like to
    //  speak, ordered by preference."
    //
    bool addProtocol = false;
    if (_parser->getHeader("Sec-WebSocket-Protocol", val, true))
    {
        vector<string> protocols;
        if (!IceInternal::splitString(val, ",", protocols))
        {
            throw WebSocketException("invalid value '" + val + "' for WebSocket protocol");
        }
        for (const auto& protocol : protocols)
        {
            if (IceInternal::trim(protocol) != _iceProtocol)
            {
                throw WebSocketException("unknown value '" + protocol + "' for WebSocket protocol");
            }
            addProtocol = true;
        }
    }

    //
    // "A |Sec-WebSocket-Key| header field with a base64-encoded
    //  value that, when decoded, is 16 bytes in length."
    //
    string key;
    if (!_parser->getHeader("Sec-WebSocket-Key", key, false))
    {
        throw WebSocketException("missing value for WebSocket key");
    }

    vector<byte> decodedKey = Base64::decode(key);
    if (decodedKey.size() != 16)
    {
        throw WebSocketException("invalid value '" + key + "' for WebSocket key");
    }

    //
    // Retain the target resource.
    //
    const_cast<string&>(_resource) = _parser->uri();

    //
    // Compose the response.
    //
    ostringstream out;
    out << "HTTP/1.1 101 Switching Protocols\r\n"
        << "Upgrade: websocket\r\n"
        << "Connection: Upgrade\r\n";
    if (addProtocol)
    {
        out << "Sec-WebSocket-Protocol: " << _iceProtocol << "\r\n";
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
    out << "Sec-WebSocket-Accept: ";
    string input = key + _wsUUID;
    vector<byte> hash;
    sha1(reinterpret_cast<const byte*>(&input[0]), input.size(), hash);
    out << IceInternal::Base64::encode(hash) << "\r\n"
        << "\r\n"; // EOM

    string str = out.str();
    responseBuffer.b.resize(str.size());
    memcpy(&responseBuffer.b[0], str.c_str(), str.size());
    responseBuffer.i = responseBuffer.b.begin();
}

void
IceInternal::WSTransceiver::handleResponse()
{
    string val;

    //
    // HTTP/1.1
    //
    if (_parser->versionMajor() != 1 || _parser->versionMinor() != 1)
    {
        throw WebSocketException("unsupported HTTP version");
    }

    //
    // "If the status code received from the server is not 101, the
    //  client handles the response per HTTP [RFC2616] procedures.  In
    //  particular, the client might perform authentication if it
    //  receives a 401 status code; the server might redirect the client
    //  using a 3xx status code (but clients are not required to follow
    //  them), etc."
    //
    if (_parser->status() != 101)
    {
        ostringstream out;
        out << "unexpected status value " << _parser->status();
        if (!_parser->reason().empty())
        {
            out << ":" << endl << _parser->reason();
        }
        throw WebSocketException(out.str());
    }

    //
    // "If the response lacks an |Upgrade| header field or the |Upgrade|
    //  header field contains a value that is not an ASCII case-
    //  insensitive match for the value "websocket", the client MUST
    //  _Fail the WebSocket Connection_."
    //
    if (!_parser->getHeader("Upgrade", val, true))
    {
        throw WebSocketException("missing value for Upgrade field");
    }
    else if (val != "websocket")
    {
        throw WebSocketException("invalid value '" + val + "' for Upgrade field");
    }

    //
    // "If the response lacks a |Connection| header field or the
    //  |Connection| header field doesn't contain a token that is an
    //  ASCII case-insensitive match for the value "Upgrade", the client
    //  MUST _Fail the WebSocket Connection_."
    //
    if (!_parser->getHeader("Connection", val, true))
    {
        throw WebSocketException("missing value for Connection field");
    }
    else if (val.find("upgrade") == string::npos)
    {
        throw WebSocketException("invalid value '" + val + "' for Connection field");
    }

    //
    // "If the response includes a |Sec-WebSocket-Protocol| header field
    //  and this header field indicates the use of a subprotocol that was
    //  not present in the client's handshake (the server has indicated a
    //  subprotocol not requested by the client), the client MUST _Fail
    //  the WebSocket Connection_."
    //
    if (_parser->getHeader("Sec-WebSocket-Protocol", val, true) && val != _iceProtocol)
    {
        throw WebSocketException("invalid value '" + val + "' for WebSocket protocol");
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
    if (!_parser->getHeader("Sec-WebSocket-Accept", val, false))
    {
        throw WebSocketException("missing value for Sec-WebSocket-Accept");
    }
    string input = _key + _wsUUID;
    vector<byte> hash;
    sha1(reinterpret_cast<const byte*>(&input[0]), input.size(), hash);
    if (val != IceInternal::Base64::encode(hash))
    {
        throw WebSocketException("invalid value '" + val + "' for Sec-WebSocket-Accept");
    }
}

bool
IceInternal::WSTransceiver::preRead(Buffer& buf)
{
    while (true)
    {
        if (_readState == ReadStateOpcode)
        {
            //
            // Is there enough data available to read the opcode?
            //
            if (!readBuffered(2))
            {
                return true;
            }

            //
            // Most-significant bit indicates whether this is the
            // last frame. Least-significant four bits hold the
            // opcode.
            //
            byte ch = *_readI++;
            _readOpCode = static_cast<int>(ch & byte{0xf});

            //
            // Remember if last frame if we're going to read a data or
            // continuation frame, this is only for protocol
            // correctness checking purpose.
            //
            if (_readOpCode == OP_DATA)
            {
                if (!_readLastFrame)
                {
                    throw ProtocolException(__FILE__, __LINE__, "invalid data frame, no FIN on previous frame");
                }
                _readLastFrame = (ch & byte{FLAG_FINAL}) == byte{FLAG_FINAL};
            }
            else if (_readOpCode == OP_CONT)
            {
                if (_readLastFrame)
                {
                    throw ProtocolException(__FILE__, __LINE__, "invalid continuation frame, previous frame FIN set");
                }
                _readLastFrame = (ch & byte{FLAG_FINAL}) == byte{FLAG_FINAL};
            }

            ch = *_readI++;

            //
            // Check the MASK bit. Messages sent by a client must be masked;
            // messages sent by a server must not be masked.
            //
            const bool masked = (ch & byte{FLAG_MASKED}) == byte{FLAG_MASKED};
            if (masked != _incoming)
            {
                throw ProtocolException(__FILE__, __LINE__, "invalid masking");
            }

            //
            // Extract the payload length, which can have the following values:
            //
            // 0-125: The payload length
            // 126:   The subsequent two bytes contain the payload length
            // 127:   The subsequent eight bytes contain the payload length
            //
            _readPayloadLength = static_cast<size_t>((ch & byte{0x7f}));
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
            if (_readHeaderLength > 0 && !readBuffered(_readHeaderLength))
            {
                return true;
            }

            if (_readPayloadLength == 126)
            {
                _readPayloadLength = static_cast<size_t>(ntohs(*reinterpret_cast<uint16_t*>(_readI)));
                _readI += 2;
            }
            else if (_readPayloadLength == 127)
            {
                assert(_readPayloadLength == 127);
                int64_t l = ice_nlltoh(_readI);
                _readI += 8;
                if (l < 0 || l > INT_MAX)
                {
                    ostringstream ostr;
                    ostr << "invalid WebSocket payload length: " << l;
                    throw ProtocolException(__FILE__, __LINE__, ostr.str());
                }
                _readPayloadLength = static_cast<size_t>(l);
            }

            //
            // Read the mask if this is an incoming connection.
            //
            if (_incoming)
            {
                assert(_readBuffer.i - _readI >= 4); // We must have needed to read the mask.
                memcpy(_readMask, _readI, 4);        // Copy the mask.
                _readI += 4;
            }

            switch (_readOpCode)
            {
                case OP_TEXT: // Text frame
                {
                    throw ProtocolException(__FILE__, __LINE__, "text frames not supported");
                }
                case OP_DATA: // Data frame
                case OP_CONT: // Continuation frame
                {
                    if (_instance->traceLevel() >= 2)
                    {
                        Trace out(_instance->logger(), _instance->traceCategory());
                        out << "received " << protocol() << (_readOpCode == OP_DATA ? " data" : " continuation");
                        out << " frame with payload length of " << _readPayloadLength;
                        out << " bytes\n" << toString();
                    }

                    if (_readPayloadLength <= 0)
                    {
                        throw ProtocolException(__FILE__, __LINE__, "payload length is 0");
                    }
                    _readState = ReadStatePayload;
                    assert(buf.i != buf.b.end());
                    _readFrameStart = buf.i;
                    break;
                }
                case OP_CLOSE: // Connection close
                {
                    if (_instance->traceLevel() >= 2)
                    {
                        Trace out(_instance->logger(), _instance->traceCategory());
                        out << "received " << protocol() << " connection close frame\n" << toString();
                    }

                    State s = _nextState == StateOpened ? _state : _nextState;
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
                        throw ConnectionLostException(__FILE__, __LINE__, 0);
                    }
                }
                case OP_PING:
                {
                    if (_instance->traceLevel() >= 2)
                    {
                        Trace out(_instance->logger(), _instance->traceCategory());
                        out << "received " << protocol() << " connection ping frame\n" << toString();
                    }
                    _readState = ReadStateControlFrame;
                    break;
                }
                case OP_PONG: // Pong
                {
                    if (_instance->traceLevel() >= 2)
                    {
                        Trace out(_instance->logger(), _instance->traceCategory());
                        out << "received " << protocol() << " connection pong frame\n" << toString();
                    }
                    _readState = ReadStateControlFrame;
                    break;
                }
                default:
                {
                    ostringstream ostr;
                    ostr << "unsupported opcode: " << _readOpCode;
                    throw ProtocolException(__FILE__, __LINE__, ostr.str());
                }
            }
        }

        if (_readState == ReadStateControlFrame)
        {
            if (_readPayloadLength > 0 && !readBuffered(_readPayloadLength))
            {
                return true;
            }

            if (_readPayloadLength > 0 && _readOpCode == OP_PING)
            {
                _pingPayload.clear();
                _pingPayload.resize(_readPayloadLength);
                memcpy(&_pingPayload[0], _readI, _pingPayload.size());
            }

            _readI += _readPayloadLength;
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
            _readStart = buf.i;

            if (buf.b.empty() || buf.i == buf.b.end())
            {
                return false;
            }

            size_t n = min(static_cast<size_t>(_readBuffer.i - _readI), static_cast<size_t>(buf.b.end() - buf.i));

            if (n > _readPayloadLength)
            {
                n = _readPayloadLength;
            }
            if (n > 0)
            {
                memcpy(buf.i, _readI, n);
                buf.i += n;
                _readI += n;
            }
            //
            // Continue reading if we didn't read the full message, otherwise give back
            // the control to the connection
            //
            return buf.i < buf.b.end() && n < _readPayloadLength;
        }
    }
}

bool
IceInternal::WSTransceiver::postRead(Buffer& buf)
{
    if (_readState != ReadStatePayload)
    {
        return _readStart < _readBuffer.i; // Returns true if data was read.
    }

    if (_readStart == buf.i)
    {
        return false; // Nothing was read or nothing to read.
    }
    assert(_readStart < buf.i);

    if (_incoming)
    {
        //
        // Unmask the data we just read.
        //
        IceInternal::Buffer::Container::iterator p = _readStart;
        for (auto n = static_cast<size_t>(_readStart - _readFrameStart); p < buf.i; ++p, ++n)
        {
            *p ^= _readMask[n % 4];
        }
    }

    _readPayloadLength -= static_cast<size_t>(buf.i - _readStart);
    _readStart = buf.i;
    if (_readPayloadLength == 0)
    {
        //
        // We've read the complete payload, we're ready to read a new frame.
        //
        _readState = ReadStateOpcode;
    }
    return buf.i != buf.b.end();
}

bool
IceInternal::WSTransceiver::preWrite(Buffer& buf)
{
    if (_writeState == WriteStateHeader)
    {
        if (_state == StateOpened)
        {
            if (buf.b.empty() || buf.i == buf.b.end())
            {
                return false;
            }

            assert(buf.i == buf.b.begin());
            prepareWriteHeader(OP_DATA, buf.b.size());

            _writeState = WriteStatePayload;
        }
        else if (_state == StatePingPending)
        {
            prepareWriteHeader(OP_PING, 0); // Don't send any payload

            _writeBuffer.b.resize(static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin()));
            _writeState = WriteStateControlFrame;
            _writeBuffer.i = _writeBuffer.b.begin();
        }
        else if (_state == StatePongPending)
        {
            prepareWriteHeader(OP_PONG, _pingPayload.size());
            if (_pingPayload.size() > static_cast<size_t>(_writeBuffer.b.end() - _writeBuffer.i))
            {
                auto pos = static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin());
                _writeBuffer.b.resize(pos + _pingPayload.size());
                _writeBuffer.i = _writeBuffer.b.begin() + pos;
            }
            memcpy(_writeBuffer.i, &_pingPayload[0], _pingPayload.size());
            _writeBuffer.i += _pingPayload.size();
            _pingPayload.clear();

            _writeBuffer.b.resize(static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin()));
            _writeState = WriteStateControlFrame;
            _writeBuffer.i = _writeBuffer.b.begin();
        }
        else if (
            (_state == StateClosingRequestPending && !_closingInitiator) ||
            (_state == StateClosingResponsePending && _closingInitiator))
        {
            prepareWriteHeader(OP_CLOSE, 2);

            // Write closing reason
            *reinterpret_cast<uint16_t*>(_writeBuffer.i) = htons(static_cast<uint16_t>(_closingReason));
            if (!_incoming)
            {
                *_writeBuffer.i++ ^= _writeMask[0];
                *_writeBuffer.i++ ^= _writeMask[1];
            }
            else
            {
                _writeBuffer.i += 2;
            }

            _writeState = WriteStateControlFrame;
            _writeBuffer.b.resize(static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin()));
            _writeBuffer.i = _writeBuffer.b.begin();
        }
        else
        {
            assert(_state != StateClosed);
            return false; // Nothing to write in this state
        }

        _writePayloadLength = 0;
    }

    if (_writeState == WriteStatePayload)
    {
        //
        // For an outgoing connection, each message must be masked with a random
        // 32-bit value, so we copy the entire message into the internal buffer
        // for writing. For incoming connections, we just copy the start of the
        // message in the internal buffer after the header. If the message is
        // larger, the reminder is sent directly from the message buffer to avoid
        // copying.
        //

        if (!_incoming && (_writePayloadLength == 0 || _writeBuffer.i == _writeBuffer.b.end()))
        {
            if (_writeBuffer.i == _writeBuffer.b.end())
            {
                _writeBuffer.i = _writeBuffer.b.begin();
            }

            auto n = static_cast<size_t>(buf.i - buf.b.begin());
            for (; n < buf.b.size() && _writeBuffer.i < _writeBuffer.b.end(); ++_writeBuffer.i, ++n)
            {
                *_writeBuffer.i = buf.b[n] ^ _writeMask[n % 4];
            }
            _writePayloadLength = n;
            if (_writeBuffer.i < _writeBuffer.b.end())
            {
                _writeBuffer.b.resize(static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin()));
            }
            _writeBuffer.i = _writeBuffer.b.begin();
        }
        else if (_writePayloadLength == 0)
        {
            size_t n =
                min(static_cast<size_t>(_writeBuffer.b.end() - _writeBuffer.i),
                    static_cast<size_t>(buf.b.end() - buf.i));
            memcpy(_writeBuffer.i, buf.i, n);
            _writeBuffer.i += n;
            buf.i += n;
            _writePayloadLength = n;
            if (_writeBuffer.i < _writeBuffer.b.end())
            {
                _writeBuffer.b.resize(static_cast<size_t>(_writeBuffer.i - _writeBuffer.b.begin()));
            }
            _writeBuffer.i = _writeBuffer.b.begin();
        }
        return true;
    }
    else
    {
        return _writeBuffer.i < _writeBuffer.b.end();
    }
}

bool
IceInternal::WSTransceiver::postWrite(Buffer& buf)
{
    if (_state > StateOpened && _writeState == WriteStateControlFrame)
    {
        if (_writeBuffer.i == _writeBuffer.b.end())
        {
            if (_state == StatePingPending)
            {
                if (_instance->traceLevel() >= 2)
                {
                    Trace out(_instance->logger(), _instance->traceCategory());
                    out << "sent " << protocol() << " connection ping frame\n" << toString();
                }
            }
            else if (_state == StatePongPending)
            {
                if (_instance->traceLevel() >= 2)
                {
                    Trace out(_instance->logger(), _instance->traceCategory());
                    out << "sent " << protocol() << " connection pong frame\n" << toString();
                }
            }
            else if (
                (_state == StateClosingRequestPending && !_closingInitiator) ||
                (_state == StateClosingResponsePending && _closingInitiator))
            {
                if (_instance->traceLevel() >= 2)
                {
                    Trace out(_instance->logger(), _instance->traceCategory());
                    out << "sent " << protocol() << " connection close frame\n" << toString();
                }

                if (_state == StateClosingRequestPending && !_closingInitiator)
                {
                    _writeState = WriteStateHeader;
                    _state = StateClosingResponsePending;
                    return false;
                }
                else
                {
                    throw ConnectionLostException(__FILE__, __LINE__, 0);
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
            return true;
        }
    }

    if ((!_incoming || buf.i == buf.b.begin()) && _writePayloadLength > 0)
    {
        if (_writeBuffer.i == _writeBuffer.b.end())
        {
            buf.i = buf.b.begin() + _writePayloadLength;
        }
    }

    if (buf.b.empty() || buf.i == buf.b.end())
    {
        _writeState = WriteStateHeader;
        if (_state == StatePingPending || _state == StatePongPending ||
            (_state == StateClosingRequestPending && !_closingInitiator) ||
            (_state == StateClosingResponsePending && _closingInitiator))
        {
            return true;
        }
    }
    else if (_state == StateOpened)
    {
        return true;
    }
    return false;
}

bool
IceInternal::WSTransceiver::readBuffered(IceInternal::Buffer::Container::size_type sz)
{
    if (_readI == _readBuffer.i)
    {
        _readBuffer.b.resize(_readBufferSize);
        _readI = _readBuffer.i = _readBuffer.b.begin();
    }
    else
    {
        auto available = static_cast<size_t>(_readBuffer.i - _readI);
        if (available < sz)
        {
            if (_readI != &_readBuffer.b[0])
            {
                memmove(&_readBuffer.b[0], _readI, available);
            }
            _readBuffer.b.resize(max(_readBufferSize, sz));
            _readI = _readBuffer.b.begin();
            _readBuffer.i = _readI + available;
        }
    }

    _readStart = _readBuffer.i;
    if (_readI + sz > _readBuffer.i)
    {
        return false; // Not enough read.
    }
    assert(_readBuffer.i > _readI);
    return true;
}

void
IceInternal::WSTransceiver::prepareWriteHeader(uint8_t opCode, IceInternal::Buffer::Container::size_type payloadLength)
{
    //
    // We need to prepare the frame header.
    //
    _writeBuffer.b.resize(_writeBufferSize);
    _writeBuffer.i = _writeBuffer.b.begin();

    //
    // Set the opcode - this is the one and only data frame.
    //
    *_writeBuffer.i++ = static_cast<byte>(opCode | FLAG_FINAL);

    //
    // Set the payload length.
    //
    if (payloadLength <= 125)
    {
        *_writeBuffer.i++ = static_cast<byte>(payloadLength);
    }
    else if (payloadLength > 125 && payloadLength <= USHRT_MAX)
    {
        //
        // Use an extra 16 bits to encode the payload length.
        //
        *_writeBuffer.i++ = byte{126};
        *reinterpret_cast<uint16_t*>(_writeBuffer.i) = htons(static_cast<uint16_t>(payloadLength));
        _writeBuffer.i += 2;
    }
    else if (payloadLength > USHRT_MAX)
    {
        //
        // Use an extra 64 bits to encode the payload length.
        //
        *_writeBuffer.i++ = byte{127};
        ice_htonll(static_cast<int64_t>(payloadLength), _writeBuffer.i);
        _writeBuffer.i += 8;
    }

    if (!_incoming)
    {
        //
        // Add a random 32-bit mask to every outgoing frame, copy the payload data,
        // and apply the mask.
        //
        _writeBuffer.b[1] |= byte{FLAG_MASKED};
        IceInternal::generateRandom(reinterpret_cast<char*>(_writeMask), sizeof(_writeMask));
        memcpy(_writeBuffer.i, _writeMask, sizeof(_writeMask));
        _writeBuffer.i += sizeof(_writeMask);
    }
}
