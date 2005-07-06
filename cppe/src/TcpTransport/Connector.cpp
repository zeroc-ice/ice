// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Connector.h>
#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Network.h>
#include <IceE/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Connector* p) { p->__incRef(); }
void IceInternal::decRef(Connector* p) { p->__decRef(); }

TransceiverPtr
Connector::connect(int timeout)
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "trying to establish tcp connection to " << toString();
    }

    SOCKET fd = createSocket();
    setBlock(fd, false);
    doConnect(fd, _addr, timeout);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "tcp connection established\n" << fdToString(fd);
    }

    return new Transceiver(_instance, fd);
}

string
Connector::toString() const
{
    return addrToString(_addr);
}

Connector::Connector(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    getAddress(host, port, _addr);
}

Connector::~Connector()
{
}
