// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Logger.h>
#include <Ice/Network.h>

#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/SslConnector.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceSSL::SslConnector::connect(int timeout)
{
    TraceLevelsPtr traceLevels = _plugin->getTraceLevels();
    LoggerPtr logger = _plugin->getLogger();

    if(traceLevels->network >= 2)
    {
	ostringstream s;
	s << "trying to establish ssl connection to " << toString();
	logger->trace(traceLevels->networkCat, s.str());
    }

    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, timeout);

    if(traceLevels->network >= 1)
    {
	ostringstream s;
	s << "ssl connection established\n" << fdToString(fd);
	logger->trace(traceLevels->networkCat, s.str());
    }

    return _plugin->createTransceiver(IceSSL::Client, fd);
}

string
IceSSL::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceSSL::SslConnector::SslConnector(const OpenSSLPluginIPtr& plugin, const string& host, int port) :
    _plugin(plugin)
{
    getAddress(host.c_str(), port, _addr);
}

IceSSL::SslConnector::~SslConnector()
{
}
