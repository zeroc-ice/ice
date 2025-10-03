// Copyright (c) ZeroC, Inc.

#include "StreamSocket.h"
#include "NetworkProxy.h"
#include "ProtocolInstance.h"

using namespace IceInternal;

StreamSocket::StreamSocket(
    ProtocolInstancePtr instance,
    const NetworkProxyPtr& proxy,
    const Address& addr,
    const Address& sourceAddr)
    : NativeInfo(createSocket(false, proxy ? proxy->getAddress() : addr)),
      _instance(std::move(instance)),
      _proxy(proxy),
      _addr(addr),
      _sourceAddr(sourceAddr),
      _state(StateNeedConnect)
#if defined(ICE_USE_IOCP)
      ,
      _read(SocketOperationRead),
      _write(SocketOperationWrite)
#endif
{
    init();
#if !defined(ICE_USE_IOCP)
    if (doConnect(_fd, _proxy ? _proxy->getAddress() : _addr, sourceAddr))
    {
        _state = _proxy ? StateProxyWrite : StateConnected;
    }
#endif
    try
    {
        _desc = fdToString(_fd, _proxy, _addr);
    }
    catch (const Ice::Exception&)
    {
        closeSocketNoThrow(_fd);
        throw;
    }
}

StreamSocket::StreamSocket(ProtocolInstancePtr instance, SOCKET fd)
    : NativeInfo(fd),
      _instance(std::move(instance)),
      _addr(),
      _sourceAddr(),
      _state(StateConnected)
#if defined(ICE_USE_IOCP)
      ,
      _read(SocketOperationRead),
      _write(SocketOperationWrite)
#endif
{
    init();
    try
    {
        _desc = fdToString(fd);
    }
    catch (const Ice::Exception&)
    {
        closeSocketNoThrow(fd);
        throw;
    }
}

StreamSocket::~StreamSocket() { assert(_fd == INVALID_SOCKET); }

SocketOperation
StreamSocket::connect(Buffer& readBuffer, Buffer& writeBuffer)
{
    if (_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return SocketOperationConnect;
    }
    else if (_state <= StateConnectPending)
    {
#if defined(ICE_USE_IOCP)
        doFinishConnectAsync(_fd, _write);
#else
        doFinishConnect(_fd);
#endif
        _desc = fdToString(_fd, _proxy, _addr);
        _state = _proxy ? StateProxyWrite : StateConnected;
    }

    if (_state == StateProxyWrite)
    {
        _proxy->beginWrite(_addr, writeBuffer);
        return IceInternal::SocketOperationWrite;
    }
    else if (_state == StateProxyRead)
    {
        _proxy->beginRead(readBuffer);
        return IceInternal::SocketOperationRead;
    }
    else if (_state == StateProxyConnected)
    {
        _proxy->finish(readBuffer, writeBuffer);

        readBuffer.b.clear();
        readBuffer.i = readBuffer.b.end();

        writeBuffer.b.clear();
        writeBuffer.i = writeBuffer.b.end();

        _state = StateConnected;
    }

    assert(_state == StateConnected);
    return IceInternal::SocketOperationNone;
}

bool
StreamSocket::isConnected()
{
    return _state == StateConnected && _fd != INVALID_SOCKET;
}

void
StreamSocket::setBufferSize(int rcvSize, int sndSize)
{
    setTcpBufSize(_fd, rcvSize, sndSize, _instance);
}

SocketOperation
StreamSocket::read(Buffer& buf)
{
    if (_state == StateProxyRead)
    {
        while (true)
        {
            ssize_t ret = read(reinterpret_cast<char*>(&*buf.i), static_cast<size_t>(buf.b.end() - buf.i));
            if (ret == 0)
            {
                return SocketOperationRead;
            }
            buf.i += ret;
            _state = toState(_proxy->endRead(buf));
            if (_state != StateProxyRead)
            {
                return SocketOperationNone;
            }
        }
    }
    buf.i += read(reinterpret_cast<char*>(&*buf.i), static_cast<size_t>(buf.b.end() - buf.i));
    return buf.i != buf.b.end() ? SocketOperationRead : SocketOperationNone;
}

SocketOperation
StreamSocket::write(Buffer& buf)
{
    if (_state == StateProxyWrite)
    {
        while (true)
        {
            ssize_t ret = write(reinterpret_cast<const char*>(&*buf.i), static_cast<size_t>(buf.b.end() - buf.i));
            if (ret == 0)
            {
                return SocketOperationWrite;
            }
            buf.i += ret;
            _state = toState(_proxy->endWrite(buf));
            if (_state != StateProxyWrite)
            {
                return SocketOperationNone;
            }
        }
    }
    buf.i += write(reinterpret_cast<const char*>(&*buf.i), static_cast<size_t>(buf.b.end() - buf.i));
    return buf.i != buf.b.end() ? SocketOperationWrite : SocketOperationNone;
}

ssize_t
StreamSocket::read(char* buf, size_t length)
{
    assert(_fd != INVALID_SOCKET);

    size_t packetSize = length;
    ssize_t read = 0;

    while (length > 0)
    {
#ifdef _WIN32
        ssize_t ret = ::recv(_fd, buf, static_cast<int>(packetSize), 0);
#else
        ssize_t ret = ::recv(_fd, buf, packetSize, 0);
#endif
        if (ret == 0)
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if (ret == SOCKET_ERROR)
        {
            if (interrupted())
            {
                continue;
            }

            if (noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if (wouldBlock())
            {
                return read;
            }

            if (connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }

        buf += ret;
        read += ret;
        length -= static_cast<size_t>(ret);

        if (packetSize > length)
        {
            packetSize = length;
        }
    }
    return read;
}

ssize_t
StreamSocket::write(const char* buf, size_t length)
{
    assert(_fd != INVALID_SOCKET);

    size_t packetSize = length;
    ssize_t sent = 0;
    while (length > 0)
    {
#ifdef _WIN32
        ssize_t ret = ::send(_fd, buf, static_cast<int>(packetSize), 0);
#else
        ssize_t ret = ::send(_fd, buf, packetSize, 0);
#endif
        if (ret == 0)
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if (ret == SOCKET_ERROR)
        {
            if (interrupted())
            {
                continue;
            }

            if (noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if (wouldBlock())
            {
                return sent;
            }

            if (connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }

        buf += ret;
        sent += ret;
        length -= static_cast<size_t>(ret);

        if (packetSize > length)
        {
            packetSize = length;
        }
    }
    return sent;
}

#if defined(ICE_USE_IOCP)
AsyncInfo*
StreamSocket::getAsyncInfo(SocketOperation op)
{
    switch (op)
    {
        case SocketOperationRead:
            return &_read;
        case SocketOperationWrite:
            return &_write;
        default:
            assert(false);
            return 0;
    }
}

bool
StreamSocket::startWrite(Buffer& buf)
{
    if (_state == StateConnectPending)
    {
        doConnectAsync(_fd, _proxy ? _proxy->getAddress() : _addr, _sourceAddr, _write);
        return false;
    }

    size_t length = buf.b.end() - buf.i;
    assert(length > 0);

    _write.buf.len = static_cast<DWORD>(length);
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    _write.error = ERROR_SUCCESS;
    int err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, nullptr);
    if (err == SOCKET_ERROR)
    {
        if (!wouldBlock())
        {
            if (connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
    return true;
}

void
StreamSocket::finishWrite(Buffer& buf)
{
    if (_fd == INVALID_SOCKET || (_state < StateConnected && _state != StateProxyWrite))
    {
        return;
    }

    if (_write.error != ERROR_SUCCESS)
    {
        WSASetLastError(_write.error);
        if (connectionLost())
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    buf.i += _write.count;
    if (_state == StateProxyWrite)
    {
        _state = toState(_proxy->endWrite(buf));
    }
}

void
StreamSocket::startRead(Buffer& buf)
{
    size_t length = buf.b.end() - buf.i;
    assert(length > 0);

    _read.buf.len = static_cast<DWORD>(length);
    _read.buf.buf = reinterpret_cast<char*>(&*buf.i);
    _read.error = ERROR_SUCCESS;
    int err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, nullptr);
    if (err == SOCKET_ERROR)
    {
        if (!wouldBlock())
        {
            if (connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
}

void
StreamSocket::finishRead(Buffer& buf)
{
    if (_fd == INVALID_SOCKET)
    {
        return;
    }

    if (_read.error != ERROR_SUCCESS)
    {
        WSASetLastError(_read.error);
        if (connectionLost())
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
    else if (_read.count == 0)
    {
        throw Ice::ConnectionLostException(__FILE__, __LINE__, 0);
    }

    buf.i += _read.count;

    if (_state == StateProxyRead)
    {
        _state = toState(_proxy->endRead(buf));
    }
}

#endif

void
StreamSocket::close()
{
    assert(_fd != INVALID_SOCKET);
    try
    {
        closeSocket(_fd);
        _fd = INVALID_SOCKET;
    }
    catch (const Ice::SocketException&)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

const std::string&
StreamSocket::toString() const
{
    return _desc;
}

void
StreamSocket::init()
{
    setBlock(_fd, false);
    setTcpBufSize(_fd, _instance);
}

StreamSocket::State
StreamSocket::toState(SocketOperation operation) const
{
    switch (operation)
    {
        case SocketOperationRead:
            return StateProxyRead;
        case SocketOperationWrite:
            return StateProxyWrite;
        default:
            return StateProxyConnected;
    }
}
