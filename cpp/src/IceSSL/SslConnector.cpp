// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    SslTransceiverPtr transceiver = _plugin->createTransceiver(IceSSL::Client, fd, timeout);

    transceiver->forceHandshake();

    return transceiver;
}

string
IceSSL::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceSSL::SslConnector::SslConnector(const OpenSSLPluginIPtr& plugin, const string& host, int port) :
    _plugin(plugin)
{
    getAddress(host, port, _addr);
}

IceSSL::SslConnector::~SslConnector()
{
}
