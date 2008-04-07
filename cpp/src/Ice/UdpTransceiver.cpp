// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Stats.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

SOCKET
IceInternal::UdpTransceiver::fd()
{
    assert(_fd != INVALID_SOCKET);
    return _fd;
}

void
IceInternal::UdpTransceiver::close()
{
    if(_traceLevels->network >= 1)
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
        cerr << packetSize << " " << _maxPacketSize << " " << _sndSize << endl;
        throw DatagramLimitException(__FILE__, __LINE__);
    }

repeat:

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

string
IceInternal::UdpTransceiver::type() const
{
    return "udp";
}

string
IceInternal::UdpTransceiver::toString() const
{
    if(_mcastServer && _fd != INVALID_SOCKET)
    {
        struct sockaddr_storage remoteAddr;
        bool peerConnected = fdToRemoteAddress(_fd, remoteAddr);
        return addressesToString(_addr, remoteAddr, peerConnected);
    }
    else
    {
        return fdToString(_fd);
    }
}

SocketStatus
IceInternal::UdpTransceiver::initialize()
{
    if(!_incoming)
    {
        if(_connect)
        {
            //
            // If we're not connected yet, return NeedConnect. The transceiver will be 
            // connected once initialize is called again.
            //
            _connect = false;
            return NeedConnect;
        }
        else
        {
            if(_traceLevels->network >= 1)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "starting to send udp packets\n" << toString();
            }
            return Finished;
        }
    }
    return Finished;
}

void
IceInternal::UdpTransceiver::checkSendSize(const Buffer& buf, size_t messageSizeMax)
{
    if(buf.b.size() > messageSizeMax)
    {
        throw MemoryLimitException(__FILE__, __LINE__);
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

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const struct sockaddr_storage& addr,
                                            const string& mcastInterface, int mcastTtl) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(false),
    _addr(addr),
    _connect(true),
    _warn(instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0)
{
    try
    {
        _fd = createSocket(true, _addr.ss_family);
        setBufSize(instance);
        setBlock(_fd, false);

        if(doConnect(_fd, _addr))
        {
            _connect = false; // We're connected now
        }

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
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

#ifdef _WIN32
    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
#endif
}

IceInternal::UdpTransceiver::UdpTransceiver(const InstancePtr& instance, const string& host, int port,
                                            const string& mcastInterface, bool connect) :
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _stats(instance->initializationData().stats),
    _incoming(true),
    _connect(connect),
    _warn(instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0)
{
    try
    {
        getAddressForServer(host, port, _addr, instance->protocolSupport());
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

#ifdef _WIN32
            //
            // Windows does not allow binding to the mcast address itself
            // so we bind to INADDR_ANY (0.0.0.0) instead.
            //
            struct sockaddr_storage addr;
            getAddressForServer("", getPort(_addr), addr, _addr.ss_family == AF_INET ? EnableIPv4 : EnableIPv6);
            doBind(_fd, addr);
#else
            doBind(_fd, _addr);
#endif
            setMcastGroup(_fd, _addr, mcastInterface);
            _mcastServer = true;
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
            doBind(_fd, _addr);
        }

        if(_traceLevels->network >= 1)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "starting to receive udp packets\n" << toString();
        }
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

#ifdef _WIN32
    FD_ZERO(&_rFdSet);
    FD_ZERO(&_wFdSet);
#endif
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
