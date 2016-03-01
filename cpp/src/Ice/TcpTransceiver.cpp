// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::TcpTransceiver::getNativeInfo()
{
    return _stream;
}

SocketOperation
IceInternal::TcpTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer, bool&)
{
    return _stream->connect(readBuffer, writeBuffer);
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
    _stream->close();
}

SocketOperation
IceInternal::TcpTransceiver::write(Buffer& buf)
{
    return _stream->write(buf);
}

SocketOperation
IceInternal::TcpTransceiver::read(Buffer& buf, bool&)
{
    return _stream->read(buf);
}

#ifdef ICE_USE_IOCP
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
IceInternal::TcpTransceiver::finishRead(Buffer& buf, bool&)
{
    _stream->finishRead(buf);
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
    return _stream->toString();
}

string
IceInternal::TcpTransceiver::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceInternal::TcpTransceiver::getInfo() const
{
    TCPConnectionInfoPtr info = new TCPConnectionInfo();
    fillConnectionInfo(info);
    return info;
}

Ice::ConnectionInfoPtr
IceInternal::TcpTransceiver::getWSInfo(const Ice::HeaderDict& headers) const
{
    WSConnectionInfoPtr info = new WSConnectionInfo();
    fillConnectionInfo(info);
    info->headers = headers;
    return info;
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

IceInternal::TcpTransceiver::TcpTransceiver(const ProtocolInstancePtr& instance, const StreamSocketPtr& stream) :
    _instance(instance),
    _stream(stream)
{
}

IceInternal::TcpTransceiver::~TcpTransceiver()
{
}

void
IceInternal::TcpTransceiver::fillConnectionInfo(const TCPConnectionInfoPtr& info) const
{
    fdToAddressAndPort(_stream->fd(), info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    if(_stream->fd() != INVALID_SOCKET)
    {
        info->rcvSize = getRecvBufferSize(_stream->fd());
        info->sndSize = getSendBufferSize(_stream->fd());
    }
}
