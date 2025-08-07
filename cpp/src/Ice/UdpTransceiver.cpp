// Copyright (c) ZeroC, Inc.

#include "UdpTransceiver.h"
#include "Ice/Buffer.h"
#include "Ice/Connection.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/StringUtil.h"
#include "ProtocolInstance.h"
#include "UdpEndpointI.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::UdpTransceiver::getNativeInfo()
{
    return shared_from_this();
}

#if defined(ICE_USE_IOCP)
AsyncInfo*
IceInternal::UdpTransceiver::getAsyncInfo(SocketOperation status)
{
    switch (status)
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
IceInternal::UdpTransceiver::initialize(Buffer& /*readBuffer*/, Buffer& /*writeBuffer*/)
{
    if (_state == StateNeedConnect)
    {
        _state = StateConnectPending;
        return SocketOperationConnect;
    }
    else if (_state <= StateConnectPending)
    {
#if defined(ICE_USE_IOCP)
        doFinishConnectAsync(_fd, _write);
#else
        doFinishConnect(_fd);
#endif
        _state = StateConnected;
    }

    assert(_state >= StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceInternal::UdpTransceiver::closing(bool, exception_ptr)
{
    // Nothing to do.
    return SocketOperationNone;
}

void
IceInternal::UdpTransceiver::close()
{
    assert(_fd != INVALID_SOCKET);
    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

EndpointIPtr
IceInternal::UdpTransceiver::bind()
{
    if (isMulticast(_addr))
    {
        setReuseAddress(_fd, true);
        _mcastAddr = _addr;

#ifdef _WIN32
        // Windows does not allow binding to the mcast address itself so we bind to INADDR_ANY (0.0.0.0) instead. As a
        // result, bi-directional connection won't work because the source address won't be the multicast address and
        // the client will therefore reject the datagram.
        const_cast<Address&>(_addr) = getAddressForServer("", _port, getProtocolSupport(_addr), false, false);
#endif

        const_cast<Address&>(_addr) = doBind(_fd, _addr, _mcastInterface);
        if (getPort(_mcastAddr) == 0)
        {
            setPort(_mcastAddr, getPort(_addr));
        }
        setMcastGroup(_fd, _mcastAddr, _mcastInterface);
    }
    else
    {
#ifndef _WIN32
        // Enable SO_REUSEADDR on Unix platforms to allow re-using the socket even if it's in the TIME_WAIT state. On
        // Windows, this doesn't appear to be necessary and enabling SO_REUSEADDR would actually not be a good thing
        // since it allows a second process to bind to an address even it's already bound by another process. TODO:
        // using SO_EXCLUSIVEADDRUSE on Windows would probably be better but it's only supported by recent Windows
        // versions (XP SP2, Windows Server 2003).
        setReuseAddress(_fd, true);
#endif
        const_cast<Address&>(_addr) = doBind(_fd, _addr);
    }

    _bound = true;

    _endpoint = _endpoint->endpoint(shared_from_this());
    return _endpoint;
}

SocketOperation
IceInternal::UdpTransceiver::write(Buffer& buf)
{
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    assert(buf.i == buf.b.begin());
    assert(_fd != INVALID_SOCKET && _state >= StateConnected);

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

repeat:

    ssize_t ret;
    if (_state == StateConnected)
    {
#ifdef _WIN32
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0);
#else
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0);
#endif
    }
    else
    {
        auto len = static_cast<socklen_t>(sizeof(sockaddr_storage));
        if (_peerAddr.saStorage.ss_family == AF_INET)
        {
            len = static_cast<socklen_t>(sizeof(sockaddr_in));
        }
        else if (_peerAddr.saStorage.ss_family == AF_INET6)
        {
            len = static_cast<socklen_t>(sizeof(sockaddr_in6));
        }
        else
        {
            // No peer has sent a datagram yet.
            throw SocketException(__FILE__, __LINE__, 0);
        }

#ifdef _WIN32
        ret = ::sendto(
            _fd,
            reinterpret_cast<const char*>(&buf.b[0]),
            static_cast<int>(buf.b.size()),
            0,
            &_peerAddr.sa,
            len);
#else
        ret = ::sendto(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0, &_peerAddr.sa, len);
#endif
    }

    if (ret == SOCKET_ERROR)
    {
        if (interrupted())
        {
            goto repeat;
        }

        if (wouldBlock())
        {
            return SocketOperationWrite;
        }

        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    assert(ret == static_cast<ssize_t>(buf.b.size()));
    buf.i = buf.b.end();
    return SocketOperationNone;
}

SocketOperation
IceInternal::UdpTransceiver::read(Buffer& buf)
{
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    assert(buf.i == buf.b.begin());
    assert(_fd != INVALID_SOCKET);

#ifdef _WIN32
    int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
#else
    const size_t packetSize = static_cast<size_t>(min(_maxPacketSize, _rcvSize - _udpOverhead));
#endif
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

repeat:

#ifdef _WIN32
    int ret;
#else
    ssize_t ret;
#endif
    if (_state == StateConnected)
    {
        ret = ::recv(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0);
    }
    else
    {
        assert(_incoming);

        Address peerAddr;
        memset(&peerAddr.saStorage, 0, sizeof(sockaddr_storage));
        auto len = static_cast<socklen_t>(sizeof(sockaddr_storage));

        ret = recvfrom(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0, &peerAddr.sa, &len);

        if (ret != SOCKET_ERROR)
        {
            _peerAddr = peerAddr;
        }
    }

    if (ret == SOCKET_ERROR)
    {
        if (recvTruncated())
        {
            // The message was truncated and the whole buffer is filled. We ignore
            // this error here, it will be detected at the connection level when
            // the Ice message size is checked against the buffer size.
            ret = static_cast<ssize_t>(buf.b.size());
        }
        else
        {
            if (interrupted())
            {
                goto repeat;
            }

            if (wouldBlock())
            {
                return SocketOperationRead;
            }

            if (connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }

    // Client connections are connected at this point, and server connections are never connected.
    assert(_state != StateNeedConnect);

    buf.b.resize(static_cast<size_t>(ret));
    buf.i = buf.b.end();
    return SocketOperationNone;
}

#if defined(ICE_USE_IOCP)
bool
IceInternal::UdpTransceiver::startWrite(Buffer& buf)
{
    assert(buf.i == buf.b.begin());

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));
    assert(_fd != INVALID_SOCKET);

    _write.buf.len = static_cast<int>(buf.b.size());
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    _write.error = ERROR_SUCCESS;
    int err;
    if (_state == StateConnected)
    {
        err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, nullptr);
    }
    else
    {
        socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
        if (_peerAddr.saStorage.ss_family == AF_INET)
        {
            len = sizeof(sockaddr_in);
        }
        else if (_peerAddr.saStorage.ss_family == AF_INET6)
        {
            len = sizeof(sockaddr_in6);
        }
        else
        {
            // No peer has sent a datagram yet.
            throw SocketException(__FILE__, __LINE__, 0);
        }
        err = WSASendTo(_fd, &_write.buf, 1, &_write.count, 0, &_peerAddr.sa, len, &_write, nullptr);
    }

    if (err == SOCKET_ERROR)
    {
        if (!wouldBlock())
        {
            if (connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
    return true;
}

void
IceInternal::UdpTransceiver::finishWrite(Buffer& buf)
{
    if (_fd == INVALID_SOCKET || _state < StateConnected)
    {
        return;
    }

    if (_write.error != ERROR_SUCCESS)
    {
        WSASetLastError(_write.error);
        if (connectionLost())
        {
            throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    assert(static_cast<size_t>(_write.count) == buf.b.size());
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
    _read.error = ERROR_SUCCESS;
    int err;
    if (_state == StateConnected)
    {
        err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, nullptr);
    }
    else
    {
        memset(&_readAddr.saStorage, 0, sizeof(struct sockaddr_storage));
        _readAddrLen = static_cast<socklen_t>(sizeof(sockaddr_storage));

        err =
            WSARecvFrom(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_readAddr.sa, &_readAddrLen, &_read, nullptr);
    }

    if (err == SOCKET_ERROR)
    {
        if (recvTruncated())
        {
            // Nothing to do.
        }
        else if (!wouldBlock())
        {
            if (connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
}

void
IceInternal::UdpTransceiver::finishRead(Buffer& buf)
{
    if (_read.error != ERROR_SUCCESS)
    {
        WSASetLastError(_read.error);

        if (recvTruncated())
        {
            // The message was truncated and the whole buffer is filled. We ignore
            // this error here, it will be detected at the connection level when
            // the Ice message size is checked against the buffer size.
            _read.count = static_cast<int>(buf.b.size());
        }
        else
        {
            if (connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }

    if (_state == StateNotConnected)
    {
        _peerAddr = _readAddr;
    }

    int ret = _read.count;

    buf.b.resize(ret);
    buf.i = buf.b.end();
}
#endif

string
IceInternal::UdpTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::UdpTransceiver::toString() const
{
    if (_fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    ostringstream s;
    if (_incoming && !_bound)
    {
        s << "local address = " << addrToString(_addr);
    }
    else if (_state == StateNotConnected)
    {
        Address localAddr;
        fdToLocalAddress(_fd, localAddr);
        s << "local address = " << addrToString(localAddr);
    }
    else
    {
        s << fdToString(_fd);
    }

    if (isAddressValid(_mcastAddr))
    {
        s << "\nmulticast address = " + addrToString(_mcastAddr);
    }
    return s.str();
}

string
IceInternal::UdpTransceiver::toDetailedString() const
{
    ostringstream os;
    os << toString();
    if (isAddressValid(_mcastAddr))
    {
        vector<string> intfs = getInterfacesForMulticast(_mcastInterface, getProtocolSupport(_mcastAddr));
        if (!intfs.empty())
        {
            os << "\nlocal interfaces = ";
            os << IceInternal::joinString(intfs, ", ");
        }
    }
    return os.str();
}

Ice::ConnectionInfoPtr
IceInternal::UdpTransceiver::getInfo(bool incoming, string adapterName, string connectionId) const
{
    if (_fd == INVALID_SOCKET)
    {
        return make_shared<UDPConnectionInfo>(incoming, std::move(adapterName), std::move(connectionId));
    }
    else
    {
        string mcastAddress;
        int mcastPort = 0;
        if (isAddressValid(_mcastAddr))
        {
            addrToAddressAndPort(_mcastAddr, mcastAddress, mcastPort);
        }

        if (_state == StateNotConnected)
        {
            assert(_incoming);
            Address localAddr;
            fdToLocalAddress(_fd, localAddr);
            string localAddress;
            int localPort;
            addrToAddressAndPort(localAddr, localAddress, localPort);

            // Since this info is cached in the Connection object shared by all the clients, we don't store the
            // remote address/port of the latest client in this info.
            return make_shared<UDPConnectionInfo>(
                incoming,
                std::move(adapterName),
                std::move(connectionId),
                std::move(localAddress),
                localPort,
                "", // remoteAddress
                -1, // remotePort
                std::move(mcastAddress),
                mcastPort,
                _rcvSize,
                _sndSize);
        }
        else
        {
            assert(!_incoming);
            string localAddress;
            int localPort;
            string remoteAddress;
            int remotePort;
            fdToAddressAndPort(_fd, localAddress, localPort, remoteAddress, remotePort);

            return make_shared<UDPConnectionInfo>(
                incoming,
                std::move(adapterName),
                std::move(connectionId),
                std::move(localAddress),
                localPort,
                std::move(remoteAddress),
                remotePort,
                std::move(mcastAddress),
                mcastPort,
                _rcvSize,
                _sndSize);
        }
    }
}

void
IceInternal::UdpTransceiver::checkSendSize(const Buffer& buf)
{
    // The maximum packetSize is either the maximum allowable UDP packet size, or the UDP send buffer size (which ever
    // is smaller).
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if (packetSize < static_cast<int>(buf.b.size()))
    {
        throw DatagramLimitException(__FILE__, __LINE__);
    }
}

void
IceInternal::UdpTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    setBufSize(rcvSize, sndSize);
}

int
IceInternal::UdpTransceiver::effectivePort() const
{
    return getPort(_addr);
}

IceInternal::UdpTransceiver::UdpTransceiver(
    ProtocolInstancePtr instance,
    const Address& addr,
    const Address& sourceAddr,
    const string& mcastInterface,
    int mcastTtl)
    : _instance(std::move(instance)),
      _incoming(false),
      _bound(false),
      _addr(addr),
      _state(StateNeedConnect)
#if defined(ICE_USE_IOCP)
      ,
      _read(SocketOperationRead),
      _write(SocketOperationWrite)
#endif
{
    _fd = createSocket(true, _addr);
    setBufSize(-1, -1);
    setBlock(_fd, false);

    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
    _peerAddr.saStorage.ss_family = AF_UNSPEC; // Not initialized yet.

    // NOTE: setting the multicast interface before performing the connect is important for some OS such as macOS.
    if (isMulticast(_addr))
    {
        if (mcastInterface.length() > 0)
        {
            setMcastInterface(_fd, mcastInterface, _addr);
        }
        if (mcastTtl != -1)
        {
            setMcastTtl(_fd, mcastTtl, _addr);
        }
    }

    // In general, connecting a datagram socket should be non-blocking as this just setups the default destination
    // address for the socket. However, on some OS, connect sometime returns EWOULDBLOCK. If that's the case, we keep
    // the state as StateNeedConnect. This will make sure the transceiver is notified when the socket is ready for
    // sending (see the initialize() implementation).
    if (doConnect(_fd, _addr, sourceAddr))
    {
        _state = StateConnected;
    }

#ifdef ICE_USE_IOCP
    // On Windows when using IOCP, we must make sure that the socket is connected without blocking as there's no way to
    // do a non-blocking datagram socket connection (ConnectEx only supports connection oriented sockets). According to
    // Microsoft documentation of the connect() call, this should always be the case.
    assert(_state == StateConnected);
#endif
}

IceInternal::UdpTransceiver::UdpTransceiver(
    UdpEndpointIPtr endpoint,
    const ProtocolInstancePtr& instance,
    const string& host,
    int port,
    string mcastInterface)
    : _endpoint(std::move(endpoint)),
      _instance(instance),
      _incoming(true),
      _bound(false),
      _addr(getAddressForServer(host, port, instance->protocolSupport(), instance->preferIPv6(), true)),
      _mcastInterface(std::move(mcastInterface)),
#ifdef _WIN32
      _port(port),
#endif
      _state(StateNotConnected)
#if defined(ICE_USE_IOCP)
      ,
      _read(SocketOperationRead),
      _write(SocketOperationWrite)
#endif
{
    _fd = createServerSocket(true, _addr, instance->protocolSupport());
    setBufSize(-1, -1);
    setBlock(_fd, false);

    memset(&_mcastAddr.saStorage, 0, sizeof(sockaddr_storage));
    memset(&_peerAddr.saStorage, 0, sizeof(sockaddr_storage));
    _peerAddr.saStorage.ss_family = AF_UNSPEC;
    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
}

IceInternal::UdpTransceiver::~UdpTransceiver() { assert(_fd == INVALID_SOCKET); }

// Set UDP receive and send buffer sizes.
void
IceInternal::UdpTransceiver::setBufSize(int rcvSize, int sndSize)
{
    assert(_fd != INVALID_SOCKET);

    for (int i = 0; i < 2; ++i)
    {
        bool isSnd;
        string direction;
        string prop;
        int* addr;
        int dfltSize;
        int sizeRequested;
        if (i == 0)
        {
            isSnd = false;
            direction = "receive";
            prop = "Ice.UDP.RcvSize";
            addr = &_rcvSize;
            dfltSize = getRecvBufferSize(_fd);
            sizeRequested = rcvSize;
        }
        else
        {
            isSnd = true;
            direction = "send";
            prop = "Ice.UDP.SndSize";
            addr = &_sndSize;
            dfltSize = getSendBufferSize(_fd);
            sizeRequested = sndSize;
        }

        if (dfltSize <= 0)
        {
            dfltSize = _maxPacketSize;
        }
        *addr = dfltSize;

        // Get property for buffer size if size not passed in.
        if (sizeRequested == -1)
        {
            sizeRequested = _instance->properties()->getPropertyAsIntWithDefault(prop, dfltSize);
        }
        // Check for sanity.
        if (sizeRequested < (_udpOverhead + headerSize))
        {
            Warning out(_instance->logger());
            out << "Invalid " << prop << " value of " << sizeRequested << " adjusted to " << dfltSize;
            sizeRequested = dfltSize;
        }

        if (sizeRequested != dfltSize)
        {
            // Try to set the buffer size. The kernel will silently adjust the size to an acceptable value. Then read
            // the size back to get the size that was actually set.
            if (i == 0)
            {
                setRecvBufferSize(_fd, sizeRequested);
                *addr = getRecvBufferSize(_fd);
            }
            else
            {
                setSendBufferSize(_fd, sizeRequested);
                *addr = getSendBufferSize(_fd);
            }

            // Warn if the size that was set is less than the requested size and we have not already warned.
            if (*addr == 0) // set buffer size not supported.
            {
                *addr = sizeRequested;
            }
            else if (*addr < sizeRequested)
            {
                BufSizeWarnInfo winfo = _instance->getBufSizeWarn(UDPEndpointType);
                if ((isSnd && (!winfo.sndWarn || winfo.sndSize != sizeRequested)) ||
                    (!isSnd && (!winfo.rcvWarn || winfo.rcvSize != sizeRequested)))
                {
                    Warning out(_instance->logger());
                    out << "UDP " << direction << " buffer size: requested size of " << sizeRequested << " adjusted to "
                        << *addr;

                    if (isSnd)
                    {
                        _instance->setSndBufSizeWarn(UDPEndpointType, sizeRequested);
                    }
                    else
                    {
                        _instance->setRcvBufSizeWarn(UDPEndpointType, sizeRequested);
                    }
                }
            }
        }
    }
}

// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header to get the
// maximum payload.
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
