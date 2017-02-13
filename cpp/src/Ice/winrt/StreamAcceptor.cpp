// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/winrt/StreamAcceptor.h>
#include <Ice/winrt/StreamTransceiver.h>
#include <Ice/winrt/StreamEndpointI.h>

#include <Ice/ProtocolInstance.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

IceUtil::Shared* IceInternal::upCast(StreamAcceptor* p) { return p; }

NativeInfoPtr
IceInternal::StreamAcceptor::getNativeInfo()
{
    return this;
}

void
IceInternal::StreamAcceptor::setCompletedHandler(SocketOperationCompletedHandler^ handler)
{
    _completedHandler = handler;
}

void
IceInternal::StreamAcceptor::close()
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(_acceptPending)
    {
        assert(_accepted.empty());
        _completedHandler(SocketOperationRead);
        _acceptPending = false;
    }
    else if(!_accepted.empty())
    {
        for(deque<StreamSocket^>::const_iterator p = _accepted.begin(); p != _accepted.end(); ++p)
        {
            closeSocket(*p);
        }
        _accepted.clear();
    }

    if(_fd != INVALID_SOCKET)
    {
        closeSocketNoThrow(_fd);
        _fd = INVALID_SOCKET;
    }
}

EndpointIPtr
IceInternal::StreamAcceptor::listen()
{
    try
    {
        const_cast<Address&>(_addr) = doBind(_fd, _addr);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
    _endpoint = _endpoint->endpoint(this);
    return _endpoint;
}

void
IceInternal::StreamAcceptor::startAccept()
{
    assert(_fd != INVALID_SOCKET);

    //
    // If there are already sockets waiting to be accepted, we just
    // notify the selector that the acceptor is ready for acceting the
    // new socket. Otherwise, we set the _acceptPending flag, when a
    // new socket connection event is received, the message handler
    // will notify the selector.
    //
    IceUtil::Mutex::Lock lock(_mutex);
    assert(!_acceptPending);
    if(!_accepted.empty())
    {
        _completedHandler(SocketOperationRead);
    }
    else
    {
        _acceptPending = true;
    }
}

void
IceInternal::StreamAcceptor::finishAccept()
{
    //
    // Nothing to do, we just check there's at least one accepted
    // socket or the acceptor was closed.
    //
    IceUtil::Mutex::Lock lock(_mutex);
    assert(!_acceptPending && (!_accepted.empty() || _fd == INVALID_SOCKET));
}

TransceiverPtr
IceInternal::StreamAcceptor::accept()
{
    if(_fd == INVALID_SOCKET) // Acceptor closed.
    {
        assert(_accepted.empty());
        throw SocketException(__FILE__, __LINE__);
    }

    StreamSocket^ fd;
    {
        IceUtil::Mutex::Lock lock(_mutex);
        assert(!_accepted.empty());
        fd = _accepted.front();
        _accepted.pop_front();
    }

    return new StreamTransceiver(_instance, fd, true);
}

string
IceInternal::StreamAcceptor::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::StreamAcceptor::toString() const
{
    return addrToString(_addr);
}

string
IceInternal::StreamAcceptor::toDetailedString() const
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
IceInternal::StreamAcceptor::effectivePort() const
{
    return getPort(_addr);
}

IceInternal::StreamAcceptor::StreamAcceptor(const StreamEndpointIPtr& endpoint,
                                            const ProtocolInstancePtr& instance,
                                            const string& host,
                                            int port) :
    _endpoint(endpoint),
    _instance(instance),
    _addr(getAddressForServer(host, port, _instance->protocolSupport(), instance->preferIPv6())),
    _acceptPending(false)
{
    _fd = ref new StreamSocketListener();

    safe_cast<StreamSocketListener^>(_fd)->ConnectionReceived +=
        ref new TypedEventHandler<StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^>(
            [=](StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^ args)
                {
                    queueAcceptedSocket(args->Socket);
                });
}

IceInternal::StreamAcceptor::~StreamAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}

void
IceInternal::StreamAcceptor::queueAcceptedSocket(StreamSocket^ socket)
{
    IceUtil::Mutex::Lock lock(_mutex);
    if(_fd == INVALID_SOCKET) // Acceptor was closed.
    {
        closeSocket(socket);
        return;
    }
    _accepted.push_back(socket);

    //
    // If the acceptor is waiting for a socket to be accepted, notify
    // the selector that the acceptor is ready for "read". This will
    // in turn caused finishAccept() and accept() to be called by the
    // thread pool. If the acceptor isn't ready to accept the socket,
    // it is just queued, when startAccept is called it will be dequed.
    //
    if(_acceptPending)
    {
        _completedHandler(SocketOperationRead);
        _acceptPending = false;
    }
}
