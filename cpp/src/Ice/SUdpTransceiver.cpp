// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SUdpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Buffer.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>
#include <Ice/SUdpClientControlChannel.h>
#include <Ice/SUdpServerControlChannel.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSecurity::SecureUdp;

void IceInternal::incRef(SUdpTransceiver* p) { p->__incRef(); }
void IceInternal::decRef(SUdpTransceiver* p) { p->__decRef(); }

SOCKET
IceInternal::SUdpTransceiver::fd()
{
    return _udpTransceiver.fd();
}

void
IceInternal::SUdpTransceiver::close()
{
    _udpTransceiver.close();
}

void
IceInternal::SUdpTransceiver::shutdown()
{
    _udpTransceiver.shutdown();
}

void
IceInternal::SUdpTransceiver::write(Buffer& buf, int)
{
    _udpTransceiver.write(buf,0);
}

void
IceInternal::SUdpTransceiver::read(Buffer& buf, int)
{
    _udpTransceiver.read(buf,0);
}

string
IceInternal::SUdpTransceiver::toString() const
{
    return _udpTransceiver.toString();
}

bool
IceInternal::SUdpTransceiver::equivalent(const string& host, int port) const
{
    return _udpTransceiver.equivalent(host, port);
}

int
IceInternal::SUdpTransceiver::effectivePort()
{
    return _udpTransceiver.effectivePort();
}

IceInternal::SUdpTransceiver::SUdpTransceiver(const InstancePtr& instance, const string& host, int port) :
    _udpTransceiver(instance, host, port, "sudp"),
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    // Perform our handshake with the server
    connectControlChannel(host, port);
}

IceInternal::SUdpTransceiver::SUdpTransceiver(const InstancePtr& instance, int port, bool connect) :
    _udpTransceiver(instance, port, connect, "sudp"),
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    // Build our control channel
    createControlChannel(port);
}

IceInternal::SUdpTransceiver::~SUdpTransceiver()
{
    _controlChannel->unsetTransceiver();
}

void
IceInternal::SUdpTransceiver::connectControlChannel(const string& host, int port)
{
    // Create a control channel, one for this Client SUdp connection
    _controlChannel = new ClientControlChannel(this, _instance, host, port);
}

void
IceInternal::SUdpTransceiver::createControlChannel(int port)
{
    // Create a control channel, one for this Server SUdp connection
    _controlChannel = new ServerControlChannel(this, _instance, port);
}

