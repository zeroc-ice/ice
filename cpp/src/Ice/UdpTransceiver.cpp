// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// The following is required for the Vista PSDK to bring in
// the definitions of the IN6_IS_ADDR_* macros.
//
#if defined(_WIN32) && !defined(_WIN32_WINNT)
#       define _WIN32_WINNT 0x0501
#endif

#include <Ice/UdpTransceiver.h>
#include <Ice/Connection.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::UdpTransceiver::getNativeInfo()
{
    return this;
}


#if defined(ICE_USE_IOCP)
AsyncInfo*
IceInternal::UdpTransceiver::getAsyncInfo(SocketOperation status)
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
IceInternal::UdpTransceiver::initialize()
{
    if(!_incoming)
    {
        if(_connect)
        {
            //
            // If we're not connected yet, return SocketOperationConnect. The transceiver will be 
            // connected once initialize is called again.
            //
            _connect = false;
            return SocketOperationConnect;
        }
        else
        {
            if(_traceLevels->network >= 1)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "starting to send udp packets\n" << toString();
            }
            return SocketOperationNone;
        }
    }
    return SocketOperationNone;
}

void
IceInternal::UdpTransceiver::close()
{
    if(!_connect && _traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "closing udp connection\n" << toString();
    }

    assert(_fd != INVALID_SOCKET);
    closeSocket(_fd);
    _fd = INVALID_SOCKET;
}

bool
IceInternal::UdpTransceiver::write(Buffer& buf)
{
    assert(buf.i == buf.b.begin());
    //
    // The maximum packetSize is either the maximum allowable UDP
    // packet size, or the UDP send buffer size (which ever is
    // smaller).
    //
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
        //
        // We don't log a warning here because the client gets an exception anyway.
        //
        throw DatagramLimitException(__FILE__, __LINE__);
    }

repeat:
    assert(!_connect);
    assert(_fd != INVALID_SOCKET);
#ifdef _WIN32
    ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0);
#else
    ssize_t ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0);
#endif

    if(ret == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeat;
        }

        if(wouldBlock())
        {
            return false;
        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(_traceLevels->network >= 3)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "sent " << ret << " bytes via udp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesSent(type(), static_cast<Int>(ret));
    }

    assert(ret == static_cast<ssize_t>(buf.b.size()));
    buf.i = buf.b.end();
    return true;
}

bool
IceInternal::UdpTransceiver::read(Buffer& buf)
{
    assert(buf.i == buf.b.begin());

    //
    // The maximum packetSize is either the maximum allowable UDP
    // packet size, or the UDP send buffer size (which ever is
    // smaller).
    //
    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
        //
        // We log a warning here because this is the server side -- without the
        // the warning, there would only be silence.
        //
        if(_warn)
        {
            Warning out(_logger);
            out << "DatagramLimitException: maximum size of " << packetSize << " exceeded";
        }
        throw DatagramLimitException(__FILE__, __LINE__);
    }
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:

    ssize_t ret;
    if(_connect)
    {
        //
        // If we must connect, then we connect to the first peer that
        // sends us a packet.
        //
        struct sockaddr_storage peerAddr;
        memset(&peerAddr, 0, sizeof(struct sockaddr_storage));
        socklen_t len = static_cast<socklen_t>(sizeof(peerAddr));
        assert(_fd != INVALID_SOCKET);
        ret = recvfrom(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize,
                       0, reinterpret_cast<struct sockaddr*>(&peerAddr), &len);
        if(ret != SOCKET_ERROR)
        {
#ifndef NDEBUG
            bool connected = doConnect(_fd, peerAddr);
            assert(connected);
#else
            doConnect(_fd, peerAddr);
#endif
            _connect = false; // We are connected now.

            if(_traceLevels->network >= 1)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "connected udp socket\n" << toString();
            }
        }
    }
    else
    {
        assert(_fd != INVALID_SOCKET);
        ret = ::recv(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0);
    }

    if(ret == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeat;
        }

        if(wouldBlock())
        {
            return false;
        }

        if(recvTruncated())
        {
            DatagramLimitException ex(__FILE__, __LINE__);
            if(_warn)
            {
                Warning out(_logger);
                out << "DatagramLimitException: maximum size of " << packetSize << " exceeded";
            }
            throw ex;

        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(_traceLevels->network >= 3)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "received " << ret << " bytes via udp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesReceived(type(), static_cast<Int>(ret));
    }

    buf.b.resize(ret);
    buf.i = buf.b.end();
    return true;
}

#ifdef ICE_USE_IOCP
void
IceInternal::UdpTransceiver::startWrite(Buffer& buf)
{
    assert(buf.i == buf.b.begin());

    //
    // The maximum packetSize is either the maximum allowable UDP
    // packet size, or the UDP send buffer size (which ever is
    // smaller).
    //
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
        //
        // We don't log a warning here because the client gets an exception anyway.
        //
        throw DatagramLimitException(__FILE__, __LINE__);
    }

    assert(!_connect);
    assert(_fd != INVALID_SOCKET);

    _write.buf.len = static_cast<int>(buf.b.size());
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
}

void
IceInternal::UdpTransceiver::finishWrite(Buffer& buf)
{
    if(_write.count == SOCKET_ERROR)
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

    if(_traceLevels->network >= 3)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "sent " << _write.count << " bytes via udp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesSent(type(), static_cast<Int>(_write.count));
    }

    assert(_write.count == static_cast<ssize_t>(buf.b.size()));
    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::startRead(Buffer& buf)
{
    //
    // The maximum packetSize is either the maximum allowable UDP
    // packet size, or the UDP send buffer size (which ever is
    // smaller).
    //
    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
        //
        // We log a warning here because this is the server side -- without the
        // the warning, there would only be silence.
        //
        if(_warn)
        {
            Warning out(_logger);
            out << "DatagramLimitException: maximum size of " << packetSize << " exceeded";
        }
        throw DatagramLimitException(__FILE__, __LINE__);
    }
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

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
IceInternal::UdpTransceiver::finishRead(Buffer& buf)
{
    if(_read.count == SOCKET_ERROR)
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

    if(_traceLevels->network >= 3)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "received " << _read.count << " bytes via udp\n" << toString();
    }

    if(_stats)
    {
        _stats->bytesReceived(type(), static_cast<Int>(_read.count));
    }

    buf.b.resize(_read.count);
    buf.i = buf.b.end();
}
#endif

string
IceInternal::UdpTransceiver::type() const
{
    return "udp";
}

string
IceInternal::UdpTransceiver::toString() const
{
    if(_mcastAddr.ss_family != AF_UNSPEC && _fd != INVALID_SOCKET)
    {
        return fdToString(_fd) + "\nmulticast address = " + addrToString(_mcastAddr);
    }
    else
    {
        return fdToString(_fd);
    }
}

Ice::ConnectionInfoPtr
IceInternal::UdpTransceiver::getInfo() const
{
    assert(_fd != INVALID_SOCKET);
    Ice::UdpConnectionInfoPtr info = new Ice::UdpConnectionInfo();
    info->endpoint = _endpointInfo;
    fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);
    addrToAddressAndPort(_mcastAddr, info->mcastAddress, info->mcastPort);
    return info;
}

void
IceInternal::UdpTransceiver::checkSendSize(const Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, buf.b.size(), messageSizeMax);
    }
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if(packetSize < static_cast<int>(buf.b.size()))
    {
        throw DatagramLimitException(__FILE__, __LINE__);
    }
}

int
IceInternal::UdpTransceiver::effectivePort() const
{
    return getPort(_addr);
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance,
                                            const Ice::UdpEndpointInfoPtr& endpointInfo,
                                            const struct sockaddr_storage& addr) :
    _endpointInfo(endpointInfo),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(false),
    _addr(addr),
    _connect(true),
    _warn(instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0)
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead), 
    _write(SocketOperationWrite)
#endif
{
    // AF_UNSPEC means not multicast.
    _mcastAddr.ss_family = AF_UNSPEC;

    _fd = createSocket(true, _addr.ss_family);
    setBufSize(instance);
    setBlock(_fd, false);
    
    if(doConnect(_fd, _addr))
    {
        _connect = false; // We're connected now
    }
    
    if(isMulticast(_addr))
    {
        if(endpointInfo->mcastInterface.length() > 0)
        {
            setMcastInterface(_fd, endpointInfo->mcastInterface, _addr.ss_family == AF_INET);
        }
        if(endpointInfo->mcastTtl != -1)
        {
            setMcastTtl(_fd, endpointInfo->mcastTtl, _addr.ss_family == AF_INET);
        }
    }
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance,
                                            const Ice::UdpEndpointInfoPtr& endpointInfo,
                                            bool connect) :
    _endpointInfo(endpointInfo),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(true),
    _addr(getAddressForServer(_endpointInfo->host, _endpointInfo->port, instance->protocolSupport())),
    _connect(connect),
    _warn(instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0)
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead), 
    _write(SocketOperationWrite)
#endif
{
    _fd = createSocket(true, _addr.ss_family);
    setBufSize(instance);
    setBlock(_fd, false);
    if(_traceLevels->network >= 2)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "attempting to bind to udp socket " << addrToString(_addr);
    }

    if(isMulticast(_addr))
    {
        setReuseAddress(_fd, true);
        _mcastAddr = _addr;

#ifdef _WIN32
        //
        // Windows does not allow binding to the mcast address itself
        // so we bind to INADDR_ANY (0.0.0.0) instead.
        //
        const_cast<struct sockaddr_storage&>(_addr) =
            getAddressForServer("", getPort(_mcastAddr),
                                _mcastAddr.ss_family == AF_INET ? EnableIPv4 : EnableIPv6);
#endif

        const_cast<struct sockaddr_storage&>(_addr) = doBind(_fd, _addr);
        if(getPort(_mcastAddr) == 0)
        {
            setPort(_mcastAddr, getPort(_addr));
        }
        setMcastGroup(_fd, _mcastAddr, _endpointInfo->mcastInterface);
    }
    else
    {
#ifndef _WIN32
        //
        // Enable SO_REUSEADDR on Unix platforms to allow re-using
        // the socket even if it's in the TIME_WAIT state. On
        // Windows, this doesn't appear to be necessary and
        // enabling SO_REUSEADDR would actually not be a good
        // thing since it allows a second process to bind to an
        // address even it's already bound by another process.
        //
        // TODO: using SO_EXCLUSIVEADDRUSE on Windows would
        // probably be better but it's only supported by recent
        // Windows versions (XP SP2, Windows Server 2003).
        //
        setReuseAddress(_fd, true);
#endif
        const_cast<struct sockaddr_storage&>(_addr) = doBind(_fd, _addr);

        // AF_UNSPEC means not multicast.
        _mcastAddr.ss_family = AF_UNSPEC;
    }

    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "starting to receive udp packets\n" << toString();
        
        vector<string> interfaces = 
            getHostsForEndpointExpand(inetAddrToString(_addr), instance->protocolSupport(), true);
        if(!interfaces.empty())
        {
            out << "\nlocal interfaces: ";
            out << IceUtilInternal::joinString(interfaces, ", ");
        }
    }
}

IceInternal::UdpTransceiver::~UdpTransceiver()
{
    assert(_fd == INVALID_SOCKET);
}

//
// Set UDP receive and send buffer sizes.
//

void
IceInternal::UdpTransceiver::setBufSize(const InstancePtr& instance)
{
    assert(_fd != INVALID_SOCKET);

    for(int i = 0; i < 2; ++i)
    {
        string direction;
        string prop;
        int* addr;
        int dfltSize;
        if(i == 0)
        {
            direction = "receive";
            prop = "Ice.UDP.RcvSize";
            addr = &_rcvSize;
            dfltSize = getRecvBufferSize(_fd);
            _rcvSize = dfltSize;
        }
        else
        {
            direction = "send";
            prop = "Ice.UDP.SndSize";
            addr = &_sndSize;
            dfltSize = getSendBufferSize(_fd);
            _sndSize = dfltSize;
        }

        //
        // Get property for buffer size and check for sanity.
        //
        Int sizeRequested = instance->initializationData().properties->getPropertyAsIntWithDefault(prop, dfltSize);
        if(sizeRequested < _udpOverhead)
        {
            Warning out(_logger);
            out << "Invalid " << prop << " value of " << sizeRequested << " adjusted to " << dfltSize;
            sizeRequested = dfltSize;
        }

        if(sizeRequested != dfltSize)
        {
            //
            // Try to set the buffer size. The kernel will silently adjust
            // the size to an acceptable value. Then read the size back to
            // get the size that was actually set.
            //
            if(i == 0)
            {
                setRecvBufferSize(_fd, sizeRequested);
                *addr = getRecvBufferSize(_fd);
            }
            else
            {
                setSendBufferSize(_fd, sizeRequested);
                *addr = getSendBufferSize(_fd);
            }

            //
            // Warn if the size that was set is less than the requested size.
            //
            if(*addr < sizeRequested)
            {
                Warning out(_logger);
                out << "UDP " << direction << " buffer size: requested size of "
                    << sizeRequested << " adjusted to " << *addr;
            }
        }
    }
}

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
