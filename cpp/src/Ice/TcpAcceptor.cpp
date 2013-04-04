// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpAcceptor.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>
#include <IceUtil/StringUtil.h>

#ifdef ICE_USE_IOCP
#  include <Mswsock.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::TcpAcceptor::getNativeInfo()
{
    return this;
}

#ifdef ICE_USE_IOCP
AsyncInfo*
#  ifndef NDEBUG
IceInternal::TcpAcceptor::getAsyncInfo(SocketOperation op)
#  else
IceInternal::TcpAcceptor::getAsyncInfo(SocketOperation)
#  endif
{
    assert(op == SocketOperationRead);
    return &_info;
}
#endif

void
IceInternal::TcpAcceptor::close()
{
    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "stopping to accept tcp connections at " << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceInternal::TcpAcceptor::listen()
{
    try
    {
        doListen(_fd, _backlog);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
    
    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "listening for tcp connections at " << toString();

        vector<string> interfaces = 
            getHostsForEndpointExpand(inetAddrToString(_addr), _instance->protocolSupport(), true);
        if(!interfaces.empty())
        {
            out << "\nlocal interfaces: ";
            out << IceUtilInternal::joinString(interfaces, ", ");
        }
    }
}

#ifdef ICE_USE_IOCP
void
IceInternal::TcpAcceptor::startAccept()
{
    LPFN_ACCEPTEX AcceptEx = NULL; // a pointer to the 'AcceptEx()' function
    GUID GuidAcceptEx = WSAID_ACCEPTEX; // The Guid
    DWORD dwBytes;
    if(WSAIoctl(_fd, 
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidAcceptEx,
                sizeof(GuidAcceptEx),
                &AcceptEx,
                sizeof(AcceptEx),
                &dwBytes,
                NULL, 
                NULL) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }        

    assert(_acceptFd == INVALID_SOCKET);
    _acceptFd = createSocket(false, _addr);
    const int sz = static_cast<int>(_acceptBuf.size() / 2);
    if(!AcceptEx(_fd, _acceptFd, &_acceptBuf[0], 0, sz, sz, &_info.count, &_info))
    {
        if(!wouldBlock())
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }
}

void
IceInternal::TcpAcceptor::finishAccept()
{
    if(static_cast<int>(_info.count) == SOCKET_ERROR || _fd == INVALID_SOCKET)
    {
        closeSocketNoThrow(_acceptFd);
        _acceptFd = INVALID_SOCKET;
        _acceptError = _info.error;
    }
}

#endif

TransceiverPtr
IceInternal::TcpAcceptor::accept()
{
#ifdef ICE_USE_IOCP
    if(_acceptFd == INVALID_SOCKET)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = _acceptError;
        throw ex;        
    }
    if(setsockopt(_acceptFd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&_acceptFd, sizeof(_acceptFd)) == 
       SOCKET_ERROR)
    {
        closeSocketNoThrow(_acceptFd);
        _acceptFd = INVALID_SOCKET;
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;        
    }

    SOCKET fd = _acceptFd;
    _acceptFd = INVALID_SOCKET;
#else
    SOCKET fd = doAccept(_fd);
#endif

    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "accepted tcp connection\n" << fdToString(fd);
    }
    return new TcpTransceiver(_instance, fd);
}

string
IceInternal::TcpAcceptor::toString() const
{
    return addrToString(_addr);
}

int
IceInternal::TcpAcceptor::effectivePort() const
{
    return getPort(_addr);
}

IceInternal::TcpAcceptor::TcpAcceptor(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger),
    _addr(getAddressForServer(host, port, _instance->protocolSupport(), instance->preferIPv6()))
#ifdef ICE_USE_IOCP
    , _acceptFd(INVALID_SOCKET),
    _info(SocketOperationRead)
#endif
{
#ifdef SOMAXCONN
    _backlog = instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.TCP.Backlog", SOMAXCONN);
#else
    _backlog = instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);
#endif

    _fd = createServerSocket(false, _addr, instance->protocolSupport());
#ifdef ICE_USE_IOCP
    _acceptBuf.resize((sizeof(sockaddr_storage) + 16) * 2);
#endif

    setBlock(_fd, false);
    setTcpBufSize(_fd, _instance->initializationData().properties, _logger);
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
    
    if(_traceLevels->network >= 2)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "attempting to bind to tcp socket " << toString();
    }
    const_cast<Address&>(_addr) = doBind(_fd, _addr);
}

IceInternal::TcpAcceptor::~TcpAcceptor()
{
    assert(_fd == INVALID_SOCKET);
#ifdef ICE_USE_IOCP
    assert(_acceptFd == INVALID_SOCKET);
#endif
}

