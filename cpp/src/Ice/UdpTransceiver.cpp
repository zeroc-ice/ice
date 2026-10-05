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

#if defined(ICE_USE_NETWORK_FRAMEWORK)
#    include "apple/ObjectRef.h"

#    include <dispatch/dispatch.h>
#    include <mutex>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#if defined(ICE_USE_NETWORK_FRAMEWORK)

//
// On Apple platforms the thread pool is driven by the completion-based selector: an operation is started with
// startRead/startWrite and posts one completion through the NativeInfo once it is done. UDP keeps the BSD socket of
// the other platforms and uses a small adapter to complete the operations: the outstanding receive or send is
// attempted right away and, when the socket is not ready, a dispatch source waits for its readiness and performs
// the operation from its event handler. A completion is only posted once a datagram was received or sent, or a
// terminal error occurred, never for readiness alone. Nothing is received before Ice asks for it, so the kernel
// socket buffer provides the buffering and drops the datagrams a server does not read.
//
// The sources and their handlers only use this state (never the transceiver): they can outlive it. The socket is
// closed by the cancellation handler of the last source, once their event handlers can no longer run, as dispatch
// requires; without sources, close() closes it directly.
//
struct IceInternal::UdpAsyncState
{
    UdpAsyncState(SOCKET socket, NativeInfoPtr info)
        : fd(socket),
          nativeInfo(std::move(info)),
          queue(DispatchRef<dispatch_queue_t>::adopt(dispatch_queue_create("com.zeroc.ice.udp", DISPATCH_QUEUE_SERIAL)))
    {
    }

    UdpAsyncState(const UdpAsyncState&) = delete;
    UdpAsyncState& operator=(const UdpAsyncState&) = delete;

    std::mutex mutex;
    SOCKET fd;
    NativeInfoPtr nativeInfo;

    // The sources retain the queue while they are alive. The state owns the queue so that it is released even when
    // the transceiver constructor throws after creating the state.
    const DispatchRef<dispatch_queue_t> queue;

    // The wrappers only release the sources: cancelSource resumes and cancels a source first, and the cancellation
    // handler of the last source closes the socket.
    DispatchRef<dispatch_source_t> readSource;
    bool readArmed{false};
    DispatchRef<dispatch_source_t> writeSource;
    bool writeArmed{false};
    int activeSources{0};
    bool closed{false};

    // Outstanding connect: the thread pool expects a completion for SocketOperationConnect once the socket is
    // writable.
    bool connectPending{false};

    // Outstanding receive into [readData, readData + readSize): readCount bytes received, or readError.
    bool readPending{false};
    char* readData{nullptr};
    size_t readSize{0};
    ssize_t readCount{0};
    int readError{0};
    bool readFrom{false}; // recvfrom (server) rather than recv (connected client socket).
    Address readAddr;
    socklen_t readAddrLen{0};

    // Outstanding send of the datagram [writeData, writeData + writeSize).
    bool writePending{false};
    const char* writeData{nullptr};
    size_t writeSize{0};
    int writeError{0};
    bool writeTo{false}; // sendto the peer address (server) rather than send (connected client socket).
    Address writeAddr;
    socklen_t writeAddrLen{0};
};

namespace
{
    // The functions below run with the state mutex held, on an Ice thread or on the dispatch queue.

    // Attempts the outstanding receive. Returns false when the socket is not readable yet.
    bool tryRead(UdpAsyncState& state)
    {
        if (!state.readPending)
        {
            return true;
        }

        if (!state.closed)
        {
            while (true)
            {
                ssize_t ret;
                if (state.readFrom)
                {
                    memset(&state.readAddr.saStorage, 0, sizeof(sockaddr_storage));
                    state.readAddrLen = static_cast<socklen_t>(sizeof(sockaddr_storage));
                    ret = recvfrom(state.fd, state.readData, state.readSize, 0, &state.readAddr.sa, &state.readAddrLen);
                }
                else
                {
                    ret = recv(state.fd, state.readData, state.readSize, 0);
                }

                if (ret == SOCKET_ERROR)
                {
                    if (interrupted())
                    {
                        continue;
                    }
                    if (wouldBlock())
                    {
                        return false;
                    }
                    state.readError = getSocketErrno();
                }
                else
                {
                    state.readCount = ret;
                }
                break;
            }
        }
        else
        {
            state.readError = ECANCELED;
        }

        state.readPending = false;
        state.nativeInfo->completed(SocketOperationRead);
        return true;
    }

    // Attempts the outstanding send. Returns false when the socket is not writable yet.
    bool tryWrite(UdpAsyncState& state)
    {
        if (!state.writePending)
        {
            return true;
        }

        if (!state.closed)
        {
            while (true)
            {
                ssize_t ret;
                if (state.writeTo)
                {
                    ret =
                        sendto(state.fd, state.writeData, state.writeSize, 0, &state.writeAddr.sa, state.writeAddrLen);
                }
                else
                {
                    ret = send(state.fd, state.writeData, state.writeSize, 0);
                }

                if (ret == SOCKET_ERROR)
                {
                    if (interrupted())
                    {
                        continue;
                    }
                    if (wouldBlock())
                    {
                        return false;
                    }
                    // ENOBUFS means the local send buffer is momentarily full, a routine transient condition on
                    // macOS during bursts: drop the datagram like write() does (UDP is best-effort).
                    if (!noBuffers())
                    {
                        state.writeError = getSocketErrno();
                    }
                }
                break;
            }
        }
        else
        {
            state.writeError = ECANCELED;
        }

        state.writePending = false;
        state.nativeInfo->completed(SocketOperationWrite);
        return true;
    }

    void completeConnect(UdpAsyncState& state)
    {
        if (state.connectPending)
        {
            state.connectPending = false;
            state.nativeInfo->completed(SocketOperationConnect);
        }
    }

    // Suspends a source from its own event handler; the suspension takes effect once the handler returns.
    void disarm(dispatch_source_t source, bool& armed)
    {
        if (source && armed)
        {
            dispatch_suspend(source);
            armed = false;
        }
    }

    void cancelHandler(const shared_ptr<UdpAsyncState>& state)
    {
        // The socket is closed once the last source is cancelled: its event handler can no longer run.

        lock_guard lock(state->mutex);
        if (--state->activeSources == 0)
        {
            closeSocketNoThrow(state->fd);
            state->fd = INVALID_SOCKET;
        }
    }

    // Waits for the socket to be readable, then performs the outstanding receive. The state is taken by value:
    // the blocks capture a copy of this parameter, and must not capture a reference to the transceiver's member.
    void armRead(shared_ptr<UdpAsyncState> state) // NOLINT(performance-unnecessary-value-param)
    {
        if (!state->readSource)
        {
            state->readSource = DispatchRef<dispatch_source_t>::adopt(dispatch_source_create(
                DISPATCH_SOURCE_TYPE_READ,
                static_cast<uintptr_t>(state->fd),
                0,
                state->queue.get()));
            ++state->activeSources;
            dispatch_source_set_event_handler(state->readSource.get(), ^{
              lock_guard lock(state->mutex);
              if (state->closed)
              {
                  // Cancellation lets a handler that already started finish: close() completed the operations and
                  // released the sources, nothing is left to do.
                  return;
              }
              if (tryRead(*state))
              {
                  disarm(state->readSource.get(), state->readArmed);
              }
            });
            dispatch_source_set_cancel_handler(state->readSource.get(), ^{
              cancelHandler(state);
            });
        }
        if (!state->readArmed)
        {
            dispatch_resume(state->readSource.get());
            state->readArmed = true;
        }
    }

    // Waits for the socket to be writable, then completes the connect or performs the outstanding send. The state
    // is taken by value, see armRead.
    void armWrite(shared_ptr<UdpAsyncState> state) // NOLINT(performance-unnecessary-value-param)
    {
        if (!state->writeSource)
        {
            state->writeSource = DispatchRef<dispatch_source_t>::adopt(dispatch_source_create(
                DISPATCH_SOURCE_TYPE_WRITE,
                static_cast<uintptr_t>(state->fd),
                0,
                state->queue.get()));
            ++state->activeSources;
            dispatch_source_set_event_handler(state->writeSource.get(), ^{
              lock_guard lock(state->mutex);
              if (state->closed)
              {
                  return; // See the read event handler.
              }
              completeConnect(*state);
              if (tryWrite(*state))
              {
                  disarm(state->writeSource.get(), state->writeArmed);
              }
            });
            dispatch_source_set_cancel_handler(state->writeSource.get(), ^{
              cancelHandler(state);
            });
        }
        if (!state->writeArmed)
        {
            dispatch_resume(state->writeSource.get());
            state->writeArmed = true;
        }
    }

    // Cancels a source, then releases it; a suspended source must be resumed first, dispatch cannot cancel or
    // release it otherwise. Releasing alone would not stop the source: its cancellation is a separate operation.
    void cancelSource(DispatchRef<dispatch_source_t>& source, bool& armed)
    {
        if (source)
        {
            if (!armed)
            {
                dispatch_resume(source.get());
                armed = true;
            }
            dispatch_source_cancel(source.get());
            source.reset();
        }
    }

    // Completes the outstanding operations (the thread pool waits for their completions before it finishes the
    // connection) and closes the socket: through the cancellation handler of the last source once the event
    // handlers can no longer run, or directly when there is no source. Idempotent.
    void closeState(const shared_ptr<UdpAsyncState>& state)
    {
        lock_guard lock(state->mutex);
        if (state->closed)
        {
            return;
        }
        state->closed = true;

        completeConnect(*state);
        tryRead(*state);
        tryWrite(*state);

        if (state->activeSources > 0)
        {
            cancelSource(state->readSource, state->readArmed);
            cancelSource(state->writeSource, state->writeArmed);
        }
        else if (state->fd != INVALID_SOCKET)
        {
            closeSocketNoThrow(state->fd);
            state->fd = INVALID_SOCKET;
        }
    }

    // Unlike the Network.cpp helpers, these do not close the socket when the option cannot be set or read: the
    // socket may have dispatch sources and its closure belongs to closeState.
    void setSocketBufferSize(SOCKET fd, int option, int size)
    {
        if (setsockopt(fd, SOL_SOCKET, option, &size, static_cast<socklen_t>(sizeof(int))) == SOCKET_ERROR)
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    int getSocketBufferSize(SOCKET fd, int option)
    {
        int size = option == SO_RCVBUF ? getRecvBufferSizeNoThrow(fd) : getSendBufferSizeNoThrow(fd);
        if (size == 0)
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
        return size;
    }

    socklen_t addressLength(const Address& addr)
    {
        if (addr.saStorage.ss_family == AF_INET)
        {
            return static_cast<socklen_t>(sizeof(sockaddr_in));
        }
        else if (addr.saStorage.ss_family == AF_INET6)
        {
            return static_cast<socklen_t>(sizeof(sockaddr_in6));
        }
        return 0; // No peer has sent a datagram yet.
    }
}

#endif

NativeInfoPtr
IceInternal::UdpTransceiver::getNativeInfo()
{
#if defined(ICE_USE_NETWORK_FRAMEWORK)
    return _nativeInfo;
#else
    return shared_from_this();
#endif
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
#if defined(ICE_USE_NETWORK_FRAMEWORK)
        // The thread pool waits for the Connect completion: post it once the socket is writable.
        {
            lock_guard lock(_async->mutex);
            _async->connectPending = true;
            armWrite(_async);
        }
#endif
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
    // _fd can be INVALID_SOCKET when bind failed: the bind/setup helper that threw already closed the socket, and
    // bind()'s catch reset _fd.
    if (_fd != INVALID_SOCKET)
    {
#if defined(ICE_USE_NETWORK_FRAMEWORK)
        closeState(_async);
#else
        closeSocketNoThrow(_fd);
#endif
        _fd = INVALID_SOCKET;
    }
}

EndpointIPtr
IceInternal::UdpTransceiver::bind()
{
    try
    {
        if (isMulticast(_addr))
        {
            // Set SO_REUSEADDR socket option to allow multiple sockets to bind to the same multicast address.
            setReuseAddress(_fd, true);
            _mcastAddr = _addr;

#ifdef _WIN32
            // Windows does not allow binding to the mcast address itself so we bind to INADDR_ANY instead.
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
            const_cast<Address&>(_addr) = doBind(_fd, _addr);
        }
    }
    catch (...)
    {
        // setReuseAddress/doBind/setMcastGroup close the socket before throwing, so reset _fd to avoid a double
        // close when close() runs during cleanup. Mirrors TcpAcceptor::listen.
        _fd = INVALID_SOCKET;
        throw;
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

#ifndef _WIN32
        // ENOBUFS means the local send buffer is momentarily full. This is routine on macOS/BSD during bursts
        // and is a transient local condition rather than a connection failure. UDP is best-effort, so drop this
        // datagram instead of throwing (throwing would needlessly close the connection). Limited to non-Windows:
        // there noBuffers() also matches WSAEFAULT, which must not be silently dropped.
        if (noBuffers())
        {
            buf.i = buf.b.end();
            return SocketOperationNone;
        }
#endif

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
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno(), addrToString(_peerAddr));
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
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno(), addrToString(_peerAddr));
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
            throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno(), addrToString(_peerAddr));
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
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno(), addrToString(_peerAddr));
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
                throw ConnectionLostException(__FILE__, __LINE__, getSocketErrno(), addrToString(_peerAddr));
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
#elif defined(ICE_USE_NETWORK_FRAMEWORK)
bool
IceInternal::UdpTransceiver::startWrite(Buffer& buf)
{
    assert(buf.i == buf.b.begin());
    assert(_fd != INVALID_SOCKET && _state >= StateConnected);

    // The caller is supposed to check the send size before by calling checkSendSize
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

    lock_guard lock(_async->mutex);
    _async->writeTo = _state != StateConnected;
    if (_async->writeTo)
    {
        _async->writeAddr = _peerAddr;
        _async->writeAddrLen = addressLength(_peerAddr);
        if (_async->writeAddrLen == 0)
        {
            // No peer has sent a datagram yet.
            throw SocketException(__FILE__, __LINE__, 0);
        }
    }
    _async->writePending = true;
    _async->writeData = reinterpret_cast<const char*>(&*buf.i);
    _async->writeSize = buf.b.size();
    _async->writeError = 0;

    if (!tryWrite(*_async))
    {
        armWrite(_async);
    }
    return true; // The datagram is sent whole by this operation.
}

void
IceInternal::UdpTransceiver::finishWrite(Buffer& buf)
{
    if (_fd == INVALID_SOCKET || _state < StateConnected)
    {
        return;
    }

    lock_guard lock(_async->mutex);
    if (_async->writeError != 0)
    {
        errno = _async->writeError;
        if (errno == ECANCELED || connectionLost())
        {
            throw ConnectionLostException(__FILE__, __LINE__, errno, addrToString(_peerAddr));
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }

    buf.i = buf.b.end();
}

void
IceInternal::UdpTransceiver::startRead(Buffer& buf)
{
    assert(_fd != INVALID_SOCKET);

    const auto packetSize = static_cast<size_t>(min(_maxPacketSize, _rcvSize - _udpOverhead));
    buf.b.resize(packetSize);
    buf.i = buf.b.begin();

    lock_guard lock(_async->mutex);
    _async->readPending = true;
    _async->readData = reinterpret_cast<char*>(&*buf.i);
    _async->readSize = packetSize;
    _async->readCount = 0;
    _async->readError = 0;
    _async->readFrom = _state != StateConnected;

    if (!tryRead(*_async))
    {
        armRead(_async);
    }
}

void
IceInternal::UdpTransceiver::finishRead(Buffer& buf)
{
    lock_guard lock(_async->mutex);
    ssize_t count = _async->readCount;
    if (_async->readError != 0)
    {
        errno = _async->readError;
        if (recvTruncated())
        {
            // The message was truncated and the whole buffer is filled. We ignore
            // this error here, it will be detected at the connection level when
            // the Ice message size is checked against the buffer size.
            count = static_cast<ssize_t>(buf.b.size());
        }
        else if (errno == ECANCELED || connectionLost())
        {
            throw ConnectionLostException(__FILE__, __LINE__, errno, addrToString(_peerAddr));
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }

    if (_state == StateNotConnected)
    {
        _peerAddr = _async->readAddr;
    }

    buf.b.resize(static_cast<size_t>(count));
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
    //
    // The maximum packetSize is either the maximum allowable UDP packet size, or
    // the UDP send buffer size (which ever is smaller).
    //
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if (packetSize < static_cast<int>(buf.b.size()))
    {
        throw DatagramLimitException(__FILE__, __LINE__);
    }
}

//
// Set UDP receive and send buffer sizes.
//
void
IceInternal::UdpTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    assert(_fd != INVALID_SOCKET);

#if defined(ICE_USE_NETWORK_FRAMEWORK)
    // The socket may have dispatch sources: the helpers must not close it on failure (see setSocketBufferSize).
    auto getRecvBufferSize = [](SOCKET fd) { return getSocketBufferSize(fd, SO_RCVBUF); };
    auto setRecvBufferSize = [](SOCKET fd, int size) { setSocketBufferSize(fd, SO_RCVBUF, size); };
    auto getSendBufferSize = [](SOCKET fd) { return getSocketBufferSize(fd, SO_SNDBUF); };
    auto setSendBufferSize = [](SOCKET fd, int size) { setSocketBufferSize(fd, SO_SNDBUF, size); };
#endif

    try
    {
        // The default size is the size currently configured on the socket. We don't set the buffer size when the
        // requested size matches the default: re-setting it would needlessly grow the buffer on systems (e.g. Linux)
        // where the kernel doubles the value on each set.

        int rcvDefault = getRecvBufferSize(_fd);
        rcvSize = adjustBufferSize(rcvSize, rcvDefault, "Ice.UDP.RcvSize");
        if (rcvSize == rcvDefault)
        {
            _rcvSize = rcvDefault;
        }
        else
        {
            // The kernel silently adjusts the size to an acceptable value, so read it back to get the size actually
            // set.
            setRecvBufferSize(_fd, rcvSize);
            _rcvSize = getRecvBufferSize(_fd);
            if (_rcvSize < rcvSize)
            {
                // The kernel reduced the requested size; warn unless we already warned for this size.
                BufSizeWarnInfo winfo = _instance->getBufSizeWarn(UDPEndpointType);
                if (!winfo.rcvWarn || winfo.rcvSize != rcvSize)
                {
                    Warning out(_instance->logger());
                    out << "UDP receive buffer size: requested size of " << rcvSize << " adjusted to " << _rcvSize;
                    _instance->setRcvBufSizeWarn(UDPEndpointType, rcvSize);
                }
            }
        }

        int sndDefault = getSendBufferSize(_fd);
        sndSize = adjustBufferSize(sndSize, sndDefault, "Ice.UDP.SndSize");
        if (sndSize == sndDefault)
        {
            _sndSize = sndDefault;
        }
        else
        {
            setSendBufferSize(_fd, sndSize);
            _sndSize = getSendBufferSize(_fd);
            if (_sndSize < sndSize)
            {
                BufSizeWarnInfo winfo = _instance->getBufSizeWarn(UDPEndpointType);
                if (!winfo.sndWarn || winfo.sndSize != sndSize)
                {
                    Warning out(_instance->logger());
                    out << "UDP send buffer size: requested size of " << sndSize << " adjusted to " << _sndSize;
                    _instance->setSndBufSizeWarn(UDPEndpointType, sndSize);
                }
            }
        }
    }
    catch (const SocketException&)
    {
#if defined(ICE_USE_NETWORK_FRAMEWORK)
        // The socket is still open: complete the outstanding operations for the thread pool and close the socket
        // through its sources, if any. The connection is closed by the caller with this exception.
        closeState(_async);
        _fd = INVALID_SOCKET;
#else
        // The failing call closed the fd.
        clearFd();
#endif
        throw;
    }
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
    int rcvSize = _instance->properties()->getIcePropertyAsInt("Ice.UDP.RcvSize");
    int sndSize = _instance->properties()->getIcePropertyAsInt("Ice.UDP.SndSize");
    _fd = createSocket(true, _addr);
#if defined(ICE_USE_NETWORK_FRAMEWORK)
    _nativeInfo = make_shared<NativeInfo>(INVALID_SOCKET);
    _async = make_shared<UdpAsyncState>(_fd, _nativeInfo);
#endif

    // Sets _rcvSize and _sndSize:
    setBufferSize(rcvSize, sndSize);
    assert(_rcvSize >= _udpOverhead + headerSize);
    assert(_sndSize >= _udpOverhead + headerSize);

    setBlock(_fd, false);

    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
    _peerAddr.saStorage.ss_family = AF_UNSPEC; // Not initialized yet.

    //
    // NOTE: setting the multicast interface before performing the
    // connect is important for some OS such as macOS.
    //
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

    //
    // In general, connecting a datagram socket should be non-blocking as this just sets up
    // the default destination address for the socket. However, on some OS, connect sometimes
    // returns EWOULDBLOCK. If that's the case, we keep the state as StateNeedConnect. This
    // will make sure the transceiver is notified when the socket is ready for sending (see
    // the initialize() implementation).
    //
    if (doConnect(_fd, _addr, sourceAddr))
    {
        _state = StateConnected;
    }

#ifdef ICE_USE_IOCP
    //
    // On Windows when using IOCP, we must make sure that the socket is connected without
    // blocking as there's no way to do a non-blocking datagram socket connection (ConnectEx
    // only supports connection oriented sockets). According to Microsoft documentation of
    // the connect() call, this should always be the case.
    //
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
    int rcvSize = _instance->properties()->getIcePropertyAsInt("Ice.UDP.RcvSize");
    int sndSize = _instance->properties()->getIcePropertyAsInt("Ice.UDP.SndSize");
    _fd = createServerSocket(true, _addr, instance->protocolSupport());
#if defined(ICE_USE_NETWORK_FRAMEWORK)
    _nativeInfo = make_shared<NativeInfo>(INVALID_SOCKET);
    _async = make_shared<UdpAsyncState>(_fd, _nativeInfo);
#endif

    // Sets _rcvSize and _sndSize:
    setBufferSize(rcvSize, sndSize);
    assert(_rcvSize >= _udpOverhead + headerSize);
    assert(_sndSize >= _udpOverhead + headerSize);

    setBlock(_fd, false);

    memset(&_mcastAddr.saStorage, 0, sizeof(sockaddr_storage));
    memset(&_peerAddr.saStorage, 0, sizeof(sockaddr_storage));
    _peerAddr.saStorage.ss_family = AF_UNSPEC;
    _mcastAddr.saStorage.ss_family = AF_UNSPEC;
}

IceInternal::UdpTransceiver::~UdpTransceiver() { assert(_fd == INVALID_SOCKET); }

int
IceInternal::UdpTransceiver::adjustBufferSize(int sizeRequested, int defaultSize, string_view prop)
{
    if (sizeRequested == 0)
    {
        // 0 (the property default) means we want the default size.
        return defaultSize;
    }

    if (sizeRequested < _udpOverhead + headerSize)
    {
        Warning out(_instance->logger());
        out << "Invalid " << prop << " value of " << sizeRequested << " adjusted to " << defaultSize;
        return defaultSize;
    }

    return sizeRequested;
}

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::UdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::UdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;
