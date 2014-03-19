// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UdpTransceiver.h>
#include <Ice/Connection.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <IceUtil/StringUtil.h>

#include <IceUtil/DisableWarnings.h>
#include <Ice/Stats.h>

#ifdef ICE_OS_WINRT
#   include <ppltasks.h> // For Concurrency::task
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
#ifdef ICE_OS_WINRT
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
#endif

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
#elif defined(ICE_OS_WINRT)
void 
IceInternal::UdpTransceiver::setCompletedHandler(SocketOperationCompletedHandler^ handler)
{
    _completedHandler = handler;
    _writeOperationCompletedHandler = ref new AsyncOperationCompletedHandler<unsigned int>(
        [=] (IAsyncOperation<unsigned int>^ operation, Windows::Foundation::AsyncStatus status)
        {
            if(status != Windows::Foundation::AsyncStatus::Completed)
            {
                _write.count = SOCKET_ERROR;
                _write.error = operation->ErrorCode.Value;
            }
            else
            {
                _write.count = static_cast<int>(operation->GetResults());
            }
            _completedHandler(SocketOperationWrite);
        });
}
#endif

SocketOperation
IceInternal::UdpTransceiver::initialize(Buffer& /*readBuffer*/, Buffer& /*writeBuffer*/, bool& /*hasMoreData*/)
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
#if defined(ICE_USE_IOCP)
            doFinishConnectAsync(_fd, _write);
#elif defined(ICE_OS_WINRT)
            if(_write.count == SOCKET_ERROR)
            {
                checkConnectErrorCode(__FILE__, __LINE__, _write.error, _addr.host);
            }
#else
            doFinishConnect(_fd);
#endif
            _state = StateConnected;
        }
        catch(const Ice::LocalException& ex)
        {
            if(_instance->traceLevel() >= 2)
            {
                Trace out(_instance->logger(), _instance->traceCategory());
                out << "failed to connect " << _instance->protocol() << " socket\n" << toString() << "\n" << ex;
            }
            throw;
        }
    }

    if(_state == StateConnected)
    {
        if(_instance->traceLevel() >= 1)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "starting to send " << _instance->protocol() << " packets\n" << toString();
        }
    }
    assert(_state >= StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceInternal::UdpTransceiver::closing(bool, const Ice::LocalException&)
{
    // Nothing to do.
    return SocketOperationNone;
}

void
IceInternal::UdpTransceiver::close()
{
    if(_state >= StateConnected && _instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "closing " << _instance->protocol() << " connection\n" << toString();
    }

#ifdef ICE_OS_WINRT
    IceUtil::Mutex::Lock lock(_mutex);
    if(_readPending)
    {
        assert(_received.empty());
        _completedHandler(SocketOperationRead);
        _readPending = false;
    }
    _received.clear();
#endif

    assert(_fd != INVALID_SOCKET);
    closeSocket(_fd);
    _fd = INVALID_SOCKET;
}


#ifdef ICE_OS_WINRT
SocketOperation
IceInternal::UdpTransceiver::write(Buffer&)
{
    return SocketOperationWrite;
}
#else
SocketOperation
IceInternal::UdpTransceiver::write(Buffer& buf)
{
    if(buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    assert(buf.i == buf.b.begin());
    assert(_fd != INVALID_SOCKET && _state >= StateConnected);

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

repeat:

    ssize_t ret;
    if(_state == StateConnected)
    {
#   ifdef _WIN32
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0);
#   else
        ret = ::send(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0);
#   endif
    }
    else
    {
        socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
        if(_peerAddr.saStorage.ss_family == AF_INET)
        {
            len = static_cast<socklen_t>(sizeof(sockaddr_in));
        }
        else if(_peerAddr.saStorage.ss_family == AF_INET6)
        {
            len = static_cast<socklen_t>(sizeof(sockaddr_in6));
        }
        else
        {
            // No peer has sent a datagram yet.
            SocketException ex(__FILE__, __LINE__);
            ex.error = 0;
            throw ex;
        }

#   ifdef _WIN32
        ret = ::sendto(_fd, reinterpret_cast<const char*>(&buf.b[0]), static_cast<int>(buf.b.size()), 0, 
                       &_peerAddr.sa, len);
#   else
        ret = ::sendto(_fd, reinterpret_cast<const char*>(&buf.b[0]), buf.b.size(), 0,
                       &_peerAddr.sa, len);
#   endif
    }

    if(ret == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeat;
        }

        if(wouldBlock())
        {
            return SocketOperationWrite;
        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(_instance->traceLevel() >= 3)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "sent " << ret << " bytes via " << _instance->protocol() << '\n' << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesSent(protocol(), static_cast<Int>(ret));
    }

    assert(ret == static_cast<ssize_t>(buf.b.size()));
    buf.i = buf.b.end();
    return SocketOperationNone;
}
#endif

#ifdef ICE_OS_WINRT
SocketOperation
IceInternal::UdpTransceiver::read(Buffer&, bool&)
{
    return SocketOperationRead;
}
#else
SocketOperation
IceInternal::UdpTransceiver::read(Buffer& buf, bool&)
{
    if(buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }

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

        Address peerAddr;
        memset(&peerAddr.saStorage, 0, sizeof(sockaddr_storage));
        socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));

        ret = recvfrom(_fd, reinterpret_cast<char*>(&buf.b[0]), packetSize, 0, 
                       &peerAddr.sa, &len);

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
                return SocketOperationRead;
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

#   ifndef NDEBUG
        bool connected = doConnect(_fd, _peerAddr);
        assert(connected);
#   else
        doConnect(_fd, _peerAddr);
#   endif
        _state = StateConnected;

        if(_instance->traceLevel() >= 1)
        {
            Trace out(_instance->logger(), _instance->traceCategory());
            out << "connected " << _instance->protocol() << " socket\n" << toString();
        }
    }

    if(_instance->traceLevel() >= 3)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "received " << ret << " bytes via " << _instance->protocol() << '\n' << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesReceived(protocol(), static_cast<Int>(ret));
    }

    buf.b.resize(ret);
    buf.i = buf.b.end();
    return SocketOperationNone;
}
#endif

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
bool
IceInternal::UdpTransceiver::startWrite(Buffer& buf)
{
    assert(buf.i == buf.b.begin());

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));
    assert(_fd != INVALID_SOCKET);

#ifdef ICE_OS_WINRT
    if(_state < StateConnected)
    {
        try
        {
            bool multicast = isMulticast(_addr) || isAddressValid(_mcastAddr);
            if(!multicast)
            {
                auto operation = safe_cast<DatagramSocket^>(_fd)->ConnectAsync(_addr.host, _addr.port);
                if(!checkIfErrorOrCompleted(SocketOperationConnect, operation))
                {
                    operation->Completed = ref new AsyncActionCompletedHandler(
                        [=] (IAsyncAction^ info, Windows::Foundation::AsyncStatus status)
                        {
                            if(status != Windows::Foundation::AsyncStatus::Completed)
                            {
                                _write.count = SOCKET_ERROR;
                                _write.error = info->ErrorCode.Value;
                            }
                            else
                            {
                                _write.count = 0;
                                _writer = ref new DataWriter(safe_cast<DatagramSocket^>(_fd)->OutputStream);
                            }
                            _completedHandler(SocketOperationConnect);
                        });
                }
                else
                {
                    _write.count = 0;
                    _writer = ref new DataWriter(safe_cast<DatagramSocket^>(_fd)->OutputStream);
                }
            }
            else
            {
                auto operation = safe_cast<DatagramSocket^>(_fd)->GetOutputStreamAsync(_addr.host, _addr.port);
                if(!checkIfErrorOrCompleted(SocketOperationConnect, operation))
                {
                    operation->Completed = ref new AsyncOperationCompletedHandler<IOutputStream^>(
                        [=] (IAsyncOperation<IOutputStream^>^ info, Windows::Foundation::AsyncStatus status)
                        {
                            if(status != Windows::Foundation::AsyncStatus::Completed)
                            {
                                _write.count = SOCKET_ERROR;
                                _write.error = info->ErrorCode.Value;
                            }
                            else
                            {
                                _write.count = 0;
                                _writer = ref new DataWriter(info->GetResults());
                            }
                            if(_mcastAddr.host != nullptr)
                            {
                                setMcastGroup(_fd, _mcastAddr, "");
                            }
                            _completedHandler(SocketOperationConnect);
                        });
                }
                else
                {
                    _write.count = 0;
                    _writer = ref new DataWriter(operation->GetResults());
                    if(_mcastAddr.host != nullptr)
                    {
                        setMcastGroup(_fd, _mcastAddr, "");
                    }
                }
            }
        }
        catch(Platform::Exception^ ex)
        {
            checkConnectErrorCode(__FILE__, __LINE__, ex->HResult, _addr.host);
        }
        return false;
    }
    else if(_state == StateNotConnected)
    {
        try
        {
            DatagramSocket^ fd = safe_cast<DatagramSocket^>(_fd);
            concurrency::create_task(fd->GetOutputStreamAsync(_peerAddr.host, _peerAddr.port)).then(
                [=,&buf](concurrency::task<IOutputStream^> task)
                {
                    try
                    {
                        DataWriter^ writer = ref new DataWriter(task.get());
                        writer->WriteBytes(ref new Array<unsigned char>(&*buf.i, static_cast<int>(buf.b.size())));
                        DataWriterStoreOperation^ operation = writer->StoreAsync();

                        Windows::Foundation::AsyncStatus status = operation->Status;
                        if(status == Windows::Foundation::AsyncStatus::Completed)
                        {
                            //
                            // NOTE: unlike other methods, it's important to modify _write.count
                            // _before_ calling checkIfErrorOrCompleted since this isn't called
                            // with the connection mutex but from a Windows thread pool thread.
                            // So we can't modify the _write structure after calling the 
                            // completed callback.
                            //
                            _write.count = operation->GetResults();
                            _completedHandler(SocketOperationWrite);
                        }
                        else if(status == Windows::Foundation::AsyncStatus::Started)
                        {
                            operation->Completed = _writeOperationCompletedHandler;
                        }
                        else
                        {
                            if(_state < StateConnected)
                            {
                                checkConnectErrorCode(__FILE__, __LINE__, operation->ErrorCode.Value, _addr.host);
                            }
                            else 
                            {
                                checkErrorCode(__FILE__, __LINE__, operation->ErrorCode.Value);
                            }
                        }
                    }
                    catch(Platform::Exception^ pex)
                    {
                        _write.count = SOCKET_ERROR;
                        _write.error = pex->HResult;
                        _completedHandler(SocketOperationWrite);
                    }
                });
        }
        catch(Platform::Exception^ ex)
        {
            checkErrorCode(__FILE__, __LINE__, ex->HResult);
        }
        return true;
    }
    else
    {
        try
        {
            _writer->WriteBytes(ref new Array<unsigned char>(&*buf.i, static_cast<int>(buf.b.size())));
            DataWriterStoreOperation^ operation = _writer->StoreAsync();
            if(checkIfErrorOrCompleted(SocketOperationWrite, operation))
            {
                _write.count = operation->GetResults();
            }
            else
            {
                operation->Completed = _writeOperationCompletedHandler;
            }
        }
        catch(Platform::Exception^ ex)
        {
            checkErrorCode(__FILE__, __LINE__, ex->HResult);
        }
        return true;
    }
#else
    _write.buf.len = static_cast<int>(buf.b.size());
    _write.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err;
    if(_state == StateConnected)
    {
        err = WSASend(_fd, &_write.buf, 1, &_write.count, 0, &_write, NULL);
    }
    else
    {
        socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
        if(_peerAddr.saStorage.ss_family == AF_INET)
        {
            len = sizeof(sockaddr_in);
        }
        else if(_peerAddr.saStorage.ss_family == AF_INET6)
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
        err = WSASendTo(_fd, &_write.buf, 1, &_write.count, 0, &_peerAddr.sa, 
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
#endif
    return true;
}

void
IceInternal::UdpTransceiver::finishWrite(Buffer& buf)
{
    if(_state < StateConnected)
    {
        return;
    }

    if(static_cast<int>(_write.count) == SOCKET_ERROR)
    {
#ifndef ICE_OS_WINRT
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
#else
        checkErrorCode(__FILE__, __LINE__, _write.error);
#endif
    }

    if(_instance->traceLevel() >= 3)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "sent " << _write.count << " bytes via " << _instance->protocol() << '\n' << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesSent(protocol(), static_cast<Int>(_write.count));
    }

    assert(_write.count == buf.b.size());
    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::startRead(Buffer& buf)
{
    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();
    assert(!buf.b.empty() && buf.i != buf.b.end());
#ifndef ICE_OS_WINRT
    _read.buf.len = packetSize;
    _read.buf.buf = reinterpret_cast<char*>(&*buf.i);
    int err;
    if(_state == StateConnected)
    {
        err = WSARecv(_fd, &_read.buf, 1, &_read.count, &_read.flags, &_read, NULL);
    }
    else
    {
        memset(&_readAddr.saStorage, 0, sizeof(struct sockaddr_storage));
        _readAddrLen = static_cast<socklen_t>(sizeof(sockaddr_storage));
        
        err = WSARecvFrom(_fd, &_read.buf, 1, &_read.count, &_read.flags, 
                          &_readAddr.sa, &_readAddrLen, &_read, NULL);
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
#else
    IceUtil::Mutex::Lock lock(_mutex);
    assert(!_readPending);
    if(!_received.empty())
    {
        _completedHandler(SocketOperationRead);
    }
    else
    {
        _readPending = true;
    }
#endif
}

void
IceInternal::UdpTransceiver::finishRead(Buffer& buf)
{
#ifdef ICE_OS_WINRT
    IceUtil::Mutex::Lock lock(_mutex);
    assert(!_readPending && (!_received.empty() || _fd == INVALID_SOCKET));
    if(_fd == INVALID_SOCKET)
    {
        return;
    }

    DatagramSocketMessageReceivedEventArgs^ args = _received.front();
    _received.pop_front();

    int ret;
    try
    {
        DataReader^ reader = args->GetDataReader();
        ret = min(static_cast<int>(reader->UnconsumedBufferLength), static_cast<int>(buf.b.size())); // Truncate received data if too large.
        
        Array<unsigned char>^ data = ref new Array<unsigned char>(ret);
        reader->ReadBytes(data);
        memcpy(&*buf.i, data->Data, ret);
        if(_state == StateNotConnected)
        {
            _peerAddr.host = args->RemoteAddress;
            _peerAddr.port = args->RemotePort;
        }
    }
    catch(Platform::Exception^ ex)
    {
        checkErrorCode(__FILE__, __LINE__, ex->HResult);
    }
#else
    if(static_cast<int>(_read.count) == SOCKET_ERROR)
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

    int ret = _read.count;
#endif

    if(_instance->traceLevel() >= 3)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "received " << ret << " bytes via " << _instance->protocol() << '\n' << toString();
    }

    if(_instance->stats())
    {
        _instance->stats()->bytesReceived(protocol(), static_cast<Int>(ret));
    }

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
    if(_fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    ostringstream s;
    if(_state == StateNotConnected)
    {
        Address localAddr;
        fdToLocalAddress(_fd, localAddr);
        s << "local address = " << addrToString(localAddr);
        if(isAddressValid(_peerAddr))
        {
            s << "\nremote address = " << addrToString(_peerAddr);
        }
    }
    else
    {
#ifndef ICE_OS_WINRT
        s << fdToString(_fd);
#else
        Address localAddr;
        fdToLocalAddress(_fd, localAddr);
        s << "local address = " << addrToString(localAddr);
        s << "\nremote address = " << addrToString(_addr);
#endif
    }

    if(isAddressValid(_mcastAddr))
    {
        s << "\nmulticast address = " + addrToString(_mcastAddr);
    }
    return s.str();
}

Ice::ConnectionInfoPtr
IceInternal::UdpTransceiver::getInfo() const
{
    Ice::UDPConnectionInfoPtr info = new Ice::UDPConnectionInfo();
#if defined(ICE_OS_WINRT)
    if(isMulticast(_addr) || isAddressValid(_mcastAddr))
    {
        info->remotePort = 0;
        info->localPort = 0;
        info->mcastPort = 0;
        return info;
    }
#endif
    if(_fd == INVALID_SOCKET)
    {
        return info;
    }

    if(_state == StateNotConnected)
    {
        Address localAddr;
        fdToLocalAddress(_fd, localAddr);
        addrToAddressAndPort(localAddr, info->localAddress, info->localPort);
        if(isAddressValid(_peerAddr))
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

    if(isAddressValid(_mcastAddr))
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


IceInternal::UdpTransceiver::UdpTransceiver(const ProtocolInstancePtr& instance, 
                                            const Address& addr,
#ifdef ICE_OS_WINRT
                                            const string&,
                                            int
#else
                                            const string& mcastInterface,
                                            int mcastTtl
#endif
                                            ) :
    _instance(instance),
    _incoming(false),
    _addr(addr),
    _state(StateNeedConnect)
#if defined(ICE_USE_IOCP)
    , _read(SocketOperationRead), 
    _write(SocketOperationWrite)
#elif defined(ICE_OS_WINRT)
    , _readPending(false)
#endif
{
    _fd = createSocket(true, _addr);
    setBufSize(_instance->properties());
    setBlock(_fd, false);

#ifndef ICE_OS_WINRT
    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
    _peerAddr.saStorage.ss_family = AF_UNSPEC; // Not initialized yet.

    //
    // NOTE: setting the multicast interface before performing the
    // connect is important for some OS such as OS X.
    //
    if(isMulticast(_addr))
    {
        if(mcastInterface.length() > 0)
        {
            setMcastInterface(_fd, mcastInterface, _addr);
        }
        if(mcastTtl != -1)
        {
            setMcastTtl(_fd, mcastTtl, _addr);
        }
    }

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
#else
    DatagramSocket^ socket = safe_cast<DatagramSocket^>(_fd);
    socket->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>(
        [=](DatagramSocket^ fd, DatagramSocketMessageReceivedEventArgs^ args)
        {
            this->appendMessage(args);
        });
#endif


#ifdef ICE_USE_IOCP
    //
    // On Windows when using IOCP, we must make sure that the socket is connected without 
    // blocking as there's no way to do a non-blocking datagram socket conection (ConnectEx
    // only supports connection oriented sockets). According to Microsoft documentation of
    // the connect() call, this should always be the case.
    //
    assert(_state == StateConnected);
#endif
}

IceInternal::UdpTransceiver::UdpTransceiver(const ProtocolInstancePtr& instance, const string& host, int port,
                                            const string& mcastInterface, bool connect) :
    _instance(instance),
    _incoming(true),
    _addr(getAddressForServer(host, port, instance->protocolSupport(), instance->preferIPv6())),
    _state(connect ? StateNeedConnect : StateNotConnected)
#ifdef ICE_OS_WINRT
    , _readPending(false)
#elif defined(ICE_USE_IOCP)
    , _read(SocketOperationRead), 
    _write(SocketOperationWrite)
#endif
{
    _fd = createServerSocket(true, _addr, instance->protocolSupport());
    setBufSize(instance->properties());
    setBlock(_fd, false);

#ifndef ICE_OS_WINRT
    memset(&_mcastAddr.saStorage, 0, sizeof(sockaddr_storage));
    memset(&_peerAddr.saStorage, 0, sizeof(sockaddr_storage));
    _peerAddr.saStorage.ss_family = AF_UNSPEC;
    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
#else
    DatagramSocket^ socket = safe_cast<DatagramSocket^>(_fd);
    socket->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>(
        [=](DatagramSocket^ fd, DatagramSocketMessageReceivedEventArgs^ args)
        {
            this->appendMessage(args);
        });
#endif

    if(_instance->traceLevel() >= 2)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "attempting to bind to " << _instance->protocol() << " socket " << addrToString(_addr);
    }

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
        const_cast<Address&>(_addr) = getAddressForServer("", port, getProtocolSupport(_addr), false);
#endif

        const_cast<Address&>(_addr) = doBind(_fd, _addr);
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
        const_cast<Address&>(_addr) = doBind(_fd, _addr);
    }

    if(_instance->traceLevel() >= 1)
    {
        Trace out(_instance->logger(), _instance->traceCategory());
        out << "starting to receive " << _instance->protocol() << " packets\n" << toString();

        vector<string> interfaces = 
            getHostsForEndpointExpand(inetAddrToString(_addr), _instance->protocolSupport(), true);
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
IceInternal::UdpTransceiver::setBufSize(const Ice::PropertiesPtr& properties)
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
        }
        else
        {
            direction = "send";
            prop = "Ice.UDP.SndSize";
            addr = &_sndSize;
            dfltSize = getSendBufferSize(_fd);
        }

        if(dfltSize <= 0)
        {
            dfltSize = _maxPacketSize;
        }
        *addr = dfltSize;

        //
        // Get property for buffer size and check for sanity.
        //
        Int sizeRequested = properties->getPropertyAsIntWithDefault(prop, dfltSize);
        if(sizeRequested < (_udpOverhead + headerSize))
        {
            Warning out(_instance->logger());
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
            if(*addr == 0) // set buffer size not supported.
            {
                *addr = sizeRequested;
            }
            else if(*addr < sizeRequested)
            {
                Warning out(_instance->logger());
                out << "UDP " << direction << " buffer size: requested size of "
                    << sizeRequested << " adjusted to " << *addr;
            }
        }
    }
}

#ifdef ICE_OS_WINRT
bool
IceInternal::UdpTransceiver::checkIfErrorOrCompleted(SocketOperation op, IAsyncInfo^ info)
{
    //
    // NOTE: It's important to only check for info->Status once as it
    // might change during the checks below (the Status can be changed
    // by the Windows thread pool concurrently).
    //
    // We consider that a canceled async status is the same as an
    // error. A canceled async status can occur if there's a timeout
    // and the socket is closed.
    //
    Windows::Foundation::AsyncStatus status = info->Status;
    if(status == Windows::Foundation::AsyncStatus::Completed)
    {
        _completedHandler(op);
        return true;
    }
    else if(status == Windows::Foundation::AsyncStatus::Started)
    {
        return false;
    }
    else
    {
        if(_state < StateConnected)
        {
            checkConnectErrorCode(__FILE__, __LINE__, info->ErrorCode.Value, _addr.host);
        }
        else 
        {
            checkErrorCode(__FILE__, __LINE__, info->ErrorCode.Value);
        }
        return true; // Prevent compiler warning.
    }
}

void
IceInternal::UdpTransceiver::appendMessage(DatagramSocketMessageReceivedEventArgs^ args)
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(_fd == INVALID_SOCKET) // Transceiver was closed.
    {
        return;
    }
    _received.push_back(args);

    //
    // If the transceiver is waiting to read a message, notify the
    // selector that the transceiver is ready for read. This will in
    // turn cause finishRead() to be called. If the transceiver is
    // ready for read, we just queue the message. It will be picked
    // up by startRead() when it is invoked.
    //
    if(_readPending)
    {
        _completedHandler(SocketOperationRead);
        _readPending = false;
    }
}
#endif

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
