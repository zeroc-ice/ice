// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SslConnector.h>
#include <Ice/SslTransceiver.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Network.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::SslConnector::connect(int timeout)
{
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "trying to establish ssl connection to " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    int fd = createSocket(false);
    doConnect(fd, _addr, timeout);

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "ssl connection established\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    return new SslTransceiver(_instance, fd);
}

string
IceInternal::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceInternal::SslConnector::SslConnector(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    getAddress(host.c_str(), port, _addr);
}

IceInternal::SslConnector::~SslConnector()
{
}
