// Copyright (c) ZeroC, Inc.

#include "TransceiverI.h"
#include "Engine.h"
#include "Ice/Connection.h"
#include "Ice/LocalExceptions.h"
#include "IceBT/ConnectionInfo.h"
#include "Instance.h"
#include "Util.h"

#include "../Ice/DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

IceInternal::NativeInfoPtr
IceBT::TransceiverI::getNativeInfo()
{
    return _stream;
}

IceInternal::SocketOperation
IceBT::TransceiverI::initialize(IceInternal::Buffer& /*readBuffer*/, IceInternal::Buffer& /*writeBuffer*/)
{
    lock_guard lock(_mutex);

    if (_exception)
    {
        //
        // Raise the stored exception from a failed connection attempt.
        //
        rethrow_exception(_exception);
    }
    else if (_needConnect)
    {
        //
        // We need to initiate a connection attempt.
        //
        _needConnect = false;
        _instance->engine()->connect(_addr, _uuid, make_shared<ConnectCallbackI>(shared_from_this()));
        return IceInternal::SocketOperationConnect;
    }

    return IceInternal::SocketOperationNone; // Already connected.
}

IceInternal::SocketOperation
IceBT::TransceiverI::closing(bool initiator, exception_ptr)
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
    ConnectionPtr connection;
    {
        lock_guard lock(_mutex);
        //
        // Flag the transceiver as closed so a connection completion that races this call
        // disposes of the late-delivered fd and connection instead of installing them.
        //
        _closed = true;
        connection = std::move(_connection);
        _stream->close();
    }

    //
    // Close the connection without holding _mutex: ConnectionI::close() blocks (it makes a
    // DisconnectProfile D-Bus call and joins the connection's I/O thread).
    //
    if (connection)
    {
        connection->close();
    }
}

IceInternal::SocketOperation
IceBT::TransceiverI::write(IceInternal::Buffer& buf)
{
    if (_stream->fd() == INVALID_SOCKET)
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
    if (_stream->fd() == INVALID_SOCKET)
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
IceBT::TransceiverI::getInfo(bool incoming, string adapterName, string connectionId) const
{
    if (_stream->fd() == INVALID_SOCKET)
    {
        return make_shared<ConnectionInfo>(incoming, std::move(adapterName), std::move(connectionId));
    }
    else
    {
        string localAddress;
        int localChannel;
        string remoteAddress;
        int remoteChannel;
        fdToAddressAndChannel(_stream->fd(), localAddress, localChannel, remoteAddress, remoteChannel);

        return make_shared<ConnectionInfo>(
            incoming,
            std::move(adapterName),
            std::move(connectionId),
            std::move(localAddress),
            localChannel,
            std::move(remoteAddress),
            remoteChannel,
            _uuid,
            IceInternal::getRecvBufferSize(_stream->fd()),
            IceInternal::getSendBufferSize(_stream->fd()));
    }
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

IceBT::TransceiverI::TransceiverI(InstancePtr instance, StreamSocketPtr stream, ConnectionPtr conn, string uuid)
    : _instance(std::move(instance)),
      _stream(std::move(stream)),
      _connection(std::move(conn)),
      _uuid(std::move(uuid)),
      _needConnect(false)
{
}

IceBT::TransceiverI::TransceiverI(InstancePtr instance, string addr, string uuid)
    : _instance(std::move(instance)),
      _stream(new StreamSocket(_instance, INVALID_SOCKET)),
      _addr(std::move(addr)),
      _uuid(std::move(uuid)),
      _needConnect(true)
{
}

IceBT::TransceiverI::~TransceiverI() = default;

void
IceBT::TransceiverI::connectCompleted(int fd, const ConnectionPtr& conn)
{
    {
        lock_guard lock(_mutex);
        if (!_closed)
        {
            _connection = conn;
            _stream->setFd(fd);
            //
            // Triggers a call to write() from a different thread.
            //
            _stream->ready(IceInternal::SocketOperationConnect, true);
            return;
        }
    }

    // The transceiver was already closed before the connection completed. We must not install the fd into the abandoned
    // stream (nothing would ever close it), so close the socket here instead. We deliberately don't close 'conn': we
    // run on the connection's own I/O thread, where a blocking close would deadlock. This leaks the connection's I/O
    // thread and system-bus connection, but only on the rare race between an aborted outgoing connection and its
    // completion.
    IceInternal::closeSocketNoThrow(fd);
}

void
IceBT::TransceiverI::connectFailed(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    //
    // Save the exception - it will be raised in initialize().
    //
    _exception = ex;
    //
    // Triggers a call to write() from a different thread.
    //
    _stream->ready(IceInternal::SocketOperationConnect, true);
}
