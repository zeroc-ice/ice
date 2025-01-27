// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../Instance.h"
#    include "../Network.h"
#    include "../UniqueRef.h"
#    include "Ice/LocalException.h"
#    include "Ice/Properties.h"
#    include "Ice/StringUtil.h"
#    include "StreamAcceptor.h"
#    include "StreamEndpointI.h"
#    include "StreamTransceiver.h"

#    include <CoreFoundation/CoreFoundation.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceObjC::StreamAcceptor::getNativeInfo()
{
    return shared_from_this();
}

void
IceObjC::StreamAcceptor::close()
{
    if (_fd != INVALID_SOCKET)
    {
        closeSocketNoThrow(_fd);
        _fd = INVALID_SOCKET;
    }
}

EndpointIPtr
IceObjC::StreamAcceptor::listen()
{
    try
    {
        const_cast<Address&>(_addr) = doBind(_fd, _addr);
        doListen(_fd, _backlog);
    }
    catch (...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

    _endpoint = _endpoint->endpoint(shared_from_this());
    return _endpoint;
}

TransceiverPtr
IceObjC::StreamAcceptor::accept()
{
    SOCKET fd = doAccept(_fd);
    setBlock(fd, false);
    setTcpBufSize(fd, _instance);

    //
    // Create the read/write streams
    //
    UniqueRef<CFReadStreamRef> readStream;
    UniqueRef<CFWriteStreamRef> writeStream;
    try
    {
        CFStreamCreatePairWithSocket(nullptr, fd, &readStream.get(), &writeStream.get());
        _instance->setupStreams(readStream.get(), writeStream.get(), true, "");
        return make_shared<StreamTransceiver>(_instance, readStream.release(), writeStream.release(), fd);
    }
    catch (const Ice::LocalException& ex)
    {
        if (fd != INVALID_SOCKET)
        {
            closeSocketNoThrow(fd);
        }
        throw;
    }
}

string
IceObjC::StreamAcceptor::protocol() const
{
    return _instance->protocol();
}

string
IceObjC::StreamAcceptor::toString() const
{
    return addrToString(_addr);
}

string
IceObjC::StreamAcceptor::toDetailedString() const
{
    ostringstream os;
    os << "local address = " << toString();
    return os.str();
}

int
IceObjC::StreamAcceptor::effectivePort() const
{
    return getPort(_addr);
}

IceObjC::StreamAcceptor::StreamAcceptor(
    const StreamEndpointIPtr& endpoint,
    const InstancePtr& instance,
    const string& host,
    int port)
    : _endpoint(endpoint),
      _instance(instance),
      _addr(getAddressForServer(host, port, instance->protocolSupport(), instance->preferIPv6(), true))
{
    _backlog = instance->properties()->getIcePropertyAsInt("Ice.TCP.Backlog");

    try
    {
        _fd = createSocket(false, _addr);
        setBlock(_fd, false);
        setTcpBufSize(_fd, _instance);
        setReuseAddress(_fd, true);
    }
    catch (...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

IceObjC::StreamAcceptor::~StreamAcceptor() { assert(_fd == INVALID_SOCKET); }

#endif
