// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Logger.h>
#include <Ice/Network.h>

#include <IceSSL/PluginBaseI.h>
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

    IceSSL::ConnectionPtr connection = _plugin->createConnection(IceSSL::Client, fd);
    return new SslTransceiver(_plugin, fd, connection);
}

string
IceSSL::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceSSL::SslConnector::SslConnector(const PluginBaseIPtr& plugin, const string& host, int port) :
    _plugin(plugin)
{
    getAddress(host.c_str(), port, _addr);
}

IceSSL::SslConnector::~SslConnector()
{
}
