//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Config.h>

#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0

#include <Ice/TcpAcceptor.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/TcpEndpointI.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/StreamSocket.h>
#include <IceUtil/StringUtil.h>

#if defined(ICE_USE_IOCP)
#  include <Mswsock.h>
#endif

//
// Use the system default for the listen() backlog or 511 if not defined.
//
#ifndef SOMAXCONN
#  define SOMAXCONN 511
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(TcpAcceptor* p) { return p; }

NativeInfoPtr
IceInternal::TcpAcceptor::getNativeInfo()
{
    return this;
}

void
IceInternal::TcpAcceptor::close()
{
#if defined(ICE_USE_IOCP)
    if(_acceptFd != INVALID_SOCKET)
    {
        closeSocketNoThrow(_acceptFd);
        _acceptFd = INVALID_SOCKET;
    }
#endif
    if(_fd != INVALID_SOCKET)
    {
        closeSocketNoThrow(_fd);
        _fd = INVALID_SOCKET;
    }
}

EndpointIPtr
IceInternal::TcpAcceptor::listen()
{
    try
    {
        const_cast<Address&>(_addr) = doBind(_fd, _addr);
        doListen(_fd, _backlog);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
    _endpoint = _endpoint->endpoint(this);
    return _endpoint;
}

#if defined(ICE_USE_IOCP)

AsyncInfo*
IceInternal::TcpAcceptor::getAsyncInfo(SocketOperation)
{
    return &_info;
}

void
IceInternal::TcpAcceptor::startAccept()
{
    LPFN_ACCEPTEX AcceptEx = ICE_NULLPTR; // a pointer to the 'AcceptEx()' function
    GUID GuidAcceptEx = WSAID_ACCEPTEX; // The Guid
    DWORD dwBytes;
    if(WSAIoctl(_fd,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidAcceptEx,
                sizeof(GuidAcceptEx),
                &AcceptEx,
                sizeof(AcceptEx),
                &dwBytes,
                ICE_NULLPTR,
                ICE_NULLPTR) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    assert(_acceptFd == INVALID_SOCKET);
    _acceptFd = createSocket(false, _addr);
    const int sz = static_cast<int>(_acceptBuf.size() / 2);
    _info.error = ERROR_SUCCESS;
    if(!AcceptEx(_fd, _acceptFd, &_acceptBuf[0], 0, sz, sz, &_info.count, &_info))
    {
        if(!wouldBlock())
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
}

void
IceInternal::TcpAcceptor::finishAccept()
{
    if(_info.error != ERROR_SUCCESS || _fd == INVALID_SOCKET)
    {
        closeSocketNoThrow(_acceptFd);
        _acceptFd = INVALID_SOCKET;
        _acceptError = _info.error;
    }
}

TransceiverPtr
IceInternal::TcpAcceptor::accept()
{
    if(_acceptFd == INVALID_SOCKET)
    {
        throw SocketException(__FILE__, __LINE__, _acceptError);
    }
    if(setsockopt(_acceptFd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_acceptFd, sizeof(_acceptFd)) ==
       SOCKET_ERROR)
    {
        closeSocketNoThrow(_acceptFd);
        _acceptFd = INVALID_SOCKET;
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    SOCKET fd = _acceptFd;
    _acceptFd = INVALID_SOCKET;
    return new TcpTransceiver(_instance, new StreamSocket(_instance, fd));
}

#else

TransceiverPtr
IceInternal::TcpAcceptor::accept()
{
    return new TcpTransceiver(_instance, new StreamSocket(_instance, doAccept(_fd)));
}

#endif

string
IceInternal::TcpAcceptor::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::TcpAcceptor::toString() const
{
    return addrToString(_addr);
}

string
IceInternal::TcpAcceptor::toDetailedString() const
{
    ostringstream os;
    os << "local address = " << toString();
    vector<string> intfs = getHostsForEndpointExpand(inetAddrToString(_addr), _instance->protocolSupport(), true);
    if(!intfs.empty())
    {
        os << "\nlocal interfaces = ";
        os << IceUtilInternal::joinString(intfs, ", ");
    }
    return os.str();
}

int
IceInternal::TcpAcceptor::effectivePort() const
{
    return getPort(_addr);
}

IceInternal::TcpAcceptor::TcpAcceptor(const TcpEndpointIPtr& endpoint,
                                      const ProtocolInstancePtr& instance,
                                      const string& host,
                                      int port) :
    _endpoint(endpoint),
    _instance(instance),
    _addr(getAddressForServer(host, port, _instance->protocolSupport(), instance->preferIPv6(), true))
#ifdef ICE_USE_IOCP
    , _acceptFd(INVALID_SOCKET), _info(SocketOperationRead)
#endif
{
    _backlog = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.Backlog", SOMAXCONN);
    _fd = createServerSocket(false, _addr, instance->protocolSupport());

#ifdef ICE_USE_IOCP
    _acceptBuf.resize((sizeof(sockaddr_storage) + 16) * 2);
#endif

    setBlock(_fd, false);
    setTcpBufSize(_fd, _instance);

#ifndef _WIN32
    //
    // Enable SO_REUSEADDR on Unix platforms to allow re-using the
    // socket even if it's in the TIME_WAIT state. On Windows,
    // this doesn't appear to be necessary and enabling
    // SO_REUSEADDR would actually not be a good thing since it
    // allows a second process to bind to an address even it's
    // already bound by another process.
    //
    // TODO: using SO_EXCLUSIVEADDRUSE on Windows would probably
    // be better but it's only supported by recent Windows
    // versions (XP SP2, Windows Server 2003).
    //
    setReuseAddress(_fd, true);
#endif
}

IceInternal::TcpAcceptor::~TcpAcceptor()
{
    assert(_fd == INVALID_SOCKET);
#ifdef ICE_USE_IOCP
    assert(_acceptFd == INVALID_SOCKET);
#endif
}

#endif
