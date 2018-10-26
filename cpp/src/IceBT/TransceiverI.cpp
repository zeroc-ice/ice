// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

IceUtil::Shared* IceBT::upCast(TransceiverI* p) { return p; }

IceInternal::NativeInfoPtr
IceBT::TransceiverI::getNativeInfo()
{
    return _stream;
}

IceInternal::SocketOperation
IceBT::TransceiverI::initialize(IceInternal::Buffer& /*readBuffer*/, IceInternal::Buffer& /*writeBuffer*/)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);

    if(_exception)
    {
        //
        // Raise the stored exception from a failed connection attempt.
        //
        _exception->ice_throw();
    }
    else if(_needConnect)
    {
        //
        // We need to initiate a connection attempt.
        //
        _needConnect = false;
        _instance->engine()->connect(_addr, _uuid, ICE_MAKE_SHARED(ConnectCallbackI, this));
        return IceInternal::SocketOperationConnect;
    }

    return IceInternal::SocketOperationNone; // Already connected.
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
    if(_connection)
    {
        _connection->close();
    }
    _stream->close();
}

IceInternal::SocketOperation
IceBT::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(_stream->fd() == INVALID_SOCKET)
    {
        //
        // This can happen if the connection failed. We return None here and let initialize() handle the rest.
        //
        return IceInternal::SocketOperationNone;
    }

    return _stream->write(buf);
}

IceInternal::SocketOperation
IceBT::TransceiverI::read(IceInternal::Buffer& buf)
{
    if(_stream->fd() == INVALID_SOCKET)
    {
        //
        // This can happen if we connected successfully but the selector has not updated our FD yet.
        //
        return IceInternal::SocketOperationRead;
    }

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
    _stream->setBufferSize(_stream->fd(), rcvSize, sndSize);
}

IceBT::TransceiverI::TransceiverI(const InstancePtr& instance, const StreamSocketPtr& stream, const ConnectionPtr& conn,
                                  const string& uuid) :
    _instance(instance),
    _stream(stream),
    _connection(conn),
    _uuid(uuid),
    _needConnect(false)
{
}

IceBT::TransceiverI::TransceiverI(const InstancePtr& instance, const string& addr, const string& uuid) :
    _instance(instance),
    _stream(new StreamSocket(instance, INVALID_SOCKET)),
    _addr(addr),
    _uuid(uuid),
    _needConnect(true)
{
}

IceBT::TransceiverI::~TransceiverI()
{
}

void
IceBT::TransceiverI::connectCompleted(int fd, const ConnectionPtr& conn)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
    _connection = conn;
    _stream->setFd(fd);
    //
    // Triggers a call to write() from a different thread.
    //
    _stream->ready(IceInternal::SocketOperationConnect, true);
}

void
IceBT::TransceiverI::connectFailed(const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_lock);
    //
    // Save the exception - it will be raised in initialize().
    //
    ICE_SET_EXCEPTION_FROM_CLONE(_exception, ex.ice_clone());
    //
    // Triggers a call to write() from a different thread.
    //
    _stream->ready(IceInternal::SocketOperationConnect, true);
}
