// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "StreamTransceiver.h"
#include "StreamEndpointI.h"
#include "StreamAcceptor.h"

#include <IceUtil/StringUtil.h>

#include <Ice/Instance.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>

#include <CoreFoundation/CoreFoundation.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceObjC::StreamAcceptor::getNativeInfo()
{
    return this;
}

void
IceObjC::StreamAcceptor::close()
{
    if(_fd != INVALID_SOCKET)
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
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

    _endpoint = _endpoint->endpoint(this);
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
    CFReadStreamRef readStream = nil;
    CFWriteStreamRef writeStream = nil;
    try
    {
        CFStreamCreatePairWithSocket(NULL, fd, &readStream, &writeStream);
        _instance->setupStreams(readStream, writeStream, true, "");
        return new StreamTransceiver(_instance, readStream, writeStream, fd);
    }
    catch(const Ice::LocalException& ex)
    {
        if(fd != INVALID_SOCKET)
        {
            closeSocketNoThrow(fd);
        }
        if(readStream)
        {
            CFRelease(readStream);
        }
        if(writeStream)
        {
            CFRelease(writeStream);
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
    vector<string> intfs = getHostsForEndpointExpand(inetAddrToString(_addr), _instance->protocolSupport(), true);
    if(!intfs.empty())
    {
        os << "\nlocal interfaces = ";
        os << IceUtilInternal::joinString(intfs, ", ");
    }
    return os.str();
}

int
IceObjC::StreamAcceptor::effectivePort() const
{
    return getPort(_addr);
}

IceObjC::StreamAcceptor::StreamAcceptor(const StreamEndpointIPtr& endpoint,
                                        const InstancePtr& instance,
                                        const string& host,
                                        int port) :
    _endpoint(endpoint),
    _instance(instance),
    _addr(getAddressForServer(host, port, instance->protocolSupport(), instance->preferIPv6()))
{
#ifdef SOMAXCONN
    _backlog = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.Backlog", SOMAXCONN);
#else
    _backlog = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);
#endif

    try
    {
        _fd = createSocket(false, _addr);
        setBlock(_fd, false);
        setTcpBufSize(_fd, _instance);
        setReuseAddress(_fd, true);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

IceObjC::StreamAcceptor::~StreamAcceptor()
{
    assert(_fd == INVALID_SOCKET);
}
