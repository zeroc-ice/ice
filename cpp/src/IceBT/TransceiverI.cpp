// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/TransceiverI.h>
#include <IceBT/ConnectionInfo.h>
#include <IceBT/Engine.h>
#include <IceBT/Instance.h>
#include <IceBT/Util.h>

#include <Ice/Connection.h>
#include <Ice/LocalException.h>

#include <IceUtil/DisableWarnings.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceInternal::NativeInfoPtr
IceBT::TransceiverI::getNativeInfo()
{
    return _stream;
}

IceInternal::SocketOperation
IceBT::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    return _stream->connect(readBuffer, writeBuffer);
}

IceInternal::SocketOperation
IceBT::TransceiverI::closing(bool initiator, const Ice::LocalException&)
{
    //
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    //
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceBT::TransceiverI::close()
{
    _stream->close();
}

IceInternal::SocketOperation
IceBT::TransceiverI::write(IceInternal::Buffer& buf)
{
    return _stream->write(buf);
}

IceInternal::SocketOperation
IceBT::TransceiverI::read(IceInternal::Buffer& buf)
{
    return _stream->read(buf);
}

string
IceBT::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceBT::TransceiverI::toString() const
{
    return _stream->toString();
}

string
IceBT::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceBT::TransceiverI::getInfo() const
{
    IceBT::ConnectionInfoPtr info = ICE_MAKE_SHARED(IceBT::ConnectionInfo);
    fdToAddressAndChannel(_stream->fd(), info->localAddress, info->localChannel, info->remoteAddress,
                          info->remoteChannel);
    if(_stream->fd() != INVALID_SOCKET)
    {
        info->rcvSize = IceInternal::getRecvBufferSize(_stream->fd());
        info->sndSize = IceInternal::getSendBufferSize(_stream->fd());
    }
    info->uuid = _uuid;
    return info;
}

void
IceBT::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceBT::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _stream->setBufferSize(rcvSize, sndSize);
}

IceBT::TransceiverI::TransceiverI(const InstancePtr& instance, const StreamSocketPtr& stream, const string& uuid) :
    _instance(instance),
    _stream(stream),
    _uuid(uuid)
{
}

IceBT::TransceiverI::~TransceiverI()
{
}
