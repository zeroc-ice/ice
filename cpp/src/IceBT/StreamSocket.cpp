// Copyright (c) ZeroC, Inc.

#include "StreamSocket.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "IceBT/EndpointInfo.h"
#include "Instance.h"
#include "Util.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

IceBT::StreamSocket::StreamSocket(InstancePtr instance, SOCKET fd)
    : IceInternal::NativeInfo(fd),
      _instance(std::move(instance))
{
    if (fd != INVALID_SOCKET)
    {
        init(fd);
    }
    _desc = fdToString(fd);
}

IceBT::StreamSocket::~StreamSocket() { assert(_fd == INVALID_SOCKET); }

void
IceBT::StreamSocket::setBufferSize(SOCKET fd, int rcvSize, int sndSize)
{
    assert(fd != INVALID_SOCKET);

    if (rcvSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        IceInternal::setRecvBufferSize(fd, rcvSize);
        int size = IceInternal::getRecvBufferSize(fd);
        if (size > 0 && size < rcvSize)
        {
            //
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            //
            IceInternal::BufSizeWarnInfo winfo = _instance->getBufSizeWarn(BTEndpointType);
            if (!winfo.rcvWarn || rcvSize != winfo.rcvSize)
            {
                Ice::Warning out(_instance->logger());
                out << "BT receive buffer size: requested size of " << rcvSize << " adjusted to " << size;
                _instance->setRcvBufSizeWarn(BTEndpointType, rcvSize);
            }
        }
    }

    if (sndSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        IceInternal::setSendBufferSize(fd, sndSize);
        int size = IceInternal::getSendBufferSize(fd);
        if (size > 0 && size < sndSize)
        {
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            IceInternal::BufSizeWarnInfo winfo = _instance->getBufSizeWarn(BTEndpointType);
            if (!winfo.sndWarn || sndSize != winfo.sndSize)
            {
                Ice::Warning out(_instance->logger());
                out << "BT send buffer size: requested size of " << sndSize << " adjusted to " << size;
                _instance->setSndBufSizeWarn(BTEndpointType, sndSize);
            }
        }
    }
}

IceInternal::SocketOperation
IceBT::StreamSocket::read(IceInternal::Buffer& buf)
{
    buf.i += read(reinterpret_cast<char*>(&*buf.i), buf.b.end() - buf.i);
    return buf.i != buf.b.end() ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceBT::StreamSocket::write(IceInternal::Buffer& buf)
{
    buf.i += write(reinterpret_cast<const char*>(&*buf.i), buf.b.end() - buf.i);
    return buf.i != buf.b.end() ? IceInternal::SocketOperationWrite : IceInternal::SocketOperationNone;
}

ssize_t
IceBT::StreamSocket::read(char* buf, size_t length)
{
    assert(_fd != INVALID_SOCKET);

    size_t packetSize = length;
    ssize_t read = 0;

    while (length > 0)
    {
        ssize_t ret = ::recv(_fd, buf, packetSize, 0);
        if (ret == 0)
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__);
        }
        else if (ret == SOCKET_ERROR)
        {
            if (IceInternal::interrupted())
            {
                continue;
            }

            if (IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if (IceInternal::wouldBlock())
            {
                return read;
            }

            if (IceInternal::connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        buf += ret;
        read += ret;
        length -= ret;

        if (packetSize > length)
        {
            packetSize = length;
        }
    }
    return read;
}

ssize_t
IceBT::StreamSocket::write(const char* buf, size_t length)
{
    assert(_fd != INVALID_SOCKET);

    size_t packetSize = length;

    ssize_t sent = 0;
    while (length > 0)
    {
        ssize_t ret = ::send(_fd, buf, packetSize, 0);
        if (ret == 0)
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__);
        }
        else if (ret == SOCKET_ERROR)
        {
            if (IceInternal::interrupted())
            {
                continue;
            }

            if (IceInternal::noBuffers() && packetSize > 1024)
            {
                packetSize /= 2;
                continue;
            }

            if (IceInternal::wouldBlock())
            {
                return sent;
            }

            if (IceInternal::connectionLost())
            {
                throw Ice::ConnectionLostException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
            else
            {
                throw Ice::SocketException(__FILE__, __LINE__, IceInternal::getSocketErrno());
            }
        }

        buf += ret;
        sent += ret;
        length -= ret;

        if (packetSize > length)
        {
            packetSize = length;
        }
    }
    return sent;
}

void
IceBT::StreamSocket::close()
{
    if (_fd != INVALID_SOCKET)
    {
        try
        {
            IceInternal::closeSocket(_fd);
            _fd = INVALID_SOCKET;
        }
        catch (const Ice::SocketException&)
        {
            _fd = INVALID_SOCKET;
            throw;
        }
    }
}

const string&
IceBT::StreamSocket::toString() const
{
    return _desc;
}

void
IceBT::StreamSocket::setFd(SOCKET fd)
{
    assert(fd != INVALID_SOCKET);
    init(fd);
    setNewFd(fd);
    _desc = fdToString(fd);
}

void
IceBT::StreamSocket::init(SOCKET fd)
{
    IceInternal::setBlock(fd, false);

    int32_t rcvSize = _instance->properties()->getIcePropertyAsInt("IceBT.RcvSize");
    int32_t sndSize = _instance->properties()->getIcePropertyAsInt("IceBT.SndSize");

    setBufferSize(fd, rcvSize, sndSize);
}
