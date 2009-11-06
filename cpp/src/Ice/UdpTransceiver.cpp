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
    if(_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return SocketOperationConnect;
    }
    else if(_state <= StateConnectPending)
    {
        try
        {
            doFinishConnect(_fd);
            _state = StateConnected;
        }
        catch(const Ice::LocalException& ex)
        {
            if(_traceLevels->network >= 2)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "failed to connect udp socket\n" << toString() << "\n" << ex;
            }
            throw;
        }
    }

    if(_state == StateConnected)
    {
        if(_traceLevels->network >= 1)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "starting to send udp packets\n" << toString();
        }
    }
    assert(_state >= StateConnected);
    return SocketOperationNone;
}

void
IceInternal::UdpTransceiver::close()
{
    if(_state >= StateConnected && _traceLevels->network >= 1)
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
    assert(_fd != INVALID_SOCKET && _state >= StateConnected);

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

repeat:

    ssize_t ret;
    if(_state == StateConnected)
    {
#ifdef _WIN32
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0);
#else
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0);
#endif
    }
    else
    {
        socklen_t len = static_cast<socklen_t>(sizeof(_peerAddr));
        if(_peerAddr.ss_family == AF_INET)
        {
            len = sizeof(sockaddr_in);
        }
        else if(_peerAddr.ss_family == AF_INET6)
        {
            len = sizeof(sockaddr_in6);
        }
        else
        {
            // No peer has sent a datagram yet.
            SocketException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

#ifdef _WIN32
        ret = ::sendto(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0, 
                       reinterpret_cast<struct sockaddr*>(&_peerAddr), len);
#else
        ret = ::sendto(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0,
                       reinterpret_cast<struct sockaddr*>(&_peerAddr), len);
#endif
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
    assert(_fd != INVALID_SOCKET);

    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:

    ssize_t ret;
    if(_state == StateConnected)
    {
        ret = ::recv(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0);
    }
    else
    {
        assert(_incoming);

        sockaddr_storage peerAddr;
        memset(&peerAddr, 0, sizeof(struct sockaddr_storage));
        socklen_t len = static_cast<socklen_t>(sizeof(peerAddr));

        ret = recvfrom(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0, 
                       reinterpret_cast<struct sockaddr*>(&peerAddr), &len);

        if(ret != SOCKET_ERROR)
        {
            _peerAddr = peerAddr;
        }
    }

    if(ret == SOCKET_ERROR)
    {
        if(recvTruncated())
        {
            // The message was truncated and the whole buffer is filled. We ignore 
            // this error here, it will be detected at the connection level when
            // the Ice message size is checked against the buffer size.
            ret = static_cast<ssize_t>(buf.b.size());
        }
        else
        {
            if(interrupted())
            {
                goto repeat;
            }
            
            if(wouldBlock())
            {
                return false;
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
    }
    
    if(_state == StateNeedConnect)
    {
        //
        // If we must connect, we connect to the first peer that sends us a packet.
        //
        assert(_incoming); // Client connections should always be connected at this point.

#ifndef NDEBUG
        bool connected = doConnect(_fd, _peerAddr);
        assert(connected);
#else
        doConnect(_fd, _peerAddr);
#endif
        _state = StateConnected;

        if(_traceLevels->network >= 1)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "connected udp socket\n" << toString();
        }
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

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

    assert(_fd != INVALID_SOCKET);

    _write.buf.len = static_cast<int>(buf.b.size());
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err;
    if(_state == StateConnected)
    {
        err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, NULL);
    }
    else
    {
        socklen_t len = static_cast<socklen_t>(sizeof(_peerAddr));
        if(_peerAddr.ss_family == AF_INET)
        {
            len = sizeof(sockaddr_in);
        }
        else if(_peerAddr.ss_family == AF_INET6)
        {
            len = sizeof(sockaddr_in6);
        }
        else
        {
            // No peer has sent a datagram yet.
            SocketException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }
        err = WSASendTo(_fd, &_write.buf, 1, &_write.count, 0, reinterpret_cast<struct sockaddr*>(&_peerAddr), 
                        len, &_write, NULL);
    }

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
    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

    assert(!buf.b.empty() && buf.i != buf.b.end());

    _read.buf.len = packetSize;
    _read.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err;
    if(_state == StateConnected)
    {
        err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, NULL);
    }
    else
    {
        memset(&_readAddr, 0, sizeof(struct sockaddr_storage));
        _readAddrLen = static_cast<socklen_t>(sizeof(_readAddr));
        
        err = WSARecvFrom(_fd, &_read.buf, 1, &_read.count, &_read.flags, 
                          reinterpret_cast<struct sockaddr*>(&_readAddr), &_readAddrLen, &_read, NULL);
    }

    if(err == SOCKET_ERROR)
    {
        if(recvTruncated())
        {
            // Nothing to do.
        }
        else if(!wouldBlock())
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

        if(recvTruncated())
        {
            // The message was truncated and the whole buffer is filled. We ignore 
            // this error here, it will be detected at the connection level when
            // the Ice message size is checked against the buffer size.
            _read.count = static_cast<int>(buf.b.size());
        }
        else
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
    
    if(_state == StateNotConnected)
    {
        _peerAddr = _readAddr;
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
    if(_fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    ostringstream s;
    if(_state == StateNotConnected)
    {
        struct sockaddr_storage localAddr;
        fdToLocalAddress(_fd, localAddr);
        s << "local address = " << addrToString(localAddr);
        if(_peerAddr.ss_family != AF_UNSPEC)
        {
            s << "\nremote address = " << addrToString(_peerAddr);
        }
    }
    else
    {
        s << fdToString(_fd);
    }

    if(_mcastAddr.ss_family != AF_UNSPEC)
    {
        s << "\nmulticast address = " + addrToString(_mcastAddr);
    }
    return s.str();
}

Ice::ConnectionInfoPtr
IceInternal::UdpTransceiver::getInfo() const
{
    assert(_fd != INVALID_SOCKET);
    Ice::UDPConnectionInfoPtr info = new Ice::UDPConnectionInfo();
    if(_state == StateNotConnected)
    {
        struct sockaddr_storage localAddr;
        fdToLocalAddress(_fd, localAddr);
        addrToAddressAndPort(localAddr, info->localAddress, info->localPort);
        if(_peerAddr.ss_family != AF_UNSPEC)
        {
            addrToAddressAndPort(_peerAddr, info->remoteAddress, info->remotePort);
        }
        else
        {
            info->remotePort = 0;
        }
    }
    else
    {
        fdToAddressAndPort(_fd, info->localAddress, info->localPort, info->remoteAddress, info->remotePort);   
    }

    if(_mcastAddr.ss_family != AF_UNSPEC)
    {
        addrToAddressAndPort(_mcastAddr, info->mcastAddress, info->mcastPort);
    }
    else
    {
        info->mcastPort = 0;
    }
    return info;
}

void
IceInternal::UdpTransceiver::checkSendSize(const Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, buf.b.size(), messageSizeMax);
    }

    //
    // The maximum packetSize is either the maximum allowable UDP packet size, or 
    // the UDP send buffer size (which ever is smaller).
    //
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

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const struct sockaddr_storage& addr,
                                            const string& mcastInterface, int mcastTtl) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(false),
    _addr(addr),
    _state(StateNeedConnect)
#ifdef ICE_USE_IOCP
    , _read(SocketOperationRead), 
    _write(SocketOperationWrite)
#endif
{
    _mcastAddr.ss_family = AF_UNSPEC; // AF_UNSPEC means not multicast.
    _peerAddr.ss_family = AF_UNSPEC; // Not initialized yet.

    _fd = createSocket(true, _addr.ss_family);
    setBufSize(instance);
    setBlock(_fd, false);
    
    //
    // In general, connecting a datagram socket should be non-blocking as this just setups 
    // the default destination address for the socket. However, on some OS, connect sometime
    // returns EWOULDBLOCK. If that's the case, we keep the state as StateNeedConnect. This
    // will make sure the transceiver is notified when the socket is ready for sending (see
    // the initialize() implementation).
    //
    if(doConnect(_fd, _addr))
    {
        _state = StateConnected;
    }

#ifdef ICE_USE_IOCP
    //
    // On Windows when using IOCP, we must make sure that the socket is connected without 
    // blocking as there's no way to do a non-blocking datagram socket conection (ConnectEx
    // only supports connection oriented sockets). According to Microsoft documentation of
    // the connect() call, this should always be the case.
    //
    assert(_state == StateConnected);
#endif
    
    if(isMulticast(_addr))
    {
        if(mcastInterface.length() > 0)
        {
            setMcastInterface(_fd, mcastInterface, _addr.ss_family == AF_INET);
        }
        if(mcastTtl != -1)
        {
            setMcastTtl(_fd, mcastTtl, _addr.ss_family == AF_INET);
        }
    }
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port,
                                            const string& mcastInterface, bool connect) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(true),
    _addr(getAddressForServer(host, port, instance->protocolSupport())),
    _state(connect ? StateNeedConnect : StateNotConnected)
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

    _peerAddr.ss_family = AF_UNSPEC; // Not assigned yet.

    if(isMulticast(_addr))
    {
        setReuseAddress(_fd, true);
        _mcastAddr = _addr;

#ifdef _WIN32
        //
        // Windows does not allow binding to the mcast address itself
        // so we bind to INADDR_ANY (0.0.0.0) instead. As a result,
        // bi-directional connection won't work because the source 
        // address won't be the multicast address and the client will
        // therefore reject the datagram.
        //
        const_cast<struct sockaddr_storage&>(_addr) = 
            getAddressForServer("", port, _mcastAddr.ss_family == AF_INET ? EnableIPv4 : EnableIPv6);
#endif

        const_cast<struct sockaddr_storage&>(_addr) = doBind(_fd, _addr);
        if(getPort(_mcastAddr) == 0)
        {
            setPort(_mcastAddr, getPort(_addr));
        }
        setMcastGroup(_fd, _mcastAddr, mcastInterface);
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

        _mcastAddr.ss_family = AF_UNSPEC; // AF_UNSPEC means not multicast.
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
        if(sizeRequested < (_udpOverhead + headerSize))
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
