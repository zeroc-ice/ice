// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
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

IceUtil::Shared* IceInternal::upCast(Connector* p) { return p; }

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
    setTcpBufSize(fd, _instance->initializationData().properties, _logger);
    doConnect(fd, _addr, timeout);
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
    setBlock(fd, true);
#endif

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
    _logger(instance->initializationData().logger)
{
    getAddress(host, port, _addr);
}

Connector::~Connector()
{
}
