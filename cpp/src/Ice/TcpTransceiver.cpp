// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0

#    include "Ice/Buffer.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "ProtocolInstance.h"
#    include "TcpTransceiver.h"

#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::TcpTransceiver::getNativeInfo()
{
    return _stream;
}

SocketOperation
IceInternal::TcpTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    return _stream->connect(readBuffer, writeBuffer);
}

SocketOperation
IceInternal::TcpTransceiver::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? SocketOperationRead : SocketOperationNone;
}

void
IceInternal::TcpTransceiver::close()
{
    _stream->close();
}

SocketOperation
IceInternal::TcpTransceiver::write(Buffer& buf)
{
    return _stream->write(buf);
}

SocketOperation
IceInternal::TcpTransceiver::read(Buffer& buf)
{
    return _stream->read(buf);
}

#    if defined(ICE_USE_IOCP)
bool
IceInternal::TcpTransceiver::startWrite(Buffer& buf)
{
    return _stream->startWrite(buf);
}

void
IceInternal::TcpTransceiver::finishWrite(Buffer& buf)
{
    _stream->finishWrite(buf);
}

void
IceInternal::TcpTransceiver::startRead(Buffer& buf)
{
    _stream->startRead(buf);
}

void
IceInternal::TcpTransceiver::finishRead(Buffer& buf)
{
    _stream->finishRead(buf);
}
#    endif

string
IceInternal::TcpTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::TcpTransceiver::toString() const
{
    return _stream->toString();
}

string
IceInternal::TcpTransceiver::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceInternal::TcpTransceiver::getInfo(bool incoming, string adapterName, string connectionId) const
{
    if (_stream->fd() == INVALID_SOCKET)
    {
        return make_shared<TCPConnectionInfo>(incoming, std::move(adapterName), std::move(connectionId));
    }
    else
    {
        string localAddress;
        int localPort;
        string remoteAddress;
        int remotePort;
        fdToAddressAndPort(_stream->fd(), localAddress, localPort, remoteAddress, remotePort);
        return make_shared<TCPConnectionInfo>(
            incoming,
            std::move(adapterName),
            std::move(connectionId),
            std::move(localAddress),
            localPort,
            std::move(remoteAddress),
            remotePort,
            getRecvBufferSize(_stream->fd()),
            getSendBufferSize(_stream->fd()));
    }
}

void
IceInternal::TcpTransceiver::checkSendSize(const Buffer&)
{
}

void
IceInternal::TcpTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    _stream->setBufferSize(rcvSize, sndSize);
}

IceInternal::TcpTransceiver::TcpTransceiver(ProtocolInstancePtr instance, StreamSocketPtr stream)
    : _instance(std::move(instance)),
      _stream(std::move(stream))
{
}

IceInternal::TcpTransceiver::~TcpTransceiver() = default;
#endif
