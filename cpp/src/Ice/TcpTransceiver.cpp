// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpTransceiver.h>
#include <Ice/Connection.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>

#include <IceUtil/DisableWarnings.h>
#include <Ice/Stats.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::TcpTransceiver::getNativeInfo()
{
    return this;
}

#ifdef ICE_USE_IOCP
AsyncInfo*
IceInternal::TcpTransceiver::getAsyncInfo(SocketOperation status)
{
    switch(status)
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
#endif

SocketOperation
IceInternal::TcpTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer, bool& hasMoreData)
{
    try
    {
        if(_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return SocketOperationConnect;
        }
        else if(_state <= StateConnectPending)
        {
#ifdef ICE_USE_IOCP
            doFinishConnectAsync(_fd, _write);
#else
            doFinishConnect(_fd);
#endif

            _desc = fdToString(_fd, _proxy, _addr, true);

            if(_proxy)
            {
                //
                // Prepare the read & write buffers in advance.
                //
                _proxy->beginWriteConnectRequest(_addr, writeBuffer);
                _proxy->beginReadConnectRequestResponse(readBuffer);

#ifdef ICE_USE_IOCP
                //
                // Return SocketOperationWrite to indicate we need to start a write.
                //
                _state = StateProxyConnectRequest; // Send proxy connect request
                return IceInternal::SocketOperationWrite;
#else
                //
                // Write the proxy connection message.
                //
                if(write(writeBuffer))
                {
                    //
                    // Write completed without blocking.
                    //
                    _proxy->endWriteConnectRequest(writeBuffer);

                    //
                    // Try to read the response.
                    //
                    if(read(readBuffer, hasMoreData))
                    {
                        //
                        // Read completed without blocking - fall through.
                        //
                        _proxy->endReadConnectRequestResponse(readBuffer);
                    }
                    else
                    {
                        //
                        // Return SocketOperationRead to indicate we need to complete the read.
                        //
                        _state = StateProxyConnectRequestPending; // Wait for proxy response
                        return SocketOperationRead;
                    }
                }
                else
                {
                    //
                    // Return SocketOperationWrite to indicate we need to complete the write.
                    //
                    _state = StateProxyConnectRequest; // Send proxy connect request
                    return SocketOperationWrite;
                }
#endif
            }

            _state = StateConnected;
        }
        else if(_state == StateProxyConnectRequest)
        {
            //
            // Write completed.
            //
            _proxy->endWriteConnectRequest(writeBuffer);
            _state = StateProxyConnectRequestPending; // Wait for proxy response
            return SocketOperationRead;
        }
        else if(_state == StateProxyConnectRequestPending)
        {
            //
            // Read completed.
            //
            _proxy->endReadConnectRequestResponse(readBuffer);
            _state = StateConnected;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        if(_instance->traceLevel() >= 2)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "failed to establish " << _instance->protocol() << " connection\n"
                << fdToString(_fd, _proxy, _addr, false) << "\n" << ex;
        }
        throw;
    }

    assert(_state == StateConnected);
    if(_instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << _instance->protocol() << " connection established\n" << _desc;
    }
    return SocketOperationNone;
}

SocketOperation
IceInternal::TcpTransceiver::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceInternal::TcpTransceiver::close()
{
    //
    // If the transceiver is not connected, its description is simply "not connected",
    // which isn't very helpful.
    //
    if(_state == StateConnected && _instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "closing " << _instance->protocol() << " connection\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    try
    {
        closeSocket(_fd);
        _fd = INVALID_SOCKET;
    }
    catch(const SocketException&)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

SocketOperation
IceInternal::TcpTransceiver::write(Buffer& buf)
{
    if(buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
#ifdef ICE_USE_IOCP
    //
    // Limit packet size to avoid performance problems on WIN32
    //
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
#endif
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);

        ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&*buf.i), packetSize, 0);
        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(wouldBlock())
            {
                return SocketOperationWrite;
            }

            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "sent " << ret << " of " << packetSize << " bytes via " << _instance->protocol() << '\n'
                << toString();
        }

        if(_instance->stats())
        {
            _instance->stats()->bytesSent(_instance->protocol(), static_cast<Int>(ret));
        }

        buf.i += ret;

        if(packetSize > buf.b.end() - buf.i)
        {
            packetSize = static_cast<int>(buf.b.end() - buf.i);
        }
    }

    return SocketOperationNone;
}

SocketOperation
IceInternal::TcpTransceiver::read(Buffer& buf, bool&)
{
    if(buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }

    //
    // It's impossible for packetSize to be more than an Int.
    //
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    while(buf.i != buf.b.end())
    {
        assert(_fd != INVALID_SOCKET);
        ssize_t ret = ::recv(_fd, reinterpret_cast<char*>(&*buf.i), packetSize, 0);

        if(ret == 0)
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

        if(ret == SOCKET_ERROR)
        {
            if(interrupted())
            {
                continue;
            }

            if(noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if(wouldBlock())
            {
                return SocketOperationRead;
            }

            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }

        if(_instance->traceLevel() >= 3)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "received " << ret << " of " << packetSize << " bytes via " << _instance->protocol() << '\n' 
                << toString();
        }

        if(_instance->stats())
        {
            _instance->stats()->bytesReceived(_instance->protocol(), static_cast<Int>(ret));
        }

        buf.i += ret;

        packetSize = static_cast<int>(buf.b.end() - buf.i);
    }
    return SocketOperationNone;
}

#ifdef ICE_USE_IOCP
bool
IceInternal::TcpTransceiver::startWrite(Buffer& buf)
{
    if(_state == StateConnectPending)
    {
        Address addr = _proxy ? _proxy->getAddress() : _addr;
        doConnectAsync(_fd, addr, _write);
        return false;
    }

    assert(!buf.b.empty());
    assert(buf.i != buf.b.end());

    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
    {
        packetSize = _maxSendPacketSize;
    }
    assert(packetSize > 0);
    _write.buf.len = packetSize;
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, NULL);
    if(err == SOCKET_ERROR)
    {
        if(!wouldBlock())
        {
            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }
    return packetSize == static_cast<int>(buf.b.end() - buf.i);
}

void
IceInternal::TcpTransceiver::finishWrite(Buffer& buf)
{
    if(_state < StateConnected && _state != StateProxyConnectRequest)
    {
        return;
    }

    if(static_cast<int>(_write.count) == SOCKET_ERROR)
    {
        WSASetLastError(_write.error);
        if(connectionLost())
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

    if(_instance->traceLevel() >= 3)
    {
        int packetSize = static_cast<int>(buf.b.end() - buf.i);
        if(_maxSendPacketSize > 0 && packetSize > _maxSendPacketSize)
        {
            packetSize = _maxSendPacketSize;
        }
        Trace out(_instance->logger(), _instance->traceCategory());

        out << "sent " << _write.count << " of " << packetSize << " bytes via " << _instance->protocol() << '\n' 
            << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesSent(_instance->protocol(), _write.count);
    }

    buf.i += _write.count;
}

void
IceInternal::TcpTransceiver::startRead(Buffer& buf)
{
    int packetSize = static_cast<int>(buf.b.end() - buf.i);
    if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
    {
        packetSize = _maxReceivePacketSize;
    }
    assert(!buf.b.empty() && buf.i != buf.b.end());

    _read.buf.len = packetSize;
    _read.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, NULL);
    if(err == SOCKET_ERROR)
    {
        if(!wouldBlock())
        {
            if(connectionLost())
            {
                ConnectionLostException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
            else
            {
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }
    }
}

void
IceInternal::TcpTransceiver::finishRead(Buffer& buf)
{
    if(static_cast<int>(_read.count) == SOCKET_ERROR)
    {
        WSASetLastError(_read.error);
        if(connectionLost())
        {
            ConnectionLostException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }
    else if(_read.count == 0)
    {
        ConnectionLostException ex(__FILE__, __LINE__);
        ex.error = 0;
        throw ex;
    }

    if(_instance->traceLevel() >= 3)
    {
        int packetSize = static_cast<int>(buf.b.end() - buf.i);
        if(_maxReceivePacketSize > 0 && packetSize > _maxReceivePacketSize)
        {
            packetSize = _maxReceivePacketSize;
        }
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "received " << _read.count << " of " << packetSize << " bytes via " << _instance->protocol() << '\n' 
            << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesReceived(_instance->protocol(), static_cast<Int>(_read.count));
    }

    buf.i += _read.count;
}
#endif

string
IceInternal::TcpTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::TcpTransceiver::toString() const
{
    return _desc;
}

Ice::ConnectionInfoPtr
IceInternal::TcpTransceiver::getInfo() const
{
    Ice::TCPConnectionInfoPtr info = new Ice::TCPConnectionInfo();
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    return info;
}

void
IceInternal::TcpTransceiver::checkSendSize(const Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, buf.b.size(), messageSizeMax);
    }
}

IceInternal::TcpTransceiver::TcpTransceiver(const ProtocolInstancePtr& instance, SOCKET fd, 
                                            const NetworkProxyPtr& proxy, const Address& addr) :
    NativeInfo(fd),
    _instance(instance),
    _proxy(proxy),
    _addr(addr),
    _state(StateNeedConnect)
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead),
    _write(SocketOperationWrite)
#endif
{
    setBlock(_fd, false);

    setTcpBufSize(_fd, _instance->properties(), _instance->logger());

#ifdef ICE_USE_IOCP
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxSendPacketSize = IceInternal::getSendBufferSize(fd) / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = IceInternal::getRecvBufferSize(fd);
    if(_maxReceivePacketSize < 512)
    {
        _maxReceivePacketSize = 0;
    }
#endif
}

IceInternal::TcpTransceiver::TcpTransceiver(const ProtocolInstancePtr& instance, SOCKET fd) :
    NativeInfo(fd),
    _instance(instance),
    _addr(Address()),
    _state(StateConnected),
    _desc(fdToString(_fd))
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead),
    _write(SocketOperationWrite)
#endif
{
    setBlock(_fd, false);

    setTcpBufSize(_fd, _instance->properties(), _instance->logger());

#ifdef ICE_USE_IOCP
    //
    // On Windows, limiting the buffer size is important to prevent
    // poor throughput performances when transfering large amount of
    // data. See Microsoft KB article KB823764.
    //
    _maxSendPacketSize = IceInternal::getSendBufferSize(fd) / 2;
    if(_maxSendPacketSize < 512)
    {
        _maxSendPacketSize = 0;
    }

    _maxReceivePacketSize = IceInternal::getRecvBufferSize(fd);
    if(_maxReceivePacketSize < 512)
    {
        _maxReceivePacketSize = 0;
    }
#endif
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceInternal::TcpTransceiver::connect()
{
#ifndef ICE_USE_IOCP
    try
    {
        Address addr = _proxy ? _proxy->getAddress() : _addr;
        if(doConnect(_fd, addr))
        {
            _state = StateConnected;
            _desc = fdToString(_fd, _proxy, _addr, true);
            if(_instance->traceLevel() >= 1)
            {
                Trace out(_instance->logger(), _instance->traceCategory());
                out << _instance->protocol() << " connection established\n" << _desc;
            }
        }
        else
        {
            _desc = fdToString(_fd, _proxy, _addr, true);
        }
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
#endif
}
