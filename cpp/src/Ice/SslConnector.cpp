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
#include <Ice/LocalException.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceInternal::SslConnector::connect(int timeout)
{
#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "trying to establish ssl connection to " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif

    int fd = createSocket(false);
    doConnect(fd, _addr, timeout);

#ifndef ICE_NO_TRACE
    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "ssl connection established\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }
#endif	

    return new SslTransceiver(_instance, fd);
}

string
IceInternal::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceInternal::SslConnector::SslConnector(const InstancePtr& instance, const string& host, int port) :
    _instance(instance)
{
#ifndef ICE_NO_TRACE
    _traceLevels = _instance->traceLevels();
    _logger = _instance->logger();
#endif

    getAddress(host.c_str(), port, _addr);
}

IceInternal::SslConnector::~SslConnector()
{
}
