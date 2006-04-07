// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TcpConnector.h>
#include <Ice/TcpTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::TcpConnector::connect(int timeout)
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "trying to establish tcp connection to " << toString();
    }

    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, timeout);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "tcp connection established\n" << fdToString(fd);
    }

    return new TcpTransceiver(_instance, fd);
}

string
IceInternal::TcpConnector::toString() const
{
    return addrToString(_addr);
}

IceInternal::TcpConnector::TcpConnector(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger)
{
    getAddress(host, port, _addr);
}

IceInternal::TcpConnector::~TcpConnector()
{
}
