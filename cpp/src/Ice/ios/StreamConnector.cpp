// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../Network.h"
#    include "../NetworkProxy.h"
#    include "../UniqueRef.h"
#    include "Ice/Properties.h"
#    include "StreamConnector.h"
#    include "StreamEndpointI.h"
#    include "StreamTransceiver.h"

#    include <CoreFoundation/CoreFoundation.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceObjC::StreamConnector::connect()
{
    UniqueRef<CFReadStreamRef> readStream;
    UniqueRef<CFWriteStreamRef> writeStream;
    UniqueRef<CFStringRef> h(CFStringCreateWithCString(nullptr, _host.c_str(), kCFStringEncodingUTF8));
    UniqueRef<CFHostRef> host(CFHostCreateWithName(nullptr, h.get()));
    CFStreamCreatePairWithSocketToCFHost(nullptr, host.get(), _port, &readStream.get(), &writeStream.get());
    _instance->setupStreams(readStream.get(), writeStream.get(), false, _host);
    return make_shared<StreamTransceiver>(_instance, readStream.release(), writeStream.release(), _host, _port);
}

int16_t
IceObjC::StreamConnector::type() const
{
    return _instance->type();
}

string
IceObjC::StreamConnector::toString() const
{
    string proxyHost = _instance->proxyHost();
    ostringstream os;
    if (!proxyHost.empty())
    {
        os << proxyHost << ":" << _instance->proxyPort();
    }
    else
    {
        os << _host << ":" << _port;
    }
    return os.str();
}

bool
IceObjC::StreamConnector::operator==(const IceInternal::Connector& r) const
{
    const StreamConnector* p = dynamic_cast<const StreamConnector*>(&r);
    if (!p)
    {
        return false;
    }

    if (_timeout != p->_timeout)
    {
        return false;
    }

    if (_connectionId != p->_connectionId)
    {
        return false;
    }

    if (_host != p->_host)
    {
        return false;
    }

    if (_port != p->_port)
    {
        return false;
    }

    return true;
}

bool
IceObjC::StreamConnector::operator<(const IceInternal::Connector& r) const
{
    const StreamConnector* p = dynamic_cast<const StreamConnector*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (_timeout < p->_timeout)
    {
        return true;
    }
    else if (p->_timeout < _timeout)
    {
        return false;
    }

    if (_connectionId < p->_connectionId)
    {
        return true;
    }
    else if (p->_connectionId < _connectionId)
    {
        return false;
    }

    if (_host < p->_host)
    {
        return true;
    }
    else if (p->_host < _host)
    {
        return false;
    }

    return _port < p->_port;
}

IceObjC::StreamConnector::StreamConnector(
    const InstancePtr& instance,
    const string& host,
    int32_t port,
    int32_t timeout,
    const string& connectionId)
    : _instance(instance),
      _host(host.empty() ? string("127.0.0.1") : host),
      _port(port),
      _timeout(timeout),
      _connectionId(connectionId)
{
}

IceObjC::StreamConnector::~StreamConnector() = default;
#endif
